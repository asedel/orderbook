#ifndef ORDER_H
#define ORDER_H

#include <string>
#include <iostream>

#include "oexception.h"

using std::string;

/** Order class models an Order object

    Order is a universal spanning all the different types Given the
    limited number of subtypes(3) I opted for a single universal
    instead of polymorphic concrete subtypes for speed and efficiency;
    and as in the case of new order where there is storage involved we
    have no extra fields being stored
*/
class Order {
public:
  enum OrderType {
    eINVALID = 0,
    eNEW = 1,
    eCANCEL = 2,
    eFLUSH = 3,

    eLAST
  }

  /* requires checking value returned to avoid exceptions for speed */
  static GetOrderType(char input) {
    try {
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
  }

private:
  //Arranged for optimal storage packing so that all members are as alligned as possible
  //for further performance could use gcc Packed instructions
  int userOrderId;
  int user;
  int price;
  int qty;
  OrderType otype;
  boolean isBuy;
  string symbol;
public:
  //universal constructor through default values
  //perhaps split out into seperate functions
  Order(char otype, int user_oid=0, int user_id=0, int o_price=0, int o_qty=0, boolean o_side=false, string symbol="");
  Order(OrderType ot, int user_oid=0, int user_id=0, int o_price=0, int o_qty=0, boolean o_side=false, string symbol="");

  OrderType getType() const {
    return otype;
  }

  Order( char ot, int user_oid, int user_id, int o_price, int o_qty, boolean o_side, string o_symbol )
    : Order( GetOrderType(ot), user_oid, user_id, o_price, o_qty, o_side, o_symbol)

  Order( OrderType ot, int user_oid, int user_id, int o_price, int o_qty, boolean o_side, string o_symbol )
    : userOrderId(user_oid)
    , user(user_id)
    , price(o_price)
    , qty(o_qty)
    , isBuy(o_side),
    , symbol(o_symbol)
    {
      try {
        if ( ot == eINVALID || ot == eLAST ) {
          throw InvalidOTypeException();
        }
      }
      otype = ot;
    }

  void setType(char t) {
    OrderType temp = GetOrderType(t);
    if ( temp != eINVALID ) {
      otype = temp;
    } else {
      std::cerr << "Invalid Ordertype from : "<< t << "! not changing type." << endl;
    }
  }

  int getUserOrderId() const {
    return userOrderId;
  }

  void setUserOrderId(int uoid) {
    userOrderId = uoid;
  }

  int getUser() const;
  void setUser(int user)

  //copy the string until we determine I need something else
  string getSymbol() const;
  //take input via swap
  void setSymbol(string symbol);
  //take input via move
  void setSymbol(&&string symbol);

  int getPrice() const;
  void setPrice(int);

  int getQty() const;
  void setQty(int);

  boolean getIsBuy() const;
  void setIsBuy(boolean) const;
};



inline int getUserOrderId() const
{
  return userOrderId;
}

inline void setUserOrderId(int uoid)
{
  this->userOrderId = uoid;
}

inline getUser() const
{
   return user;
}

inline setUser( int user )
{
  this->user = user
}

inline getSymbol() const
{
  return symbol;
}

inline setSymbol(string symbol)
{
  std::swap(this->symbol, symbol);
}

inline setSymbol(string&& symbol) {
  this->symbol = std::move(symbol)
}

inline getPrice() const
{
  return price;
}

inline void setPrice(int o_price)
{
  price = o_price;
}

inline getQty() const
{
  return qty;
}

inline void setQty(int o_qty)
{
  qty = o_qty;
}

inline getIsBuy() const
{
  return isBuy;
}

/**
sets value to input bool, true for buy
@param [in] o_idbuy the desired boolean state
 */
void setIsBuy(boolean o_isbuy)
{
  isBuy = o_isbuy;
}

#endif
