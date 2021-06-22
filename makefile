CXXFLAGS+= -std=c++17 -g -Wall -O0 -ggdb #-gdwarf

#select appropriate g++ for your platform
CXX=g++-11

#CXX=g++

CXXFLAGS += -I/usr/local/include

apps = demo test bsocket
all : ${apps}
test : util.h order.h orderparser.h ordermanager.h orderbook.h level.h
demo: util.h order.h orderparser.h ordermanager.h orderbook.h level.h pool.h cwfq.h
bsocket:

all : $(apps)

clean:
	-rm -rf ${apps} *.dSYM
