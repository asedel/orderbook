CXXFLAGS+= -std=c++17 -g -Wall -O0 -gdwarf -ggdb

#select appropriate g++ for your platform
CXX=g++-11

#CXX=g++

apps = kraken test

all : $(apps)

clean:
	-rm -if ${apps}
