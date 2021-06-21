#include <string>
#include <vector>
#include <boost/algorithm/string.hpp>

#include "order.h"

using std::string;
using std::vector;

/**
   OrderParser takes a line of entry and returns a new order of the approriate type

   input formats:
   New order   : 'N', user(int), symbol(string), price(int), qty(int), side(B or S), userOrderId(int)
   Cancel Order: 'C', userOrderId(int)
   Flush OB:   'F', <None>

   Notes: price 0 is for market order, non zero is limit order
   Between scenarios flush order books

*/
class OrderParser {
public:
  OrderParser() {};

  // return via RVOO
  Order parse(const string& input) const;
private:
};

Order OrderParser::parse(const string& input) const {
  vector<string> strs;
  boost::split(strs, input, boost::is_any_of(","));

  #include <iostream>
  using std::cout;
  using std::endl;
  cout << "found strings: " << endl;
  for (auto i : strs) {
    cout << i << endl;
  }

  return Order('F');
}
