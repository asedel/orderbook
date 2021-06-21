#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <iostream>
#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;

#include "orderbook.h"

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
  unordered_map<const string, OrderBook*> book_map;
  // would be nice to use a vector if we can guarantee theyll be tight
  // and monotonicincreasing...
  unordered_map<int, Order*> orders_by_id;

};

OrderManager::OrderManager()
{}

void OrderManager::addOrder(Order *o) {
  orders_by_id[o->getUserOrderId()] = o;

  OrderBook *p = null;
  auto it = book_map.find(o->getSymbol);
  if ( it == book_map.end() ) {
    p = new OrderBook( o->getSymbol() );
    o->setBook(p);
  } else {
    p = *it;
  }
  p->addOrder(o);
}

void OrderManager::cancelOrder(Order *o) {
  auto it = orders_by_id.find(o->getUserOrderId());
  if ( it != order_by_id.end() ) {
    Order *temp = *it;
    it->getBook()->cancelOrder(temp);
    //finally delete it
    delete temp;
  }
  else {
    std::cerr << "Can't cancel order that can't be found: " << o->getUserOrderId() << "!" << endl;
    return;
  }
}

OrderManager::~OrderManager() {
  for ( OrderBook* ob : book_map ) {
    delete ob;
  }
  book_map.clear();
}

#endif
