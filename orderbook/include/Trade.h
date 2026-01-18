#ifndef TRADE_H
#define TRADE_H


struct TradeInfo {
    unsigned int orderID;
    double price;
    unsigned int quantity;
};

class Trade {
    private:
        TradeInfo m_bidTrade{};
        TradeInfo m_askTrade{};

    public:
        Trade( const TradeInfo& bidTrade, const TradeInfo& askTrade ):
            m_bidTrade{ bidTrade },
            m_askTrade{ askTrade }
        {}

        const TradeInfo& getBidTrade();
        const TradeInfo& getAskTrade();
};


#endif