#include <string>

#include "order.h"

using std::string;

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
  OrderParser();

  // return via RVOO
  Order parse(const string& input);
private:
}
