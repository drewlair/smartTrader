#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <vector>
#include <string>
#include <string_view>
#include <unordered_map>
#include <map>
#include <iterator>
#include <numeric>
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>


#include "Order.h"
#include "Trade.h"
#include "LevelInfo.h"
#include "OrderModify.h"

class Orderbook {

    private:

        struct OrderEntry {
            OrderPointer m_order{ nullptr };
            OrderPointers::iterator m_location;
        };

        struct LevelData { 
            unsigned int m_quantity;
            unsigned int m_count;

            enum class Action { 
                Add,
                Remove,
                Match,
            };
        };

        std::unordered_map<double, LevelData> m_data;
        std::map<double, OrderPointers, std::greater<double>> m_bids;
        std::map<double, OrderPointers, std::less<double>> m_asks;
        std::unordered_map<unsigned int, OrderEntry> m_orders;
        mutable std::mutex m_ordersMutex;
        std::thread m_ordersPruneThread;
        std::condition_variable m_shutdownConditionVariable;
        std::atomic<bool> m_shutdown{ false };

        void pruneGoodForDayOrders();

        // remove order from order and bids/asks lists
        void cancelOrders( std::vector<unsigned int>& orderIDs);
        void cancelOrderInternal(unsigned int orderID);

        void onOrderCancel(OrderPointer order);
        void onOrderAdded(OrderPointer order);
        void onOrderMatched(double price, unsigned int quantity, bool isFullyFilled);
        void updateLevelData(double price, unsigned int quantity, LevelData::Action action);

        bool canFullyFill(Side side, double price, unsigned int quantity) const;
        bool canMatch(Side side, double price) const;
        std::vector<Trade> matchOrders();

    public:
        //forward declare constr and destr
        Orderbook();
        Orderbook(const Orderbook&) = delete;
        void operator=(const Orderbook&) = delete;
        void operator=(const Orderbook&&) = delete;
        Orderbook(Orderbook&&) = delete;
        ~Orderbook();

        //add order to order + bids/asks lists and run matchOrders()
        std::vector<Trade> addOrder(OrderPointer order);
        void cancelOrder(unsigned int orderID);
        std::vector<Trade> modifyOrder(OrderModify order);
        
        std::vector<Trade> matchOrder( OrderModify order );

        std::size_t Size() const;

        LevelInfo createLevelInfos(const double price, const OrderPointers& orders) const;
        OrderbookLevelInfos getOrderInfos() const;

        



};


#endif