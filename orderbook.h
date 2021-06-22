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
    usually little no to activity takes place.. at least once the book
    size approaches >256 as performance starts to suffer at O(n)

    another potential optimization because we often see inner levels
    flicker in and out is to mark a level as invalid but not actually
    'delete it completely' so that when it kicks back in we dont have
    to rejuggle all of the levels.  thus it is usually beneficial to
    keep a padding of 'level' objects both above and below the core
    cluster to avoid unneeded shuffling when inside and outside levels
    flicker in and out. this would thus utilize a valid flag and not
    marking the innermost and outermost levels as 'free' immediately
    but rather into a secondary free pool to only use if none other
    are available. this would require reworking some bits of how the
    pool works but would be useful since in most markets most of the
    activity is at the center of the book and those levels are
    constantly coming in and out of existence, and the work to move
    the sorted arrays around is basically just thrashing and wasted work.

    also note that for these size books a linear search will
    outperform binary search and will be more friendly to cache and
    TLB and branch prediction.

    Orderbook is the major workhorse since the Level sides are
    basically FIFO stacks for the given price of order lookup.

    by fixing our sizes the vectors effectively become arrays offering O(1) lookup,
    and avoids resizing penalties which would periodically disrupt latency

    another enhancment would be to force orderID's to be consecutive
    and constrained so that we could use a vector for the storage
    instead of a hash structure; similary having symbolID's that are
    tightly banded together based on knowledge of all tradable symbols
    could improve performance as well.

    along those lines of the orderID's being tightly constrained we
    could look into using a circular buffer for id's so as to resuse
    the space from a dead order ID, but to do this we would have to
    have a notion of how many "live orders" we would want to support
    so as to properly size the circular buffer.

    also note that all publishing is deffered onto the secondary
    thread by using a shared memory queue so another constraint there
    would be performance tuning to see how big we need to make that
    queue so we don't get backed up, but dont waste too much excessive
    memory.  unfortunately we do have to perform a copy of the
    messages because the data sources they derive from are likely to
    potentially change before the information is published since most
    likely in reality we'd not be publishing to disk but back across a
    wire.

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
  boolean isValid() { return tob_bid && tob_ask; }

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
  void insertOrder(Order *o, bool isTob);

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
            insertOrder(o, true);
          }
        } else if ( o->getPrice() == getBestBidPrice() ) {
          tob_bid->addOrder(o);
        } else {
          insertOrder(o, false);
        }
      }
      else {
        insertOrder(o, true);
      }
    }
  }
  else {
    //its a sell/ask
    if ( o->getPrice() == 0 ) {
      if ( tob_bid ) {
        executeOrder(o);
      } else {
        // can't execute report no trade
      }
    }
    else {
      if ( tob_ask ) {
        if ( o->getPrice() < getBestOfferPrice() ) {
          if ( tob_bid && o->getPrice() <= getBestBidPrice() ) {
            executeOrder(o);
          } else {
            insertOrder(o, true);
          }
        } else if ( o->getPrice() == getBestOfferPrice() ) {
          tob_ask->addOrder(o);
        } else {
          insertOrder(o, false)
        }
      }
      else {
        insertOrder(o, true);
      }
    }
  }
}

inline void OrderBook::insertOrder(Order *order, bool tob)
{
  sorted_levels_t *sorted_levels = order->getIsBuy() ? &bids : &asks;

  //Search descending since best prices are at top
  auto insertion = sorted_levels->end();
  bool found = false;
  while ( insertion-- != sorted_levels.begin() )
  {
    PriceLevel &curprice = *insertion;
    if ( curprice.l_price == order->getPrice() ) {
      order->setLevelId( curprice.l_ptr );
      found = true;
      break;
    } else if ( order->getPrice() > curprice.l_price ) {
      // insertion will be -1 if price < all prices
      break;
    }
  }
  if ( !found ) {
    auto lvl_id = all_levels.alloc();
    order->setLevelId(lvl_id);
    Level& lvl = &all_levels[lvl_id];
    lvl.setPrice( order->getPrice() );
    lvl.setQty( 0 );
    lvl.setValid( true );
    PriceLevel const px(order->getPrice(), lvl_id);
    ++insertion;
    sorted_levels->insert(insertion_point, px);
  }
  all_levels[order->getLevelId()].addOrder(order);

  if (tob) {
    //@TODO PUBLISH TOB CHANGE
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
