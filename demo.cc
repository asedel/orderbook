/* Main file for My Order Book Programming Exercise */

//system headers
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>
#include <thread>

//my headers
#include "ordermanager.h"
#include "orderparser.h"
#include "cwfq.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

CWFQ::RingFifo<Order, 128> queue;

void read_file( const std::string &filename ) {
  std::fstream infile;
  infile.open( filename, std::ios::in);
  if ( !infile.is_open() ) {
    std::cerr << "ERROR COULDNT READ FILE" << endl;
    return;
  }
  string line;
  while( getline( infile, line ) ) {
    Order *x = OrderParser::parse(line);
    while ( false == queue.push(*x) ) {
      sleep(1);
    }
  }
  infile.close();
}

int main(int c, char **argv) {

  cout << "Welcome to Order Mgmt Demo program!" << endl;

  OrderManager order_mgr;
  //read input
  std::thread read_thread(read_file, argv[1]);

  int counter = 0;
  while ( true ) {
    counter = 0;
    bool result;
    Order *o = new Order; //todo get these from a pool of Orders
    while ( (result = queue.pop(*o)) == false && counter < 5 ) {
      sleep(1); //sleep until ready
      ++counter;
    }
    if ( result == false )
      break;
    }

    order_mgr.handle(o);
  }

  read_thread.join();

  return 0;
}
