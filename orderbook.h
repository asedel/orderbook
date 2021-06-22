#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <string>
#include <vector>

using std::string;
using std::vector;

#include "order.h"
#include "level.h"
#include "pool.h"

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
  typedef PriceLevel::level_id_t level_id_t;
  static const int DEFAULT_NUM_LEVELS = 16;
  OrderBook(const string& symbol, int num_levels = DEFAULT_NUM_LEVELS);

  /** Insert an Order and carry out approriate matching if need be*/
  void addOrder(Order *o);
  void cancelOrder(Order *o);
  void flushOrders();

  /** Note you cant call these until both sides of book are valid
      I didn't want to build the checks in here because 99% of the time both sides should have entries
      and handling the nonconforming cases isn't attrocious
   */
  int getBestBidPrice() { return tob_bid->getPrice(); }
  int getBestBidQty() { return tob_bid->getQty(); }
  int getBestOfferPrice() { return tob_ask->getPrice(); }
  int getBestOfferQty() { return tob_ask->getQty(); }

private:
  const string& symbol;
  const int num_levels;
  Level *tob_bid;
  Level *tob_ask;
  using sorted_levels_t = vector<PriceLevel>;
  sorted_levels_t asks; //keep sorted
  sorted_levels_t bids; //keep sorted
  pool<Level, level_id_t, DEFAULT_NUM_LEVELS * 2> all_levels; //single allocation

  void executeOrder(Order *o);

};

inline OrderBook::OrderBook(const string& symbol, int num_levels)
  : symbol(symbol)
  , num_levels(num_levels)
{
  flushOrders();
}

inline void OrderBook::flushOrders() {
  asks.clear();
  bids.clear();
  all_levels.clear();

  //reset
  asks.reserve(num_levels);
  bids.reserve(num_levels);
}

inline void OrderBook::addOrder(Order *o)
{
  /**
  {
    sorted_levels_t *sorted_levels = o->getIsBuy() ? &bids : &asks;

    //Search descending since best prices are at top
    auto insertion = sorted_levels->end();
    bool found = false;
    while ( insertion-- != sorted_levels.begin() )
    {
      PriceLevel &curprice = *insertion;
      if ( curprice.l_price == o->getPrice() ) {
        o->setLevelId( curprice.l_ptr );
        found = true;
        break;
      } else if ( o->getPrice() > curprice.l_price ) {
        // insertion will be -1 if price < all prices
        break;
      }
    }
  }
  */

  if ( o->getIsBuy() ) {
    // buy/bid
    if ( o->getPrice() == 0 ) {
      if ( tob_ask ) {
        executeOrder(o);
      } else {
        // can't execute report no trade
      }
    }
    else {
      if ( tob_bid ) {
        if ( o->getPrice() > getBestBidPrice() ) {
          if ( tob_ask && o->getPrice() >= getBestOfferPrice() ) {
            executeOrder(o);
          } else {
            //@todo top insert logic
          }
        } else if ( o->getPrice() == getBestBidPrice() ) {
          tob_bid->addOrder(o);
        } else {
          //@todo find and insert logic
        }
      }
      else {
        //insert level
      }
    }
  }
  else {
    //its a sell/ask
    if ( o->getPrice() == 0 ) {
      if ( tob_bid ) {
        //@todo execution
      } else {
        // can't execute report no trade
      }
    }
    else {
      if ( tob_ask ) {
        if ( o->getPrice() < getBestOfferPrice() ) {
          if ( tob_bid && o->getPrice() <= getBestBidPrice() ) {
            //@todo execution
          } else {
            //@todo top insert logic
          }
        } else if ( o->getPrice() == getBestOfferPrice() ) {
          tob_ask->addOrder(o);
        } else {
          //@todo find and insert logic
        }
      }
      else {
        //insert level
      }
    }
  }
}

inline void OrderBook::cancelOrder(Order *o)
{
  //need to get to the level so we can remove it
  //@TODO!!
}

inline void OrderBook::executeOrder(Order *o)
{
  //@TODO
}

#endif
