#ifndef ORDERPARSER_H
#define ORDERPARSER_H

#include <string>
#include <vector>
#include <iostream>
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
  // return a pointer, NULL if invalid
  static Order* parse(const string& input);
};

Order* OrderParser::parse(const string& input) {
  vector<string> strs;
  boost::split(strs, input, boost::is_any_of(","));

  Order::OrderType ot = Order::GetOrderType(strs[0][0]);
  if ( strs[0].size() != 1 || ot == Order::eINVALID || ot == Order::eLAST ) {
    std::cerr << "Invalid Order Type!" << std::endl;
    return null;
  }

  Order *result;
  switch (ot) {
    case Order::eFLUSH:
      result = buildOrder(ot);
    case Order::eCANCEL:
      result = buildOrder(ot,
                          std::stoi(strs[1]), //user
                          std::stoi(strs[2])); //uoid
    case Order::eNEW:
      result = buildOrder(ot,
                          std::stoi(strs[6]), //uoid
                          std::stoi(strs[1]), //user
                          std::stoi(strs[3]), //price
                          std::stoi(strs[4]), //qty
                          strs[5][0] == 'B' ? true : false, //side
                          strs[2] //symbol
        );
    default: //unreachable as its prehandled
      result = null;
  }

  return result;
}

#endif
