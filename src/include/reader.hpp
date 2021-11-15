#ifndef __READER_H__
#define __READER_H__

#include <iostream>

template <typename __reader_type>
class reader {
  __reader_type typ;
  char *read_buffer;
  uint32_t index;
  uint32_t size;

#define F_READING    0
#define SOCK_READING 1
public:
  reader (__reader_type &reader, const uint8_t type): typ(reader)  {
    index = size = 0;
    uint32_t sz = 0;
    switch(type) {
      case F_READING: 
        fseek(reader, 0, SEEK_END);
        size = ftell(reader);
        rewind(reader);
        read_buffer = new char[size];
        sz = fread(read_buffer, 1, size, reader);
        fclose(reader);
        break;
      
      case SOCK_READING: 
        break;
      default:
        break;
    }
  }

  inline char read_next_char() {
    if (index == size) return EOF;
    return read_buffer[index++];
  }
  
  ~reader() { delete read_buffer; }
};


#endif // __READER_H__
