CXXFLAGS+= -std=c++17 -g -Wall -O0 -ggdb -gdwarf

#select appropriate g++ for your platform
CXX=g++-11

#CXX=g++

CXXFLAGS += -I/usr/local/include

apps = kraken test

all : $(apps)

clean:
	-rm -if ${apps}
