#include "RingBuffer.h"

RingBuffer::RingBuffer(int size) : _size(size), _position(0)
{
  _values = new int[_size];
  for (int i=0; i < _size; i++)
    _values[i] = 500;
}

RingBuffer::~RingBuffer()
{
  delete[] _values;
}

void RingBuffer::Add(int value)
{
  _values[_position] = value;
  if (++_position >= _size)
    _position = 0;
}

float RingBuffer::Average()
{
  uint64_t total = 0.0;
  int items = 0;
  
  for (int i=0; i<_size; i++)
  {
    if (_values[i] != 500)
    {
      total += _values[i];
      items++;
    }
  } 

  return (int)(total / items);
}

void RingBuffer::Reset()
{
   for (int i=0; i < _size; i++)
    _values[i] = 500; 
}
