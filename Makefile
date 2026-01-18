CC := g++
CFLAGS := -Werror -Weffc++ -Wconversion -Wsign-conversion -g -Wall -Wextra -std=c++20 -O0 -pedantic-errors 
SOURCES := $(wildcard ./testFramework/src/*.cpp)
GTEST_ARCHIVES := $(wildcard ./testFramework/src/thirdPartyLibs/googletest/build/lib/*.a)

TARGET := ./bin/test
INTERNAL_INCLUDE := -I./testFramework/include
GTEST_INCLUDE := -I./testFramework/src/thirdPartyLibs/googletest/googletest/include
ORDERBOOK_SRC := -I./orderbook/src
ORDERBOOK_INCLUDE := -I./orderbook/include


$(TARGET): $(SOURCES)
	$(CC) $(CFLAGS) $^  $(INTERNAL_INCLUDE) $(GTEST_INCLUDE) $(ORDERBOOK_INCLUDE) $(ORDERBOOK_SRC) $(GTEST_ARCHIVES) -o $@


clean:
	rm  $(TARGET) 

.PHONY: clean
