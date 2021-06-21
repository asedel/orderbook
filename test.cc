#include "order.h"

#define BOOST_TEST_MODULE MyTest
#include <boost/test/included/unit_test.hpp>

BOOST_AUTO_TEST_CASE( my_test )
{
  Order myOrder1('N', 1, 2, 3, 4, true, "IBM");

  //check all fields

  Order myOrder2('C', 1,);
  //check needed fields

  Order myOrder3('F');

  // check a bad order construction failure
  BOOST_CHECK( Order::GetOrderType('X') == 0 );
  BOOST_CHECK( Order::GetOrderType('N') == 1 );
  BOOST_CHECK( Order::GetOrderType('C') == 2 );
  BOOST_CHECK( Order::GetOrderType('F') == 3 );
}
