#include <format>
#include "Orderbook.h"

Orderbook::Orderbook()
{}
    /*m_ordersPruneThread{ [this] { pruneGoodForDayOrders(); } }
{} */

Orderbook::~Orderbook()
{
    m_shutdown.store(true, std::memory_order_release);
	m_shutdownConditionVariable.notify_one();
	//m_ordersPruneThread.join();
}



bool Orderbook::canMatch( Side side, double price ) const{
    //Buy order 
    if (side == Side::Buy && m_asks.empty()) 
        return false;

    if (side == Side::Buy) {
        
        // Get best available ask from the asks list (lowest sell price)
        const auto& [bestAsk, _] = *m_asks.begin();
        return price >= bestAsk;
    }

    // Sell order
    else if (m_bids.empty())
        return false;
    else {
        // Get best available bid from bids list (highest bid price)
        const auto& [bestBid, _] = *m_bids.begin();
        return price <= bestBid;
    }
}



std::vector<Trade> Orderbook::matchOrders() {

    std::vector<Trade> trades{};

    trades.reserve( m_orders.size() );


    while (true) {
        if (m_bids.empty() || m_asks.empty())
            break;
        // std::map<double, OrderPointers, std::greater<double>> m_bids;
        auto& [bidPrice, bids] = *m_bids.begin();
        auto& [askPrice, asks] = *m_asks.begin();

        if (bidPrice < askPrice){
            break;
        }


        while (!bids.empty() && !asks.empty()) {
            //get highest bid and lowest ask
            //OrderPointer -> std::shared_ptr<Order>;
            auto bid = bids.front();
            auto ask = asks.front();

            //larget quantity that can be traded
            unsigned int quantity = std::min(bid->getRemainingQuantity(), ask->getRemainingQuantity());

            //execute the trades (changing the quantity for both order objects)
            bid->Fill(quantity);
            ask->Fill(quantity);

 
            if (bid->isFilled())
            {
                bids.pop_front();
                m_orders.erase(bid->getOrderID());
            }

            if (ask->isFilled())
            {
                asks.pop_front();
                m_orders.erase(ask->getOrderID());
            }

            //record trade in trades list
            trades.push_back( 
                Trade{ 
                    TradeInfo{ bid->getOrderID(), bid->getPrice(), quantity }, 
                    TradeInfo{ ask->getOrderID(), ask->getPrice(), quantity } 
                } 
            );

            onOrderMatched(bid->getPrice(), quantity, bid->isFilled());
            onOrderMatched(ask->getPrice(), quantity, ask->isFilled());


        }

         // if asks or bids list is empty, remove its price level altogether
        if (bids.empty()){
            m_bids.erase( bidPrice );
            m_data.erase( bidPrice );
        }

        if (asks.empty()){
            m_asks.erase( askPrice );
            m_data.erase( askPrice );
        }

    }

    //if there are no more bids/asks, 
    if (!m_bids.empty()) {
        auto& [_, bids] = *m_bids.begin();
        auto& order = bids.front();
        if (order->getOrderType() == OrderType::FillAndKill)
            cancelOrder(order->getOrderID());
    }

    if (!m_asks.empty()) {
        auto& [_, asks] = *m_asks.begin();
        auto& order = asks.front();
        if (order->getOrderType() == OrderType::FillAndKill)
            cancelOrder(order->getOrderID());
    }

    return trades;
}



std::vector<Trade> Orderbook::addOrder(OrderPointer order) {

    std::scoped_lock ordersLock{ m_ordersMutex };

    // if the order is in the system alr, don't do anything
    if (m_orders.count(order->getOrderID()))
        return {};

    //if a order is of type Market, convert the order to a limit order with the worst price on either side
    //so it is guaranteed to be executed
    if (order->getOrderType() == OrderType::Market) {
        if (order->getSide() == Side::Buy && !m_asks.empty()) {
            const auto& [worstAsk, _] = *m_asks.rbegin();
            order->toGoodTillCancel(worstAsk);
        } else if (order->getSide() == Side::Sell && !m_bids.empty()) {
            const auto& [worstAsk, _] = *m_asks.rbegin();
            order->toGoodTillCancel(worstAsk);
        } else
            return {};
    }

    // if the order is fillandkill and we can't find a match
    if (order->getOrderType() == OrderType::FillAndKill && !canMatch(order->getSide(), order->getPrice()))
        return {};

    if (order->getOrderType() == OrderType::FillOrKill && !canFullyFill(order->getSide(), order->getPrice(), order->getInitialQuantity())) {
        return {};
    }

    //either goodtillcancel or fillandkill and can match
    //get iterator for orders
    OrderPointers::iterator iter;

    //if order is a buy, add to bids orders and get orders list
    if (order->getSide() == Side::Buy) {
        //get ref to bids list
        auto& orders = m_bids[order->getPrice()];
        // add order to list
        orders.push_back(order);
        //get location of newly added order
        iter = std::prev(orders.end());
    }

    //if order is a sell, add to asks orders and get orders list
    else {
        auto& orders = m_asks[order->getPrice()];
        orders.push_back(order);
        iter = std::prev(orders.end());
    }

    // enter new order to orders list
    m_orders.insert( { order->getOrderID(), OrderEntry{ order, iter } });

    onOrderAdded(order);

    //execute trades with new order list state
    return matchOrders();

}

std::vector<Trade> Orderbook::matchOrder( OrderModify order ) { 
    //if order doesn't exist, return empty object
    if (m_orders.count(order.getOrderID()) == 0)
        return {};

    //get order from order list
    const auto& [existingOrder, _] = m_orders.at(order.getOrderID());

    //cancel the existing order
    cancelOrder(order.getOrderID());
    
    //run addOrder() on new order, replace in same spot the old order was removed from in the order list
    return addOrder(order.ToOrderPointer(existingOrder->getOrderType()));
}

std::size_t Orderbook::Size() const { return m_orders.size(); }

LevelInfo Orderbook::createLevelInfos(const double price, const OrderPointers& orders) const { 
    //for each price and order in the orders list map, return a LevelInfo object with price and the sum of the quantities for each order in that order list
    return LevelInfo{ price, 
        //sum from the beginning to the end of the orders list
        static_cast<unsigned int>(
            std::accumulate(orders.begin(), orders.end(), 0.0,
            //keep a runningSum for the total to be returned for each order
                [](std::size_t runningSum, const OrderPointer& order)
                //sum by adding the remaining units to be traded to the runningSum
                { return runningSum + order->getRemainingQuantity(); } 
            )
        )
    };
}

OrderbookLevelInfos Orderbook::getOrderInfos() const { 
    //declare lists for the bid and ask orders we'll be grabbing info from
    std::vector<LevelInfo> bidInfos, askInfos;
    bidInfos.reserve(m_orders.size());
    askInfos.reserve(m_orders.size());

    //Creating a lambda to return a levelInfo struct for each order with each price
    //each struct with have the sum of quantity of units desired to trade from each order, and the price the order was placed at

    //args will be the price and orders list


    //call the CreateLevelInfos lambda on both bids and asks
    for (const auto& [price, orders] : m_bids)
        bidInfos.push_back( createLevelInfos(price, orders));
    for (const auto& [price, orders] : m_asks)
        askInfos.push_back( createLevelInfos(price, orders));

    //return a struct with both new LevelInfo structs
    return OrderbookLevelInfos{ bidInfos, askInfos };
}

//cancel all GoodForDay orders at 4pm
void Orderbook::pruneGoodForDayOrders() { 
    //get all the time funcs in the nmspc
    using namespace std::chrono;
    //set end to 4pm (this is in 24hr time)
    const auto end = hours(16);

    //keep the thread running throughout the day
    while (true) { 
        // returns a std::chrono::time_point representing the current point in time
        const auto now = system_clock::now();
        //converts time to seconds (counted from 1970 lol), becomes a time_t struct
        const auto now_c = system_clock::to_time_t(now);

        //calendar struct std::tm
        std::tm now_parts;
        //converts given time (now_c) into calendar time (now_parts)
        localtime_r(&now_c, &now_parts);

        //if it's past 4pm, add one more day!
        if (now_parts.tm_hour >= end.count())
            now_parts.tm_mday += 1;

        //reset the specifics time in our time object
        now_parts.tm_hour = end.count();
        now_parts.tm_min = 0;
        now_parts.tm_sec = 0;

        //figure out the time needed to wait until it is time to check for GoodForDay order to cancel
        auto next = system_clock::from_time_t(mktime(&now_parts));
        auto till = next - now + milliseconds(100);

        //prevent race condition with mutex_lock
        {
            //define lock obj
            std::unique_lock ordersLock{ m_ordersMutex };
            //if shutdown interrupt (invoked independently of time by user) or we've waited for the time until 4pm (market close)
            //then finish executing code
            // the scope is for the scoped_lock, 
            //if the shutdown hits, we won't call the followup wait, which is why the conditional is used
            if (m_shutdown.load(std::memory_order_acquire) ||
                m_shutdownConditionVariable.wait_for(ordersLock, till) == std::cv_status::no_timeout)
                return;

        }

        //define orderID list
        std::vector<unsigned int> orderIDs;

        //scope for new lock
        {
            //define lock
            std::scoped_lock ordersLock{ m_ordersMutex };

            //for every order in the order list
            for (const auto& [_1, entry] : m_orders) {
                //get the order
                const auto& [order, _2] = entry;
                //if the order isn't GoodForDay, check the next one
                if (order->getOrderType() != OrderType::GoodForDay)
                    continue;
                //if it is, add it to the orders list
                orderIDs.push_back(order->getOrderID());
            }
        }

        cancelOrders(orderIDs);
    }
}


void Orderbook::cancelOrders(std::vector<unsigned int>& orderIDs) {
    std::scoped_lock ordersLock{ m_ordersMutex };

    for (unsigned int orderID : orderIDs)
        cancelOrderInternal(orderID);

}

void Orderbook::cancelOrder(unsigned int orderID) {
    std::scoped_lock ordersLock{ m_ordersMutex };
    cancelOrderInternal(orderID);
}

void Orderbook::cancelOrderInternal( unsigned int orderID ) { 
    //if order doesn't exist, do nothing
    if ( m_orders.count(orderID) == 0 ) {
        return;
    }

    // get order from list and order location
    const auto [order, orderIterator] = m_orders.at(orderID);
    m_orders.erase(orderID);
    //remote the order from the bids/asks list
    //get from asks if sell, bids if buy
    if (order->getSide() == Side::Sell) { 

        //get price of the order
        auto price = order->getPrice();

        // get list of order locations at price
        auto& orders = m_asks.at(price);

        //erase the order location from the list
        orders.erase(orderIterator);

        //if that was the last element in the list, remove the price key as well
        if (orders.empty())
            m_asks.erase(price);

    } else {
        auto price = order->getPrice();

        auto& orders = m_bids.at(price);
        orders.erase(orderIterator);

        if (orders.empty())
            m_bids.erase(price);
    }

    //remote order from order list
    onOrderCancel(order);

}


void Orderbook::onOrderCancel(OrderPointer order) { 
    updateLevelData(order->getPrice(), order->getRemainingQuantity(), LevelData::Action::Remove);
}

void Orderbook::onOrderAdded(OrderPointer order) {
    updateLevelData(order->getPrice(), order->getInitialQuantity(), LevelData::Action::Add);
}

void Orderbook::onOrderMatched(double price, unsigned int quantity, bool isFullyFilled) { 
    updateLevelData(price, quantity, isFullyFilled ? LevelData::Action::Remove : LevelData::Action::Match);
}


void Orderbook::updateLevelData(double price, unsigned int quantity, LevelData::Action action) { 
    //get levelData obj from hash map
    
    
    auto& levelData = m_data[price];
    
    //update the order count based on the action
    //if a remove action happens, one less order at that price, so subtract one
    if (action == LevelData::Action::Remove)
        levelData.m_count -= 1;
    //if it's an addition, one more order at that price, add 1
    else if (action == LevelData::Action::Add)
        levelData.m_count += 1;
    //if it's a match, no order changes, do nothing

    //update the total quantity in the bids/asks list up for buy/sell based on action
    if (action == LevelData::Action::Remove || action == LevelData::Action::Match)
        levelData.m_quantity -= quantity;
    else
        levelData.m_quantity += quantity;

    //if there's no quantity left (therefore, no more orders), remote the structure
    if (levelData.m_count == 0)
        m_data.erase(price);

}

bool Orderbook::canFullyFill(Side side, double price, unsigned int quantity) const {
    //check if the bids/asks on the opp side are empty or if the price is out of range
    if (!canMatch(side, price))
        return false;

    std::optional<double> threshold;

    if (side == Side::Buy) { 
        const auto [askPrice, _] = *m_asks.begin();
        threshold = askPrice;
    } else { 
        const auto [bidPrice, _] = *m_bids.begin();
        threshold = bidPrice;
    }

    for (const auto& [levelPrice, levelData] : m_data) {
        if ((side == Side::Buy && threshold.value() > levelPrice) ||
            (side == Side::Sell && threshold.value() < levelPrice))
            continue;
        
        if ((side == Side::Buy && levelPrice > price) ||
            (side == Side::Sell && levelPrice < price))
            continue;
        
        if (levelData.m_quantity >= quantity)
            return true;
        
        quantity -= levelData.m_quantity;
        
    }

    return false;

}

std::vector<Trade> Orderbook::modifyOrder(OrderModify order) {
    OrderType orderType;

    {
        std::scoped_lock orders_lock{ m_ordersMutex };
        
        if (m_orders.count(order.getOrderID()) == 0)
            return {};

            const auto& [existingOrder, _] = m_orders.at(order.getOrderID());
            orderType = existingOrder->getOrderType();
    }

    cancelOrder(order.getOrderID());
    return addOrder(order.ToOrderPointer(orderType));
}


