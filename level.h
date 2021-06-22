#ifndef LEVEL_H
#define LEVEL_H

#include <iostream>
#include <list>
#include <cassert>

#include "util.h"
#include "order.h"

using std::list;

/**
    price level is a smaller class we'll keep sorted to tie prices into full levels
 */
class PriceLevel {
public:
  PriceLevel( int price, level_id_t lid )
    : l_price(price)
    , l_ptr(lid)
    {}

  int l_price;
  level_id_t l_ptr;
};

bool operator>(const PriceLevel &a, const PriceLevel& b) {
  return a.l_price > b.l_price;
}

/** Represent a level in the book

    A level is a number of orders sorted by time for a given symbol
*/
class Level {
public:
  Level(int price=0, int qty=0)
    : valid(false)
    , price(price)
    , qty(qty)
    {}

  ~Level();

  /* mutators */
  void addOrder(Order *o);
  void cancelOrder(Order *o);
  void reduceOrder(Order *o, int qty);
  void flushOrders();
  void setPrice(int price);
  void setQty(int qty);
  void setValid(bool b); //reserved for future usage..

  /* accessors */
  int getQty() const { return qty; }
  int getPrice() const { return price; }
  int getNumOrders() const { return orders.size(); }
  int getValid() const { return valid; }
  Order* getFrontOrder() { return orders.front(); }

private:
  bool valid;
  int price; //price of level
  int qty; // total qty at level
  list<Order *> orders; // kept sorted by time
};

inline void Level::setPrice(int price) {
  this->price = price;
}

inline void Level::setQty(int qty) {
  this->qty = qty;
}

inline void Level::setValid(bool valid) {
  this->valid = valid;
}

inline void Level::addOrder(Order *o) {
  assert( o->getPrice() != price ); //"We shouldn't be adding this order to this price level");
  qty += o->getQty();
  orders.push_back(o);
}

inline void Level::cancelOrder(Order *o) {
  assert( o->getPrice() != price );  //"We shouldn't be adding this order to this price level");
  for ( auto it = orders.begin(); it != orders.end(); ++it ) {
    if ( (*it)->getUserOrderId() == o->getUserOrderId() ) {
      qty -= o->getQty();
      orders.erase(it);
      return; //not just break
    }
  }

  std::cerr << "Couldn't remove order in level of price " << o->getPrice()
            << " for symbol " << o->getSymbol()
            << " for orderID << " << o->getUserOrderId()
            << ". It was not found" << std::endl;
}

/** Delete all orders and then clear the array */
inline void Level::flushOrders() {
  orders.clear();
  qty = 0;
  valid = false;
}

inline Level::~Level() {
  flushOrders();
}

#endif
