#include <curl/curl.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <string>
#include <string_view>
#include <vector>
#include <span>
#include <atomic>
#include <queue>

#if !defined(_ANI_HTTP_H)

typedef bool bool_t;
typedef unsigned char byte_t;

class http_mutli;
static std::atomic_bool global_init = false;
static std::atomic_size_t curl_instance_count = 0;

class http {
public:
  http( );
  ~http( );
  struct options {
    std::string url;
    std::string user_agent;
    bool_t follow_redirect;
    uint64_t timeout;
    std::string referrer;
    std::string cookie_dest_file;
    std::string cookie_src_file;  
    std::string encoding;
  };
  enum class http_option {
    get = 0,
    post = 1,
    put = 2
  };
  friend class http_multi;
public:
  void initialize( );
  void set_debug( bool_t );
  static size_t default_write_callback( char* ptr, size_t size, size_t nmemb, void* userdatap );
  static size_t default_header_callback( char* ptr, size_t size, size_t nmemb, void* userdatap );
  static int default_progress_cb( void* clientp, curl_off_t dltotal, curl_off_t dlnow, curl_off_t ultotal, curl_off_t ulnow );
  void set_follow_redirect( bool_t _option );
  void set_url( std::string_view _url ); 
  void request( );
  void set_private_data( void* userp );
  void set_http_opt( http_option opt );
  void set_http_method_post( );
  void set_http_method_get( );
  void set_post_data( std::string_view data );
  void set_cookie_write_file( std::string_view _file );
  void set_cookie_read_file( std::string_view _file );
  void set_referrer( std::string_view _referrer );
  void set_accept_encoding( std::string_view encoding );
  void set_body_write_cb( curl_write_callback );
  void set_header_write_cb( curl_write_callback );
  void set_body_cb_data( void* );
  void set_header_cb_data( void* );
  void set_progress_info( bool value );
  void set_progress_cb( curl_xferinfo_callback );
  void set_progress_cb_data( void* userp );
  void set_custom_option_list( std::string_view option, std::string_view value );
  void set_custom_options( );
  [[nodiscard]]
  static std::string create_header_line( std::string_view option, std::string_view value );
  // void set_custom_options( std::string_view options );
  std::span<byte_t> get_body( );
  std::span<byte_t> get_header( );
  CURL* get_handle( );
  const char* get_url( );
  const char* get_current_url( );
  void* get_private_data( );
  static void* get_private_data( CURL* curl );
  void* get_body_cb_data( );
  long get_status_code( );
  std::string get_content_type();
  bool get_status_ok( );
protected:
  void* body_cb_data;
  void* private_data;
  bool_t local_init;
  CURL* curl;
  curl_slist* header_list;
  std::vector<byte_t> header;
  std::vector<byte_t> body;
  std::vector<byte_t> cookie;
  bool_t debug;
protected:
  std::string url;
  http_option method;
  std::string user_agent;
  bool_t follow_redirect;
  uint64_t timeout;
  std::string referrer;
  std::string cookie_dest_file;
  std::string cookie_src_file;  
  std::string encoding;
};

#define _ANI_HTTP_H
#endif