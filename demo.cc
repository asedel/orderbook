/* Main file for My Order Book Programming Exercise */

//system headers
#include <string>
#include <iostream>
#include <sstream>
#include <fstream>

//my headers
#include "ordermanager.h"
#include "orderparser.h"

using std::cin;
using std::cout;
using std::endl;
using std::string;

int main(int c, char **argv) {

  cout << "Welcome to Order Mgmt Demo program!" << endl;

  OrderManager order_mgr;

  //read input
  string input = "foo";

  Order *order = OrderParser::parse(input);
  order_mgr.handle(order);

  return 0;
}
