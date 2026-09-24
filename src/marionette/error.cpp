#include "../../include/marionette/error.h"

const char* error::excpt::what( ) const throw( ) {
  return err.c_str( );
}

error::excpt::excpt( ) {
  
}

error::excpt::excpt( ssize_t code, const std::string& u_str, const std::string& e_str  ) {
  err_code = code;
  msg = u_str;
  err_str = e_str;
  err += u_str;
  err += " : ";
  err += e_str;
}

error::excpt::excpt( ssize_t code, const std::string& u_err, const std::string& usr_msg, const std::string& e_str  ) {
  err_code = code;
  usr_str = u_err;
  msg = usr_msg;
  err_str = e_str;
}

error::excpt::excpt( const char* str ) {
  usr_str = str;
}

error::excpt::excpt( const std::string& str ) : usr_str( str ) {

}

error::excpt::excpt( std::string&& str ) : usr_str( std::move( str ) ) {
  
}

error::excpt::~excpt( ) {

}