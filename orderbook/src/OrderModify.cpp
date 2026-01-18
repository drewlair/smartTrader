#include "OrderModify.h"


unsigned int OrderModify::getOrderID() const { return m_orderID; }
Side OrderModify::getSide() const { return m_side; }
double OrderModify::getPrice() const { return m_price; }
unsigned int OrderModify::getQuantity() const { return m_quantity; }

OrderPointer OrderModify::ToOrderPointer( OrderType type ) const
{
    return std::make_shared<Order>( type, getOrderID(), getSide(), getPrice(), getQuantity() );
}