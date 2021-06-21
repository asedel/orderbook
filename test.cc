#include "order.h"
#include "orderparser.h"

#define BOOST_TEST_MODULE MyTest

#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE( my_test )
{
  Order myOrder1('N', 1, 2, 3, 4, true, "IBM");
  Order *pMyOrder1('N', 1, 2, 3, 4, true, "IBM");

  BOOST_CHECK( myOrder1 == *pMyOrder1 );
  delete pMyOrder1;

  //check all fields

  Order myOrder2('C', 1);
  //check needed fields

  Order myOrder3('F');

  // check a bad order construction failure
  //BOOST_CHECK( Order::GetOrderType('X') == 0 );
  BOOST_CHECK( Order::GetOrderType('N') == 1 );
  BOOST_CHECK( Order::GetOrderType('C') == 2 );
  BOOST_CHECK( Order::GetOrderType('F') == 3 );
}

BOOST_AUTO_TEST_CASE( order_parser_test )
{
  Order *a = OrderParser::parse("F");
  Order *b = OrderParser::parse("N,1,IBM,10,100,B,1");
  Order *c = OrderParser::parse("C,1,1");
  // insert more in here and check all attributes line up using getters

  // insert some corner cases involving decimals and bad cases with non ints for integers etc
  BOOST_CHECK( a->getType() == 3 );
  BOOST_CHECK( b->getType() == 2 );
  BOOST_CHECK( c->getType() == 1 );

  //@todo build them all by hand and compare them with equality operators

  delete a, b, c;
}
