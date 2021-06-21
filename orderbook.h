#ifndef ORDERBOOK_H
#define ORDERBOOK_H

#include <string>
#include <vector>
#include <list>
#include <unordered_map>

using std::vector
using std::list

#include "order.h"

/** Represent a level in the book

    A level is a number of orders sorted by time for a given symbol
*/
class Level {
public:
  Level(int price=0)
    : valid(false)
    , price(price)
    {}

  ~Level();

  /* mutators */
  void addOrder(Order *o);
  void cancelOrder(Order *o);
  void flushOrders();
  void setPrice(int price);
  void setValid(bool b);

  /* accessors */
  int getQty() const { return qty; }
  int getPrice() const { return price; }
  int getNumOrders() const { return orders.size() }
  int getValid() const { return valid }

private:
  bool valid;
  int qty; // total qty at level
  int price; //price of level
  List<Order *> orders; // kept sorted by time
};

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
  DEFAULT_NUM_LEVELS
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

class OrderManager {
public:
  OrderManager();
  ~OrderManager();

  void addOrder(Order *o);
  void cancelOrder(Order *o);
  void flushOrders();

private:
  //could speed this up with symbol to int mapping so that i could use
  //book id's would generally do this by getting all symbols and
  //enumerating
  std::unordered_map<const std::string, OrderBook*> book_map;
  std::unordered_map<int, Order*> orders_by_id; // would be nice to
                                                // use a vector if we
                                                // can guarantee
                                                // theyll be tight and
                                                // monotonicincreasing...
#endif
