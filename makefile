CXXFLAGS+= -std=c++17 -g -Wall -O0 -ggdb #-gdwarf

#select appropriate g++ for your platform
CXX=g++-11

#CXX=g++

CXXFLAGS += -I/usr/local/include

apps = kraken test
test : order.h orderparser.h
kraken: order.h orderparser.h ordermanager.h orderbook.h level.h pool.h
all : $(apps)

clean:
	-rm -rf ${apps} *.dSYM
