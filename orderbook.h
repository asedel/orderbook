#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "order.h"
#include "level.h"

/** Models an OrderBook for a single symbol has a bid side and ask side

    OrderBook for extensibility would template on type T for the order
    type which should be Order or Order dervived Type if we wanted
    additional details or potentially more complciated architecture to
    have a templated type for each level which was templated on the
    order as well

    take parameter for number of levels to maintain assuming we are
    operating for a reasonably tight book where N <= 16 or at worst
    32.  If we wanted to support arbitrarilty large books I would
    switch implementation to a hybrid skip-list so that innermost
    levels were in a vector but skip list handled levels outside where
    usually little no to activity takes place

    another potential optimization because we often see inner levels
    flicker in and out is to mark a level as invalid but not actually
    'delete it completely' so that when it kicks back in we dont have
    to rejuggle all of the levels.  thus it is usually beneficial to
    keep a padding of 'level' objects both above and below the core
    cluster to avoid unneeded shuffling when inside and outside levels
    flicker in and out.

    also note that for these size books a linear search will
    outperform binary search and will be more friendly to cache and
    TLB and branch prediction.

**/
class OrderBook {
public:
  const int DEFAULT_NUM_LEVELS = 16;
  OrderBook(const string& symbol, int num_levels = DEFAULT_NUM_LEVELS);

  /** Insert an Order and carry out approriate matching if need be*/
  void addOrder(Order *o);
  void cancelOrder(Order *o);
  void flushOrders();

  /** clear out the book */
  flushOrders();

  int getBestBidPrice() { return bestbid_price; }
  int getBestBidQty() { return bestbid_qty; }
  int getBestOfferPrice() { return bestoffer_price; }
  int getBestOfferQty() { return bestoffer_qty; }

private:
  const string& symbol;
  int bestbid_price;
  int bestbid_qty;
  int bestoffer_price;
  int bestoffer_qty;
  vector<Level> asks;
  vector<Level> bids;

};

inline OrderBook::OrderBook(const string& symbol, int num_levels)
{
  flushOrders();
}

OrderBook::flushOrders() {
  asks.clear();
  bids.clear();

  for ( Order *o : order_by_id ) {
    delete o;
  }
  order_by_id.clear();

  //clear out
  bestbid_price = bestbid_qty = bestoffer_price = bestoffer_qty = 0;

  //reset
  asks.resize(num_levels);
  bids.resize(num_levels);
}

inline void OrderBook::addOrder(Order *o)
{
  //is it bid or ask
  if ( o->getIsBuy() ) {
    // buy/bid
    if ( o->getPrice > bestbid_price ) {
      bestbid_price = o->getPrice();
      bestbid_qty = o->getQty();
      //insert level at top
    } else if ( o->getPrice == bestbid_price ) {
      bestbid_qty += o->getQty();
      // add to top level
    }
    else {
      //find and or insert level
    }
  }
  else {
    //its a sell/ask
    if ( o->getPrice < bestoffer_price ) {
      bestoffer_price = o->getPrice();
      bestoffer_qty = o->getQty();
      //insert a level at top
    } else if ( o->getPrice == bestoffer_price ) {
      bestoffer_qty += o->getQty();
      // add to top level
    }
    else {
      //find and or insert level
    }
  }

}

#endif
