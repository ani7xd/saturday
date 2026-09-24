#include <string>
#include <string_view>
#include <iostream>

#if !defined ( _ANI_STREAM_PARSER_H ) 

class stream_parser {
public:
  
public:
  stream_parser( );
  ~stream_parser( );
public:
  size_t pos;
  bool pending_char;
  bool styling;
};

#define _ANI_STREAM_PARSER_H
#endif