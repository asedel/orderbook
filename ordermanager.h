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

  void handle(Order *o);
  void ackOrder(Order *o);
  void addOrder(Order *o);
  void cancelOrder(Order *o);
  void flushOrders();

private:
  //could speed this up with symbol to int mapping so that i could use
  //book id's would generally do this by getting all symbols and
  //enumerating
  unordered_map<string, OrderBook*> book_map;
  // would be nice to use a vector if we can guarantee theyll be tight
  // and monotonicincreasing...
  unordered_map<int, Order*> orders_by_id;

};

OrderManager::OrderManager() {}

inline void OrderManager::handle(Order *order) {
  switch ( order->getType() ) {
    case Order::eFLUSH:
      flushOrders();
      break;
    case Order::eCANCEL:
      ackOrder(order);
      cancelOrder(order);
      break;
    case Order::eNEW:
      ackOrder(order);
      addOrder(order);
      break;
    default: //unreachable as its prehandled
      std::cerr << "Unhandled invalid order type" << std::endl;
      break;
  }
}

inline void OrderManager::addOrder(Order *o) {
  orders_by_id[o->getUserOrderId()] = o;

  OrderBook *p = NULL;
  auto it = book_map.find(o->getSymbol());
  if ( it == book_map.end() ) {
    p = new OrderBook( o->getSymbol(), this );
    o->setBook(p);
  } else {
    p = it->second;
  }
  p->addOrder(o);
}

inline void OrderManager::cancelOrder(Order *o) {
  auto it = orders_by_id.find(o->getUserOrderId());
  if ( it != orders_by_id.end() ) {
    Order *temp = it->second;
    temp->getBook()->cancelOrder(temp);
    //finally delete it
    delete temp;
  }
  else {
    std::cerr << "Can't cancel order that can't be found: " << o->getUserOrderId() << "!" << std::endl;
    return;
  }
}

inline void OrderManager::flushOrders() {
  for ( auto it : book_map ) {
    (it.second)->flushOrders();
  }

  //clear the memory
  for ( auto it : orders_by_id ) {
    delete it.second;
  }
  orders_by_id.clear();
}

inline OrderManager::~OrderManager() {
  for ( auto it : book_map ) {
    delete it.second;
  }
  book_map.clear();
}

inline void OrderManager::ackOrder(Order *o) {
  //@todo acknowledge the order
}

#endif
