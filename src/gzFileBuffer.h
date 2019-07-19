#pragma once

#include <iostream>
#include <fstream>
#include <algorithm>

class gzFileBuffer
{
public:
  static const size_t buffer_size = 8 * 1024;
  char buffer[buffer_size];
  gzFile file;
  char * p, *pend;
  bool eof;


  bool fill_buffer(char * buff, size_t size)
  {
    if (eof)
      return Z_NULL;

    unsigned bytes_read;

    if ((bytes_read = gzread(file, buff, size)) == Z_NULL)
    {
      eof=true;
      return false;
    }

    p = buffer;
    pend = buff + bytes_read;
    return true;
  }

public:

  gzFileBuffer(gzFile & file)
    : file(file), p(0), eof(false)
  {
    
  }

  virtual ~gzFileBuffer()
  {
  }

  int gets(char * buff, size_t size)
  {
    if ( p == 0 && !fill_buffer(buffer, buffer_size) )
      return Z_NULL;
  
    char * pnl = p;
    while ((pnl != pend) && (*pnl != '\n'))
      pnl++;

    if (pnl == pend)
    {
      int bytes = pend-p;
      
      if (bytes)
	memmove(buffer, p, bytes);
      
      if (!fill_buffer(buffer + bytes, buffer_size - bytes))
      {
	if (p!=pend)
	{
	  size_t bytes = pend - p;
	  strncpy(buff, p, bytes);
	  buff[bytes] = 0;
	  p = pend;
	  return bytes;
	}
	else
	  return Z_NULL; // todo, fix last line problem...
      }
      return gets(buff, size);
    }

    size_t bytes = std::min((long)(pnl - p), (long)(size));
    strncpy(buff, p, bytes);
    buff[bytes] = 0;
    p = pnl + 1;
    
    return bytes;
  }

};
