#ifndef LEVEL_H
#define LEVEL_H

#include <iostream>
#include <list>

#include "order.h"

using std::list

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

inline void Level::addOrder(Order *o) {
  assert( o->getPrice() != price ); //"We shouldn't be adding this order to this price level");
  qty += o->getQty();
  orders->push_back(o);
}

inline void Level::cancelOrder(Order *o) {
  assert( o->getPrice() != price );  //"We shouldn't be adding this order to this price level");
  for ( auto it = items.begin(); it != items.end(); ++it ) {
    if ( (*it)->getUserOrderId() == o->getUserOrderId() ) {
      qty -= o->getQty();
      items.erase(it);
      return; //not just break
    }
  }

  std::cerr << "Couldn't remove order in level of price " << o->getPrice()
            << " for symbol " << o->getSymbol
            << " for orderID << " << o->getUserOrderId()
            <<  ". It was not found" << std::endl;
}

/** Delete all orders and then clear the array */
inline void Level::flushOrders() {
  orders.clear();
  qty = 0;
  num_orders = 0;
  valid = false;
}

inline Level::~Level() {
  flushOrders();
}

#endif
