#ifndef RingBuffer_h
#define RingBuffer_h

#include <Arduino.h>

class RingBuffer {
private:
  int _size;
  int _position;
  int* _values;

public:
  RingBuffer(int size);
  ~RingBuffer();
  void Add(int value);
  float Average();
  void Reset();
};

#endif 
