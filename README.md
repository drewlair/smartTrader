# smartTrader
 -> a low-latency orderbook

interface
 * orders
    Type:
        - GoodTillCancel,
            -> Order stays in the book until either the order is filled or a cancel is called on the order 
            -> Order may participate in multiple trades until it is entirely filled
        - FillAndKill,
            -> Fill as much of the order as possible and cancel future trades on the remaining quantity
        - FillOrKill,
            -> Attempt to fill the entire order or do not execute any trades and cancel
        - GoodForDay,
            -> Order stays on the book until it's filled or the end of the trading day is reached
        - Market,
            -> Order doesn't specify a max bid or min ask price, order is filled based on the best available trade on the other side
    Side:
        - Buy
            -> bid to acquire a quantity of an asset
        - Sell
            -> ask to release a quantity of an asset
    - quantity
        -> amount of an asset offered to trade
    - price
        -> price asset is set to trade at
    - orderId
        -> unique identifier given for each order
* trades
    BidInfo, AskInfo:
        -> Information on the bid and ask order participating in the trade
        - quantity
            -> number of asset units traded
        - price
            -> price asset has been traded at
        - orderId
            -> unique identifier for the order
    - tradeId
        -> unique identifier for the trade
* orderbook
    Bids, Asks:
        -> lists of current bid and ask orders open to trade
    Trades:
        -> list of recorded trades
    LevelInfo:
        -> order data for each price with bids/asks open for
