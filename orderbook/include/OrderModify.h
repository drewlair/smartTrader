#ifndef ORDERMODIFY_H
#define ORDERMODIFY_H

#include "Order.h"

class OrderModify {

    private: 
        unsigned int m_orderID{};
        Side m_side{};
        double m_price{};
        unsigned int m_quantity{};

    public:
        OrderModify( unsigned int orderID, Side side, double price, unsigned int quantity ):
            m_orderID{ orderID },
            m_side{ side },
            m_price{ price },
            m_quantity{ quantity }
        {}

        unsigned int getOrderID() const;
        Side getSide() const;
        double getPrice() const;
        unsigned int getQuantity() const;

        OrderPointer ToOrderPointer( OrderType type ) const;
};

#endif