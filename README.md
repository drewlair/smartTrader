# smartTrader
### a low-latency orderbook written in C++

## Interface

* Orders
    - **Type**:
        - _GoodTillCancel_
            -> Order stays in the book until either the order is filled or a cancel is called on the order 
            -> Order may participate in multiple trades until it is entirely filled
        - _FillAndKill_
            -> Fill as much of the order as possible and cancel future trades on the remaining quantity
        - _FillOrKill
            -> Attempt to fill the entire order or do not execute any trades and cancel
        - _GoodForDay_
            -> Order stays on the book until it's filled or the end of the trading day is reached
        - _Market_
            -> Order doesn't specify a max bid or min ask price, order is filled based on the best available trade on the other side
    - **Side**:
        - _Buy_
            -> bid to acquire a quantity of an asset
        - _Sell_
            -> ask to release a quantity of an asset
    - **quantity**
        -> amount of an asset offered to trade
    - **price**
        -> price asset is set to trade at
    - **orderId**
        -> unique identifier given for each order
      
* Trades
    - **BidInfo, AskInfo**:
        -> Information on the bid and ask order participating in the trade
        - _quantity_
            -> number of asset units traded
        - _price_
            -> price asset has been traded at
        - _orderId_
            -> unique identifier for the order
    - **tradeId**
        -> unique identifier for the trade
      
* Orderbook
    - **Bids, Asks**:
        -> lists of current bid and ask orders open to trade
    - **Trades**:
        -> list of recorded trades
    - **LevelInfos**:
        -> Data on orders for each price

### Compatibility
To build the orderbook, install the latest versions of cmake and gcc
* [cmake](https://cmake.org/download/)
* [gcc](https://gcc.gnu.org)


### Testing
To run the built-in tests, build and run the test executable
````
~/smartTrader $ make
g++ -Werror -Weffc++ -Wconversion -Wsign-conversion -g -Wall -Wextra -std=c++20 -O0 -pedantic-errors  testFramework/src/pch.cpp  -I./testFramework/include -I./testFramework/src/thirdPartyLibs/googletest/googletest/include -I./orderbook/include -I./orderbook/src ./testFramework/src/thirdPartyLibs/googletest/build/lib/libgmock_main.a ./testFramework/src/thirdPartyLibs/googletest/build/lib/libgmock.a ./testFramework/src/thirdPartyLibs/googletest/build/lib/libgtest_main.a ./testFramework/src/thirdPartyLibs/googletest/build/lib/libgtest.a -o bin/test
~/smartTrader $ ./bin/test
...
````
