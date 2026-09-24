#pragma once
#include <iostream>
#include <string>
#include <string_view>
#include <filesystem>
#include <functional>
#include <source_location>
#include <stacktrace>
#include <exception>
#include <error.h>
#include <simdjson.h>

#if !defined( _ANI_ERROR_H )

#define debug(expr) debug_call( [&]() -> decltype( auto ) { return (expr); }, #expr, std::source_location::current( ) )

namespace error {
class excpt : public std::exception {
public:
  const char* what( ) const throw( ) override;
public:
  excpt( );
  // excpt( excpt&& e );
  excpt( const char* str );
  excpt( const std::string& str );
  excpt( ssize_t code, const std::string& u_str, const std::string& e_str );
  excpt( ssize_t code, const std::string& u_err, const std::string& msg, const std::string& e_str );
  excpt( std::string&& str );
  ~excpt( );
public:
  std::string err_str;
  std::string usr_str;
  std::string msg;
  std::string path;
  ssize_t err_code;
  std::string err;
};
template <typename F>
decltype(auto) debug_call( 
  F&& func, 
  std::string_view expression, std::source_location loc = std::source_location::current( ) ) {
  try {
    return std::forward<F>( func )( );
  }
  catch ( const simdjson::simdjson_error& e ) {
    std::cerr << "[SIMDJSON] " << loc.file_name( ) << ':' << loc.line( ) << " [" << expression << "] " << e.what( ) << '\n';
    throw;
  }
  catch ( const std::exception& e ) {
    std::cerr << loc.file_name( ) << ':' << loc.line( ) << " [" << expression << "] " << e.what( ) << '\n';
    throw;
  }
  }

  template<typename T>
  T require( simdjson::simdjson_result<T>&& result, std::string_view path )
  {
    if ( !result.error( ) )
      return std::move( result ).value( );

    if ( result.error( ) == simdjson::NO_SUCH_FIELD )
    {
      throw error::excpt( result.error( ), std::string( "missing required field '" ) + path + "'", simdjson::error_message( result.error( ) ) );
    }

    if ( result.error( ) == simdjson::INCORRECT_TYPE )
    {
      throw error::excpt( -1, std::string( "incorrect type for field '" ) + path + "'", simdjson::error_message( result.error( ) ) );
    }

    throw error::excpt( -1, std::string( "invalid field '" ) + path + "'", simdjson::error_message( result.error( ) ) );
  }
}

#define _ANI_ERROR_H
#endif
