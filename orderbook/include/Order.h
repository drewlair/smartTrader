#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <list>

#include "Constants.h"

enum class Side{
    Buy,
    Sell
};

enum class OrderType {
    GoodTillCancel,
    FillAndKill,
    FillOrKill,
    GoodForDay,
    Market,
};

class Order {
    private:
        OrderType m_orderType{};
        unsigned int m_orderID{};
        Side m_side{};
        unsigned int m_initialQuantity{};
        unsigned int m_remainingQuantity{};
        double m_price{};
        

    public: 
        Order( OrderType orderType, unsigned int orderID, Side side, unsigned int quantity, double price ):
            m_orderType{ orderType },
            m_orderID{ orderID },
            m_side{ side },
            m_initialQuantity{ quantity },
            m_remainingQuantity{ quantity },
            m_price{ price }
        {}

        Order( unsigned int orderID, Side side, unsigned int quantity ):
            Order( OrderType::Market, orderID, side, quantity, Constants::InvalidPrice )
        {}

        OrderType getOrderType() const;
        unsigned int getOrderID() const;
        Side getSide() const;
        unsigned int getInitialQuantity() const;
        unsigned int getRemainingQuantity() const;
        double getPrice() const;
        unsigned int getFilledQuantity() const;

        void Fill(unsigned int quantity);
        bool isFilled() const;

        void toGoodTillCancel(const double order);
    
};

using OrderPointer = std::shared_ptr<Order>;
using OrderPointers = std::list<OrderPointer>;



#endif