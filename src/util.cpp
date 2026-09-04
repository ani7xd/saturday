#include "../include/util.h"

bool utils::str_cmp( std::string_view str1, std::string_view str2 ) {
  if ( str1.size( ) != str2.size( ) ) return false;
  return ( memcmp( str1.data( ), str2.data( ), str1.size( ) ) == 0 ) ? true : false;
}
  
bool utils::str_cmp( const std::string& str1, const std::string& str2 ) {
  if ( str1.size( ) != str2.size( ) ) return false;
  return ( memcmp( str1.data( ), str2.data( ), str1.size( ) ) == 0 ) ? true : false;
}