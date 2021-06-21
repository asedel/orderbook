#ifndef OEXCEPTION_H
#define OEXCEPTION_H

#include <exception>

class InvalidOTypeException : public std::exception
{
public:
  InvalidOTypeException() {}

  virtual const char* what() const throw()
  {
    return "Can't perform operation with Invalid OrderType!";
  }
};

#endif
