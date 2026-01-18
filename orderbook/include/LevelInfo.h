#ifndef LVLINFO_H
#define LVLINFO_H

#include <vector>

struct LevelInfo {
    double price;
    unsigned int quantity;
};

class OrderbookLevelInfos { 
    private:
        std::vector<LevelInfo> m_bids{};
        std::vector<LevelInfo> m_asks{};

    public: 
        OrderbookLevelInfos( const std::vector<LevelInfo>& bids, const std::vector<LevelInfo>& asks ):
        m_bids{ bids },
        m_asks{ asks }
    {}
    
    const std::vector<LevelInfo>& getBids() const { return m_bids; }
    const std::vector<LevelInfo>& getAsks() const { return m_asks; }

};

#endif