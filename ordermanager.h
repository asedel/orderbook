#ifndef ORDERMANAGER_H
#define ORDERMANAGER_H

#include <iostream>
#include <string>
#include <unordered_map>

using std::string;
using std::unordered_map;
using std::cout;
using std::endl;

#include "orderbook.h"

class OrderManager {
public:
  OrderManager();
  ~OrderManager();

  void handle(Order *o);
  void ackOrder(Order *o);
  /* make sure a is the fully filled order*/
  void publishTrade(Order *a, Order *b);
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
    book_map[o->getSymbol()] = p;
  } else {
    p = it->second;
  }
  o->setBook(p);
  p->addOrder(o);
}

inline void OrderManager::cancelOrder(Order *o) {
  auto it = orders_by_id.find(o->getUserOrderId());
  if ( it != orders_by_id.end() ) {
    Order *temp = it->second;
    temp->getBook()->cancelOrder(temp);
    //finally delete it
    delete temp;
    orders_by_id.erase(it);
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
  cout << "A," << o->getUser() << "," << o->getUserOrderId() << endl;
}

inline void OrderManager::publishTrade(Order *a, Order *b) {
  Order *buy, *sell;
  if ( a->getIsBuy() ) {
    buy = a;
    sell = b;
  } else {
    buy = b;
    sell = a;
  }

  cout << "T," << buy->getUser()  << "," << buy->getUserOrderId()
       << ","  << sell->getUser() << "," << sell->getUserOrderId()
       << "," << a->getPrice()
       << "," << a->getQty();
}

/**  These funcs from OrderBook arent defined until now because we need OrderManager defined first */

void OrderBook::executeMarketBuy( Order *o ) {
  /** Simple case of fill and kill against asks*/

  while ( o->getQty() != 0 && getBestOfferPrice() != 0 ) {
    Level *inside_level = getBestOfferLevel();
    //exhaust all the offer at this level that we can, until we have to switch levels
    while ( o->getQty() != 0 && getBestOfferLevel() == inside_level ) {
      Order* front = inside_level->getFrontOrder();
      if ( o->getQty() < front->getQty() ) {
        mgr->publishTrade(o, front);
        front->setQty( front->getQty() - o->getQty() );
        o->setQty(0); //this will break us out
      } else {
        //o is bigger so we can remove front entirely which could nuke the level
        mgr->publishTrade(front, o);
        o->setQty( o->getQty() - front->getQty() );
        mgr->cancelOrder(front);
      }
    }
  }
}

void OrderBook::executeMarketSell( Order *o ) {
  /** Simple case of fill and kill against asks*/

  while ( o->getQty() != 0 && getBestBidPrice() != 0 ) {
    Level *inside_level = getBestBidLevel();
    //exhaust all the offer at this level that we can, until we have to switch levels
    while ( o->getQty() != 0 && getBestBidLevel() == inside_level ) {
      Order* front = inside_level->getFrontOrder();
      if ( o->getQty() < front->getQty() ) {
        mgr->publishTrade(o, front);
        front->setQty( front->getQty() - o->getQty() );
        o->setQty(0); //this will break us out
      } else {
        //o is bigger so we can remove front entirely which could nuke the level
        mgr->publishTrade(front, o);
        o->setQty( o->getQty() - front->getQty() );
        mgr->cancelOrder(front);
      }
    }
  }
}

void OrderBook::executeBuy( Order *o ) {
  int p = o->getPrice();

  while ( o->getQty() != 0 && p >= getBestOfferPrice() ) {
    Level *inside_level = getBestOfferLevel();
    while ( o->getQty() !=0 && getBestOfferLevel() != 0 ) {
      Order* front = inside_level->getFrontOrder();
      if ( o->getQty() < front->getQty() ) {
        mgr->publishTrade(o, front);
        front->setQty( front->getQty() - o->getQty() );
        o->setQty(0); //this will break us out
      } else {
        //o is bigger so we can remove front entirely which could nuke the level
        mgr->publishTrade(front, o);
        o->setQty( o->getQty() - front->getQty() );
        mgr->cancelOrder(front);
      }
    }
  }
  if (o->getQty() != 0) {
    // this is the remainder order after it swept everything it could
    // match against that goes into the book
    mgr->addOrder(o);
  }
}

void OrderBook::executeSell( Order *o ) {
  int p = o->getPrice();

  while ( o->getQty() != 0 && p <= getBestBidPrice() ) {
    Level *inside_level = getBestBidLevel();
    while ( o->getQty() !=0 && getBestBidLevel() != 0 ) {
      Order* front = inside_level->getFrontOrder();
      if ( o->getQty() < front->getQty() ) {
        mgr->publishTrade(o, front);
        front->setQty( front->getQty() - o->getQty() );
        o->setQty(0); //this will break us out
      } else {
        //o is bigger so we can remove front entirely which could nuke the level
        mgr->publishTrade(front, o);
        o->setQty( o->getQty() - front->getQty() );
        mgr->cancelOrder(front);
      }
    }
  }
  if (o->getQty() != 0) {
    // this is the remainder order after it swept everything it could
    // match against that goes into the book
    mgr->addOrder(o);
  }
}

#endif
