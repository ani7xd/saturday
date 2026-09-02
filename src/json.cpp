#include "../include/json.h"

void json::init_json( yyjson_mut_doc* doc ) {
  if ( doc == nullptr ) {
    this->doc = yyjson_mut_doc_new( nullptr );
    if ( !this->doc ) {
      std::cout << "failed to create doc...\n";
      return;
    }
    root = yyjson_mut_obj( this->doc );
    yyjson_mut_doc_set_root( this->doc, this->root );
  }
  else { 
    this->doc = doc;
    root = yyjson_mut_obj( this->doc );
  }
  type = json_type::json;
}

void json::init_arr( yyjson_mut_doc* doc ) {
  if ( doc == nullptr ) {
    this->doc = yyjson_mut_doc_new( nullptr );
    if ( !this->doc ) {
      std::cout << "failed to create doc...\n";
      return;
    }
    root = yyjson_mut_arr( this->doc );
    yyjson_mut_doc_set_root( this->doc, this->root );
  }
  else { 
    this->doc = doc;
    root = yyjson_mut_arr( this->doc );
  }
  type = json_type::array;
}

void json::add_str( std::string_view key, std::string_view value ) {
  yyjson_mut_obj_add_strncpy( this->doc, this->root, key.data( ), value.data( ), value.size( ) );        
}

void json::add_str( std::string_view value ) {
  yyjson_mut_arr_add_strncpy( this->doc, this->root, value.data( ), value.size( ) );
}

void json::add_json( std::string_view key, json* _json ) {
  yyjson_mut_obj_add_val( this->doc, this->root, key.data( ), _json->root );    
}

void json::add_json( json* _json ) {
  yyjson_mut_arr_add_val( this->root, _json->root );
}

void json::add_arr( std::string_view key, json* _json ) {
  if ( type == json_type::json )
    yyjson_mut_obj_add_val( this->doc, this->root, key.data( ), _json->root );
  else
    yyjson_mut_arr_add_val( this->root, _json->root );
}

void json::add_int( std::string_view key, int value ) {

}

char* json::get_json( size_t& len ) {
  return yyjson_mut_write( this->doc, 0, &len );
}

json::json( ) {

}

json::~json( ) {

}