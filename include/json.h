#include <yyjson.h>
#include <iostream>
#include <string>
#include <string_view>

#if !defined( _ANI_JSON_CONSTRUCTOR_H )

class json {
public:
  enum class json_type {
    json = 0,
    array = 1
  };
public:
  void init_arr( yyjson_mut_doc* doc );
  void init_json( yyjson_mut_doc* doc );
  void add_str( std::string_view key, std::string_view value );
  void add_str( std::string_view value );
  void add_json( std::string_view key, json* _json );
  void add_json( json* _json );
  void add_int( std::string_view key, int value );
  void add_arr( std::string_view key, json* _json );
  char* get_json( size_t& len );
public:
  json( );
  ~json( );
public:
  json_type type;
  yyjson_mut_doc* doc;
  yyjson_mut_val* root;
};

#define _ANI_JSON_CONSTRUCTOR_H
#endif