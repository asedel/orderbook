#ifndef CWFQ_H
#define CWFQ_H

#include <atomic>
#include <cstddef>

namespace CWFQ {

  template <typename Element, size_t Size>
    class RingFifo{
  public:
    enum { Capacity = Size + 1 };

  RingFifo() : _tail(0), _head(0){}
    virtual ~RingFifo() {}

    bool push( const Element& item );
    bool pop( Element& item );

    bool wasEmpty() const;
    bool wasFull() const;
    bool isLockFree() const;

  private:
    size_t increment( size_t idx ) const;

    std::atomic <size_t> _tail;  // tail(input) index
    Element _array[Capacity];
    std::atomic<size_t>  _head; // head(output) index
  };

  template <typename Element, size_t Size>
    bool RingFifo<Element, Size>::push( const Element& item )
  {
    const auto current_tail = _tail.load( std::memory_order_relaxed );
    const auto next_tail = increment( current_tail );
    if ( next_tail != _head.load( std::memory_order_acquire ) )
    {
      _array[current_tail] = item;
      _tail.store( next_tail, std::memory_order_release );
      return true;
    }

    return false; // full queue

  }


  // Pop by Consumer can only update the head (load with relaxed, store with release)
  // the tail must be accessed with at least aquire
  template <typename Element, size_t Size>
    bool RingFifo<Element, Size>::pop( Element& item )
  {
    const auto current_head = _head.load( std::memory_order_relaxed );
    if ( current_head == _tail.load( std::memory_order_acquire ) ) {
      return false; // empty queue
    }

    item = _array[current_head];
    _head.store(increment(current_head), std::memory_order_release);
    return true;
  }

  template <typename Element, size_t Size>
    bool RingFifo<Element, Size>::wasEmpty() const
  {
    // snapshot with acceptance that this comparison operation is not atomic
    return (_head.load() == _tail.load());
  }


  // snapshot with acceptance that this comparison is not atomic
  template <typename Element, size_t Size>
    bool RingFifo<Element, Size>::wasFull() const
  {
    const auto next_tail = increment( _tail.load() );
    return ( next_tail == _head.load() );
  }


  template <typename Element, size_t Size>
    bool RingFifo<Element, Size>::isLockFree() const
  {
    return ( _tail.is_lock_free() && _head.is_lock_free() );
  }

  template <typename Element, size_t Size>
    size_t RingFifo<Element, Size>::increment( size_t idx ) const
  {
    return (idx + 1) % Capacity;
  }

}

#endif
