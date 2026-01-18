#include <stdexcept>
#include <iostream>

#include "Order.h"

OrderType Order::getOrderType() const            { return m_orderType; }
unsigned int Order::getOrderID() const           { return m_orderID; }
Side Order::getSide() const                      { return m_side; }
unsigned int Order::getInitialQuantity() const   { return m_initialQuantity; }
unsigned int Order::getRemainingQuantity() const { return m_remainingQuantity; }
double Order::getPrice() const                   { return m_price; }
unsigned int Order::getFilledQuantity() const    { return getInitialQuantity() - getRemainingQuantity(); }


void Order::Fill( unsigned int quantity ) {
    if ( quantity > Order::getRemainingQuantity() ) 
        throw std::logic_error("Order ({}) cannot be filled for more than its remaining quantity");
    m_remainingQuantity -= quantity;
}

bool Order::isFilled() const {
    return Order::getRemainingQuantity() == 0;
}

void Order::toGoodTillCancel(double price) {
    if (getOrderType() != OrderType::Market)
        throw std::logic_error(std::format("Order ({}) cannot have its price adjusted, only market orders can.", getOrderID()));

    m_price = price;
    m_orderType = OrderType::GoodTillCancel;
}

