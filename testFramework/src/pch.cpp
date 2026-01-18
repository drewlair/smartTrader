#include "pch.h"

#include "Orderbook.cpp"
#include "Order.cpp"
#include "Trade.cpp"
#include "OrderModify.cpp"


namespace googletest = ::testing;

using Price = double;
using Quantity = std::uint64_t;
using OrderId = std::uint64_t;
using Trades = std::vector<Trade>;


enum class ActionType {
    Add,
    Modify,
    Cancel,
};

/* struct we'll be storing line from the test file in */
struct Information {
    ActionType m_type;
    OrderType m_orderType;
    Side m_side;
    Price m_price;
    Quantity m_quantity;
    OrderId m_orderId;
    
};
/* lines plural, so will need a list of these*/
using Informations = std::vector<Information>;

/* result struct for storing the result line*/
struct Result {
    std::size_t m_allCount;
    std::size_t m_bidCount;
    std::size_t m_askCount;
};

/* parsing object, guess we don't have any private members so struct > class */
class InputHandler {
    //converts string into 64 bit integral
    std::uint64_t to_number(const std::string_view& str) const {
        std::int64_t value{};

        //takes does an integral conversion of the string
        std::from_chars(str.data(), str.data() + str.size(), value);
        if (value < 0)
            throw std::logic_error("value is below zero");
        
        /* the from_chars returns a std::from_chars_result, so this static_cast ensures the type reduces to uint64_t */
        return static_cast<std::uint64_t>(value);
    }

    //parser for the result line specifically
    bool try_parse_result(const std::string_view& str, Result& result) const {
        /* ensure it's the result line */
        if (str.at(0) != 'R')
            return false;

        /* like python split, get each token by the space delimeter, convert to integral, and assign to out Information struct */
        auto values = Split(str, ' ');
        result.m_allCount = to_number(values.at(1));
        result.m_bidCount = to_number(values.at(2));
        result.m_askCount = to_number(values.at(3));

        return true;
    }

    /* */
    bool try_parse_information(const std::string_view& str, Information& info) const {
        auto value = str.at(0);
        auto values = Split(str, ' ');
        if (value == 'A') {
            info.m_type = ActionType::Add;
            info.m_side = parse_side(values.at(1));
            info.m_orderType = parse_ordertype(values.at(2));
            info.m_price = parse_price(values.at(3));
            info.m_quantity = parse_quantity(values.at(4));
            info.m_orderId = parse_orderId(values.at(5));

        } else if (value == 'M') {
            info.m_type = ActionType::Modify;
            info.m_orderId = parse_orderId(values.at(1));
            info.m_side = parse_side(values.at(2));
            info.m_price = parse_price(values.at(3));
            info.m_quantity = parse_quantity(values.at(4));
            

        } else if (value == 'C') {
            info.m_type = ActionType::Cancel;
            info.m_orderId = parse_orderId(values.at(1));

        } else { return false; }

        return true;
        
    }

    /* parse the file lines into orders */
    std::vector<std::string_view> Split( const std::string_view& str, char delimeter ) const {

        /* declare the columns for storing the order actions and indexes for parsing */
        std::vector<std::string_view> columns{};
        columns.reserve(5);
        std::size_t startIndex{}, endIndex{};

        /*  find the index of the delimeter and set it as the endIndex.
            this is so we know where to stop when getting our string characters

            the first conditional does our str.find() for the delimeter index

            the second conditional also checks if the str didn't have the delimeter. this is because
            the endIndex is set to std::string::npos str.find() can't find the delimeter
            if this is the case, then we've parsed the full string and can end the loop 
        */
        while( (endIndex = str.find(delimeter, startIndex)) && endIndex != std::string::npos) {

            /* get the string and add it to the dynamic array */
            auto distance = endIndex - startIndex;
            auto column = str.substr(startIndex, distance);
            startIndex = endIndex + 1;
            columns.push_back(column);
        }
        columns.push_back(str.substr(startIndex));
        return columns;

    }

    Side parse_side(const std::string_view& str) const { 
        if (str == "B")
            return Side::Buy;
        else if (str == "S")
            return Side::Sell;
        else
            throw std::logic_error("Unknown side");
    }

    OrderType parse_ordertype( std::string_view& str ) const {
        if (str == "FillAndKill")
            return OrderType::FillAndKill;
        else if (str == "GoodTillCancel")
            return OrderType::GoodTillCancel;
        else if (str == "GoodForDay")
            return OrderType::GoodForDay;
        else if (str == "FillOrKill")
            return OrderType::FillOrKill;
        else if (str == "Market")
            return OrderType::Market;
        else 
            throw std::logic_error("Unknown order type");
    }

    Price parse_price( std::string_view& str ) const {
        if (str.empty())
            throw std::logic_error("Unknown price");

        return to_number(str);
    }

    Quantity parse_quantity( std::string_view& str ) const {
        if (str.empty())
            throw std::logic_error("Unknown Quantity");

        return to_number(str);
    }

    OrderId parse_orderId( std::string_view& str ) const {
        if (str.empty())
            throw std::logic_error("Unknown order Id");

        return to_number(str);
    }

public:

    std::tuple<Informations, Result> get_informations( const std::filesystem::path& path) const {
        /*  
            declare return info struct list and reserve some space for the appends
            it's faster to have the memory preallocated then to keep re-allocating
            based on varying data being read
        */
        Informations infos;
        infos.reserve(1000);

        /* while loop starting with getting the full file line */
        std::string line;
        std::ifstream file{ path };
        while (std::getline(file, line)) {
            if (line.empty())
                break;

            /* determine the type of data being read */
            const bool isResult = line.at(0) == 'R';
            const bool isUpdate = !isResult;

            if (isUpdate) {
                Information update;

                auto isValid = try_parse_information(line, update);
                if (!isValid)
                    throw std::logic_error("problem with the update information");
            
                infos.push_back(update);
            } else {
                
                if (!file.eof())
                    std::logic_error("Result must be at the end of the file only.");

                Result result;

                auto isValid = try_parse_result(line, result);
                if (!isValid)
                    continue;

                return { infos, result };
            }
        }
        throw std::logic_error("problem with the result data");
        

    }


};

class OrderbookTestsFixture : public googletest::TestWithParam<const char*> {

    const static inline std::filesystem::path Root { std::filesystem::current_path() };
    const static inline std::filesystem::path TestDirectory{ "./testFramework/TestDirectory" };

public:
    const static inline std::filesystem::path TestDirectoryPath{ Root / TestDirectory };
};

TEST_P(OrderbookTestsFixture, OrderbookTestSuite) {
    const auto file = OrderbookTestsFixture::TestDirectoryPath / GetParam();

    InputHandler handler;
    const auto [updates, result] = handler.get_informations(file);

    auto getOrder = [](const Information& information) {
        return std::make_shared<Order>(
            information.m_orderType,
            information.m_orderId,
            information.m_side,
            information.m_price,
            information.m_quantity
        );
    };

    auto getOrderModify = [](const Information& information) {
        return OrderModify{
            static_cast<unsigned int>(information.m_orderId),
            information.m_side,
            information.m_price,
            static_cast<unsigned int>(information.m_quantity),
        };
    };

    Orderbook orderbook;
    for (const auto& update : updates) {
        switch( update.m_type) {

            case ActionType::Add: {
                const Trades& trades = orderbook.addOrder(getOrder(update));
            }
            break;
            case ActionType::Modify: {
                const Trades& trades = orderbook.modifyOrder(getOrderModify(update));
            }
            break;
            case ActionType::Cancel: {
                orderbook.cancelOrder(static_cast<unsigned int>(update.m_orderId));
            }
            break;
            default:
                throw std::logic_error("Unsupported update type");

        }
    }

    const auto& orderbookInfos = orderbook.getOrderInfos();
    ASSERT_EQ(orderbook.Size(), result.m_allCount );
    ASSERT_EQ(orderbookInfos.getBids().size(), result.m_bidCount);
    ASSERT_EQ(orderbookInfos.getAsks().size(), result.m_askCount);


}

INSTANTIATE_TEST_SUITE_P(Tests, OrderbookTestsFixture, googletest::ValuesIn({
    "Match_GoodTillCancel.txt",
    "Match_FillAndKill.txt",
    "Match_FillOrKill_Hit.txt",
    "Match_FillOrKill_Miss.txt",
    "Cancel_Success.txt",
}));
