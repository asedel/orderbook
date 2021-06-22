#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <iostream>
#include <string>
#include <vector>

using std::string;
using std::vector;
using std::cout;
using std::endl;

#include "util.h"
#include "order.h"
#include "level.h"
#include "pool.h"

class OrderManager; //fwd declare

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
  static const int DEFAULT_NUM_LEVELS = 16;
  OrderBook(const string& symbol, OrderManager *mgr=NULL, int num_levels=DEFAULT_NUM_LEVELS );

  /** Insert an Order and carry out approriate matching if need be*/
  void addOrder(Order *o);
  void cancelOrder(Order *o);
  void flushOrders();

  /** Note with more time i would maintain pointers to these that
      didnt require lookups and switches and which were not guaranteed
      to be safe to call in cases where a book was empty Luckily we
      don't allow prices of Zero and quantities of Zero so i can use
      those as sentinals however in reality this isn't workable as we
      saw in summer of 2020 the price of some oil future spreads went
      to 0 and then went negative!
   */

  int getBestBidPrice();
  int getBestBidQty();
  Level* getBestBidLevel();

  int getBestOfferPrice();
  int getBestOfferQty();
  Level* getBestOfferLevel();

private:
  const string& symbol;
  const int num_levels;
  using sorted_levels_t = vector<PriceLevel>;
  sorted_levels_t asks; //keep sorted
  sorted_levels_t bids; //keep sorted
  pool<Level, level_id_t, DEFAULT_NUM_LEVELS * 2> all_levels; //single allocation
  OrderManager* mgr;

  void executeOrder( Order *o );
  void executeMarketBuy( Order *o);
  void executeMarketSell( Order *o);
  void executeBuy( Order *o);
  void executeSell( Order *o);
  void insertOrder( Order *o, bool isTob );
  void deleteLevel( Order *o );
  void tobChange(Order *o);
  void tobChange(char side, int price, int quantity);

};

inline OrderBook::OrderBook(const string& symbol, OrderManager *mgr, int num_levels)
  : symbol(symbol)
  , num_levels(num_levels)
  , mgr(mgr)
{
  flushOrders();
}

inline int OrderBook::getBestBidPrice() {
  if ( !bids.empty() ) {
    return bids.back().l_price;
  }
  else {
    return 0;
  }
}

inline Level* OrderBook::getBestBidLevel() {
  if ( !bids.empty() ) {
    return &all_levels[bids.back().l_ptr];
  } else {
    return NULL;
  }
}

inline int OrderBook::getBestBidQty() {
  if ( !bids.empty() ) {
    return all_levels[bids.back().l_ptr].getQty();
  } else {
    return 0;
  }
}

inline int OrderBook::getBestOfferPrice() {
  if ( !asks.empty() ) {
    return asks.front().l_price;
  }
  else {
    return 0;
  }
}

inline Level* OrderBook::getBestOfferLevel() {
  if ( !bids.empty() ) {
    return &all_levels[asks.front().l_ptr];
  } else {
    return NULL;
  }
}

inline int OrderBook::getBestOfferQty() {
  if ( !asks.empty() ) {
    return all_levels[asks.front().l_ptr].getQty();
  } else {
    return 0;
  }
}

inline void OrderBook::flushOrders() {
  asks.clear();
  bids.clear();
  all_levels.clear();

  //reset
  asks.reserve(num_levels);
  bids.reserve(num_levels);
}

void OrderBook::addOrder(Order *o) {
  if ( o->getIsBuy() ) {
    // buy/bid
    if ( o->getPrice() == 0 ) {
      if ( getBestOfferPrice() != 0 ) {
        executeOrder(o);
        tobChange('S', getBestOfferPrice(), getBestOfferQty() );
      } else {
        // @TODO can't execute report no trade?
      }
    }
    else {
      if ( !bids.empty() ) {
        if ( o->getPrice() > getBestBidPrice() ) {
          if ( getBestOfferPrice() != 0 && o->getPrice() >= getBestOfferPrice() ) {
            int preBidP = getBestBidPrice();
            int preBidQ = getBestBidQty();
            int preAskP = getBestOfferPrice();
            int preAskQ = getBestOfferQty();

            executeOrder(o);

            int postBidP = getBestBidPrice();
            int postBidQ = getBestBidQty();
            int postAskP = getBestOfferPrice();
            int postAskQ = getBestOfferQty();

            if ( preBidP != postBidP || preBidQ != postBidQ ) {
              tobChange('B', postBidP, postBidQ);
            }
            if ( preAskP != postAskP || preAskQ != postAskQ ) {
              tobChange('S', postAskP, postAskQ);
            }

          } else {
            insertOrder(o, true);
          }

        } else if ( o->getPrice() == getBestBidPrice() ) {
          getBestBidLevel()->addOrder(o);
          tobChange(o);
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
      if ( getBestBidPrice() != 0 ) {
        executeOrder(o);
        tobChange('B', getBestBidPrice(), getBestBidQty() );
      } else {
        //@TODO  can't execute report no trade?
      }
    }
    else {
      if ( ! asks.empty() ) {
        if ( o->getPrice() < getBestOfferPrice() ) {
          if ( getBestBidPrice() != 0 && o->getPrice() <= getBestBidPrice() ) {
            int preBidP = getBestBidPrice();
            int preBidQ = getBestBidQty();
            int preAskP = getBestOfferPrice();
            int preAskQ = getBestOfferQty();

            executeOrder(o);

            int postBidP = getBestBidPrice();
            int postBidQ = getBestBidQty();
            int postAskP = getBestOfferPrice();
            int postAskQ = getBestOfferQty();

            if ( preBidP != postBidP || preBidQ != postBidQ ) {
              tobChange('B', postBidP, postBidQ);
            }
            if ( preAskP != postAskP || preAskQ != postAskQ ) {
              tobChange('S', postAskP, postAskQ);
            }

          } else {
            insertOrder(o, true);
          }
        } else if ( o->getPrice() == getBestOfferPrice() ) {
          getBestOfferLevel()->addOrder(o);
          tobChange(o);
        } else {
          insertOrder(o, false);
        }
      }
      else {
        insertOrder(o, true);
      }
    }
  }
}

inline void OrderBook::insertOrder(Order *order, bool tob) {
  sorted_levels_t *sorted_levels = order->getIsBuy() ? &bids : &asks;

  //Search descending since best prices are at top
  auto it = sorted_levels->end();
  bool found = false;
  while ( it-- != sorted_levels->begin() )
  {
    PriceLevel &cur_lvl = *it;
    if ( cur_lvl.l_price == order->getPrice() ) {
      order->setLevelId( cur_lvl.l_ptr );
      found = true;
      break;
    } else if ( order->getPrice() > cur_lvl.l_price ) {
      // it will be -1 if price < all prices
      break;
    }
  }
  if ( !found ) {
    level_id_t lvl_id = all_levels.alloc();
    order->setLevelId(lvl_id);
    Level& lvl = all_levels[lvl_id];
    lvl.setPrice( order->getPrice() );
    lvl.setQty( 0 );
    lvl.setValid( true );
    PriceLevel const px(order->getPrice(), lvl_id);
    ++it;
    sorted_levels->insert(it, px);
  }
  all_levels[order->getLevelId()].addOrder(order);

  if (tob) {
    tobChange(order);
  }

}

inline void OrderBook::cancelOrder(Order *order) {
  auto lvl_id = order->getLevelId();
  all_levels[lvl_id].cancelOrder(order); //removes order from list and qty
  if ( all_levels[lvl_id].getQty() == 0 ) {
    deleteLevel(order);
  }
}

//also can be called into by execute
inline void OrderBook::deleteLevel( Order *o ) {
  bool changeTOB = false;
  level_id_t lvl_id = o->getLevelId();
  int price = o->getPrice();
  sorted_levels_t *sorted_levels = NULL;

  if ( o->getIsBuy() ) {
    sorted_levels = &bids;
    if ( o->getPrice() == getBestBidPrice() ) {
      changeTOB = true;
    }
  } else {
    sorted_levels = &asks;
    if ( o->getPrice() == getBestOfferPrice() ) {
      changeTOB = true;
    }
  }

  auto it = sorted_levels->end();
  while ( it-- != sorted_levels->begin() ) {
    if ( it->l_price == price ) {
      sorted_levels->erase(it);
      break;
    }
  }
  all_levels.free(lvl_id);

  if ( changeTOB ) {
    tobChange(o);
  }
}

inline void OrderBook::executeOrder( Order *o ) {
  if ( o->getPrice() == 0 ) {
    if ( o->getIsBuy() ) {
      executeMarketBuy(o);
    } else {
      executeMarketSell(o);
    }
  } else {
    if ( o->getIsBuy() ) {
      executeBuy(o);
    } else {
      executeSell(o);
    }
  }
}

inline void OrderBook::tobChange(Order *o) {
  int price;
  int quantity;
  string p_s;
  string q_s;
  char side;

  if ( o->getIsBuy() ) {
    price = getBestBidPrice();
    quantity = getBestBidQty();
    side = 'B';
  } else {
    price = getBestOfferPrice();
    quantity = getBestOfferQty();
    side = 'S';
  }

  if ( price != 0 && quantity != 0 ) {
    p_s = std::to_string(price);
    q_s = std::to_string(quantity);
  } else {
    p_s = "-";
    q_s = "-";
  }

  cout << "B," << side << "," << p_s << "," << q_s << endl;
}

inline void OrderBook::tobChange(char side, int price, int quantity) {
  string p_s;
  string q_s;
  if ( price != 0 && quantity != 0 ) {
    p_s = std::to_string(price);
    q_s = std::to_string(quantity);
  } else {
    p_s = "-";
    q_s = "-";
  }
  cout << "B," << side << "," << p_s << "," << q_s << endl;
}

#endif
