#ifndef SERIALANDBUFFER_H
#define SERIALANDBUFFER_H

#include <CircularBuffer.h>
#include <Print.h>

const constexpr size_t max_chars_in_buffer = 4192;
using LogBuffer = CircularBuffer<uint8_t, max_chars_in_buffer>;

class SerialAndBuffer : public Print {
 public:
  // Ownership of `printable` remains with the caller.
  // To use this with Serial, simply allocate the object as
  //   auto serial_and_buffer = SerialAndBuffer(&Serial);
  SerialAndBuffer(Print* printable) : printable_(printable) {}

  virtual size_t write(uint8_t character) {
    buffer_.push(character);
    return printable_->write(character);
  }

  CircularBuffer<uint8_t, max_chars_in_buffer>* GetBuffer() { return &buffer_; }

  Print* printable_;
  LogBuffer buffer_;
};
#endif  // SERIALANDBUFFER_H