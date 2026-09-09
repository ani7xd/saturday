#include "../include/http.h"
#include <stdexcept>


void http::request( ) {
  CURLcode ret = curl_easy_perform( this->curl );
  if ( ret != CURLE_OK) {
    throw std::runtime_error(std::string("HTTP request failed: ") + curl_easy_strerror(ret));
  //   throw std::runtime_error(
  //     std::format(
  //         "Database connection failed\n{}",
  //         std::stacktrace::current()
  //     )
  // );
  }
}

std::span<byte_t> http::get_header( ) {
  return { this->header.data( ), this->header.size( ) };
}

std::span<byte_t> http::get_body( ) {
  return { this->body.data( ), this->body.size( ) };
}

bool http::get_status_ok( ) {
  long code;
  curl_easy_getinfo( this->curl, CURLINFO_HTTP_CODE, &code );
  if ( code == 200 ) return true;
  return false;
}

void* http::get_private_data( ) {
  void* userp;
  curl_easy_getinfo( this->curl, CURLINFO_PRIVATE, &userp );
  return userp;
}

void* http::get_private_data( CURL* curl ) {
  void* userp;
  curl_easy_getinfo( curl, CURLINFO_PRIVATE, &userp );
  return userp;
}

void* http::get_body_cb_data( ) {
  return body_cb_data;
}

long http::get_status_code( ) {
  long code;
  curl_easy_getinfo( this->curl, CURLINFO_HTTP_CODE, &code );
  return code;
}

CURL* http::get_handle( ) {
  return this->curl;
}

const char* http::get_url( ) {
  return this->url.c_str( );
}

const char* http::get_current_url( ) {
  char* final_url;
  curl_easy_getinfo( this->curl, CURLINFO_EFFECTIVE_URL, &final_url );
  return final_url;
}

void http::initialize( ) {
  if ( !global_init ) { 
    CURLcode ret = curl_global_init( CURL_GLOBAL_DEFAULT );
    if ( ret != CURLE_OK )
      std::cerr << "failed to global init\n";
    else global_init = true;
  }

  curl = curl_easy_init( );
  if ( !curl ) { 
    std::cerr << "failed to init curl\n";
    return;
  }
  curl_instance_count++;
  this->method = http_option::get;
  curl_easy_setopt( curl, CURLOPT_USERAGENT, "Mozilla/5.0 (X11; Linux x86_64; rv:140.0) Gecko/20100101 Firefox/140.0" );
}

void http::set_private_data( void* userp ) {
  curl_easy_setopt( this->curl, CURLOPT_PRIVATE, userp );
}

void http::set_debug( bool_t option ) {
  if ( option == true ) {
    curl_easy_setopt( this->curl, CURLOPT_VERBOSE, 1L );
    debug = true;
  } else {
    curl_easy_setopt( this->curl, CURLOPT_VERBOSE, 0L );
    debug = false;
  }
}

void http::set_http_opt( http_option opt ) {
  switch ( opt ) {
    case http_option::get:
      break;
    case http_option::post:
      curl_easy_setopt( this->curl, CURLOPT_POST, 1L );
      break;
    default:
      break;
  }
}

void http::set_http_method_post( ) {
  if ( this->method == http_option::get )
    curl_easy_setopt( this->curl, CURLOPT_HTTPGET, 0L );
  curl_easy_setopt( this->curl, CURLOPT_POST, 1L );
  method = http_option::post;
}

void http::set_http_method_get( ) {
  if ( this->method == http_option::post )
    curl_easy_setopt( this->curl, CURLOPT_POST, 0L );
  curl_easy_setopt( this->curl, CURLOPT_HTTPGET, 1L );
  method = http_option::get;
}

void http::set_post_data( std::string_view data ) {
  curl_easy_setopt( this->curl, CURLOPT_POSTFIELDS, data.data( ) );
  curl_easy_setopt( this->curl, CURLOPT_POSTFIELDSIZE, data.size( ) );
}

void http::set_progress_info( bool value ) {
  if ( value ) {
    curl_easy_setopt( this->curl, CURLOPT_NOPROGRESS, 0L );
  } else curl_easy_setopt( this->curl, CURLOPT_NOPROGRESS, 1L );
}

void http::set_progress_cb( curl_xferinfo_callback cb ) {
  curl_easy_setopt( this->curl, CURLOPT_XFERINFOFUNCTION, cb );
}

void http::set_progress_cb_data( void* userp ) {
  curl_easy_setopt( this->curl, CURLOPT_XFERINFODATA, userp );
}

void http::set_body_write_cb( curl_write_callback cb ) {
  curl_easy_setopt( this->curl, CURLOPT_WRITEFUNCTION, cb );
}

void http::set_header_write_cb( curl_write_callback cb ) {
  curl_easy_setopt( this->curl, CURLOPT_HEADERFUNCTION, cb );
}

void http::set_body_cb_data( void* userp ) {
  body_cb_data = userp;
  curl_easy_setopt( this->curl, CURLOPT_WRITEDATA, userp );
}

void http::set_header_cb_data( void* userp ) {
  curl_easy_setopt( this->curl, CURLOPT_HEADERDATA, userp );
}

void http::set_custom_option_list( std::string_view option, std::string_view value ) {
  header_list = curl_slist_append( header_list, create_header_line( option, value ).c_str( ) );
}

void http::set_custom_options( ) {
  curl_easy_setopt( this->curl, CURLOPT_HTTPHEADER, header_list );
}

std::string http::create_header_line( std::string_view option, std::string_view value ) {
  std::string header_line;
  header_line.reserve( option.size() + 2 + value.size( ) );
  header_line.append( option );
  header_line.append( ": " );
  header_line.append( value );
  return header_line;
}

void http::set_follow_redirect( bool_t _option ) {
  this->follow_redirect = _option;
  curl_easy_setopt( this->curl, CURLOPT_FOLLOWLOCATION, _option ? 1L : 0L );
}

void http::set_url( std::string_view _url ) {
  this->url = _url;
  curl_easy_setopt( this->curl, CURLOPT_URL, this->url.c_str( ) );
}

void http::set_cookie_write_file( std::string_view _file ) {
  this->cookie_dest_file = _file;
  curl_easy_setopt( this->curl , CURLOPT_COOKIEJAR, this->cookie_dest_file.c_str( ) );
}

void http::set_cookie_read_file( std::string_view _file ) {
  this->cookie_src_file = _file;
  curl_easy_setopt( this->curl , CURLOPT_COOKIEFILE, this->cookie_src_file.c_str( ) );
}

void http::set_referrer( std::string_view _referrer ) {
  this->referrer = _referrer;
  if ( !referrer.ends_with( '/' ) ) this->referrer.push_back( '/' );
  curl_easy_setopt( this->curl, CURLOPT_REFERER , this->referrer.c_str( ) );
}

void http::set_accept_encoding( std::string_view _encoding ) {
  this->encoding = _encoding;
  curl_easy_setopt( this->curl, CURLOPT_ACCEPT_ENCODING, this->encoding.c_str( ) );
}

int http::default_progress_cb( void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow ) {
  if (dltotal > 0) {
    std::printf( "\r%.1f%% (%lld/%lld KB)", dlnow * 100.0 / dltotal, static_cast<long long>( dlnow / 1024 ), static_cast<long long>( dltotal / 1024 ) );
  } else {
    std::printf( "\rDownloaded %lld KB", static_cast<long long>( dlnow / 1024 ) );
  }

  std::fflush(stdout);
  return 0;
}

size_t http::default_header_callback( char* ptr, size_t size, size_t nmemb, void* userdatap ) {
  auto buffer = reinterpret_cast<std::vector<byte_t>*>( userdatap );
  buffer->insert( buffer->end( ), ptr, ptr + ( size * nmemb ) );
  return size * nmemb;
}

size_t http::default_write_callback( char* ptr, size_t size, size_t nmemb, void* userdatap ) {
  auto buffer = reinterpret_cast<std::vector<byte_t>*>( userdatap );
  buffer->insert( buffer->end( ), ptr, ptr + ( size * nmemb ) );
  return size * nmemb;
}

http::http( ) {
  curl = nullptr;
  local_init = false;
  header_list = nullptr;
}

http::~http( ) {
  if ( curl ) {
    curl_easy_cleanup( curl );
    curl_instance_count--;
  }
  if ( header_list != nullptr ) curl_slist_free_all( header_list );
  if ( global_init && curl_instance_count == 0 ) {
    curl_global_cleanup( );
    global_init = false;
  }
}
std::string http::get_content_type() {
  char* type = nullptr;
  curl_easy_getinfo(curl, CURLINFO_CONTENT_TYPE, &type);
  return type ? type : "";
}
