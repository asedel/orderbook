#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <iostream>

#include "util.h"
#include "oexception.h"

using std::string;

/** Order class models an Order object

    Order is a universal spanning all the different types Given the
    limited number of subtypes(3) I opted for a single universal
    instead of polymorphic concrete subtypes for speed and efficiency;
    and as in the case of new order where there is storage involved we
    have no extra fields being stored
*/

//fwd declare for pointer
class OrderBook;

class Order {
public:
  enum OrderType {
    eINVALID = 0,
    eNEW = 1,
    eCANCEL = 2,
    eFLUSH = 3,

    eLAST
  };

  /* requires checking value returned to avoid exceptions for speed */
  static OrderType GetOrderType(char input) {
    switch (input) {
      case 'N':
        return eNEW;
      case 'C':
        return eCANCEL;
      case 'F':
        return eFLUSH;
      default:
        return eINVALID;
    }
  }

private:
  //Arranged for optimal storage packing so that all members are as alligned as possible
  //for further performance could use gcc Packed instructions
  int userOrderId;
  int user;
  int price;
  int qty;
  level_id_t levelId;
  OrderType otype;
  bool isBuy;
  OrderBook *obook;
  string symbol;
public:

  static Order* buildOrder(char otype, int user_oid=0, int user_id=0, int o_price=0, int o_qty=0, bool o_side=false, string symbol="");
  static Order* buildOrder(OrderType otype, int user_oid=0, int user_id=0, int o_price=0, int o_qty=0, bool o_side=false, string symbol="");

  // These should only be used by tests and not publicly
  //universal constructor through default values
  //perhaps split out into seperate functions
  Order();
  Order(char otype, int user_oid=0, int user_id=0, int o_price=0, int o_qty=0, bool o_side=false, string symbol="");
  Order(OrderType ot, int user_oid=0, int user_id=0, int o_price=0, int o_qty=0, bool o_side=false, string symbol="");

  OrderType getType() const {
    return otype;
  };

  void setType(char t) {
    OrderType temp = GetOrderType(t);
    if ( temp != eINVALID ) {
      otype = temp;
    } else {
      std::cerr << "Invalid Ordertype from : "<< t << "! not changing type." << std::endl;
    }
  }

  int getUserOrderId() const;
  void setUserOrderId(int uoid);

  int getUser() const;
  void setUser(int user);

  //copy the string until we determine I need something else
  string getSymbol() const;
  //take input via swap
  void setSymbol(string symbol);
  //take input via move
  void setSymbol(string&& symbol);

  int getPrice() const;
  void setPrice(int);

  int getQty() const;
  void setQty(int);

  bool getIsBuy() const;
  void setIsBuy(bool);

  OrderBook* getBook() const;
  void setBook(OrderBook *);

  level_id_t getLevelId() const;
  void setLevelId(level_id_t levelId);
};

Order::Order()
  : Order(eLAST)
{}

Order::Order( char ottype, int user_oid, int user_id, int o_price, int o_qty, bool o_side, string o_symbol )
    : Order( GetOrderType(ottype), user_oid, user_id, o_price, o_qty, o_side, o_symbol)
    {}

Order::Order( OrderType ot, int user_oid, int user_id, int o_price, int o_qty, bool o_side, string o_symbol )
  : userOrderId(user_oid)
  , user(user_id)
  , price(o_price)
  , qty(o_qty)
  , isBuy(o_side)
  , symbol(o_symbol)
{
  otype = ot;
}

inline Order* Order::buildOrder(char otype, int user_oid, int user_id, int o_price, int o_qty, bool o_side, string o_symbol ) {
  OrderType ot = GetOrderType(otype);
  return buildOrder(ot, user_oid, user_id, o_price, o_qty, o_side, o_symbol);
}

inline Order* Order::buildOrder(OrderType ot, int user_oid, int user_id, int o_price, int o_qty, bool o_side, string o_symbol ) {
  if ( ot != eINVALID && ot != eLAST ) {
    Order *p = new Order(ot, user_oid, user_id, o_price, o_qty, o_side, o_symbol);
    return p;
  } else {
    return NULL;
  }
}

inline int Order::getUserOrderId() const {
  return userOrderId;
}

inline void Order::setUserOrderId(int uoid) {
  this->userOrderId = uoid;
}

inline int Order::getUser() const {
   return user;
}

inline void Order::setUser( int user ) {
  this->user = user;
}

inline string Order::getSymbol() const {
  return symbol;
}

inline void Order::setSymbol(string symbol) {
  std::swap(this->symbol, symbol);
}

inline void Order::setSymbol(string&& symbol) {
  this->symbol = std::move(symbol);
}

inline int Order::getPrice() const {
  return price;
}

inline void Order::setPrice(int o_price) {
  price = o_price;
}

inline int Order::getQty() const {
  return qty;
}

inline void Order::setQty(int o_qty) {
  qty = o_qty;
}

inline bool Order::getIsBuy() const {
  return isBuy;
}

/**
sets value to input bool, true for buy
@param [in] o_idbuy the desired boolean state
 */
void Order::setIsBuy(bool o_isbuy) {
  isBuy = o_isbuy;
}

inline OrderBook* Order::getBook() const {
  return obook;
}

inline void Order::setBook(OrderBook *book) {
  this->obook = book;
}

inline level_id_t Order::getLevelId() const {
  return levelId;
}

inline void Order::setLevelId(level_id_t levelId) {
  this->levelId = levelId;
}

/** note we are not checking the book pointer and the level id pointer*/
bool operator==(const Order &rhs, const Order &lhs) {
  return ( rhs.getUserOrderId() == lhs.getUserOrderId() &&
           rhs.getUser() == lhs.getUser() &&
           rhs.getPrice() == lhs.getPrice() &&
           rhs.getQty() == lhs.getQty() &&
           rhs.getType() == lhs.getType() &&
           rhs.getIsBuy() == rhs.getIsBuy() &&
           rhs.getSymbol() == lhs.getSymbol()
    );
}

/** note we are not checking the book pointer and the level id pointer*/
bool operator!=(const Order& rhs, const Order& lhs) {
  return ( rhs.getUserOrderId() != lhs.getUserOrderId() ||
           rhs.getUser() != lhs.getUser() ||
           rhs.getPrice() != lhs.getPrice() ||
           rhs.getQty() != lhs.getQty() ||
           rhs.getType() != lhs.getType() ||
           rhs.getIsBuy() != rhs.getIsBuy() ||
           rhs.getSymbol() != lhs.getSymbol()
    );
}

#endif
