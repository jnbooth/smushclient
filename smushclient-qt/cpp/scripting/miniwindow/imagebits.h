#pragma once

#include "channel.h"
#include <QtGui/QImage>

class ImageBitsRange
{
public:
  using difference_type = ptrdiff_t;

  ImageBitsRange(QImage& image, ColorChannel channel);

  class iterator
  {
    friend class ImageBitsRange;

  public:
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    uchar& operator*() const { return *i; }
    uchar* operator->() const { return i; }
    uchar& operator[](difference_type j) const
    {
      return *(i + (j << bitshift));
    }
    constexpr bool operator==(iterator o) const { return i == o.i; }
    constexpr bool operator!=(iterator o) const { return i != o.i; }
    constexpr bool operator<(iterator other) const { return i < other.i; }
    constexpr bool operator<=(iterator other) const { return i <= other.i; }
    constexpr bool operator>(iterator other) const { return i > other.i; }
    constexpr bool operator>=(iterator other) const { return i >= other.i; }
    iterator& operator++()
    {
      i += increment;
      return *this;
    }
    iterator operator++(int)
    {
      auto copy = *this;
      ++*this;
      return copy;
    }
    iterator& operator--()
    {
      i -= increment;
      return *this;
    }
    iterator operator--(int)
    {
      auto copy = *this;
      --*this;
      return copy;
    }
    qsizetype operator-(iterator j) const { return (i - j.i) >> bitshift; }
    iterator& operator+=(difference_type j)
    {
      i += j << bitshift;
      return *this;
    }
    iterator& operator-=(difference_type j)
    {
      i -= j << bitshift;
      return *this;
    }
    iterator operator+(difference_type j) const
    {
      return iterator(i + (j << bitshift), bitshift);
    }
    iterator operator-(difference_type j) const
    {
      return iterator(i - (j << bitshift), bitshift);
    }

    /*
    friend iterator operator+(difference_type j, iterator k)
    {
      return k + j << bitshift;
    }
    */

    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

  private:
    constexpr iterator(uchar* i, int bitshift)
      : i(i)
      , bitshift(bitshift)
      , increment(1 << bitshift)
    {
    }

    uchar* i;
    int bitshift;
    int increment;
  };

  iterator begin() { return iterator(beginPtr, bitshift); }
  iterator end() { return iterator(endPtr, bitshift); }

private:
  uchar* beginPtr;
  uchar* endPtr;
  int bitshift;
};
