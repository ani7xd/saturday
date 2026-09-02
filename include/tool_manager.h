#include "http.h"
#include <fstream>
#include <filesystem>
#include <vector>
#include <chrono>
#include <format>
#include <iostream>
#include <yyjson.h>
#include <simdjson.h>
#include <lexbor/html/html.h>
#include <lexbor/selectors/selectors.h>
#include <lexbor/css/css.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include "client.h"

#if !defined( _ANI_TOOL_MANAGER_H )

struct tool_json {
  yyjson_doc* doc;
  yyjson_val* root;
  tool_json( ) : doc( nullptr ), root( nullptr ) { };
  tool_json( yyjson_doc* _doc, yyjson_val* _root ) : doc( _doc ), root( _root ) { };
  ~tool_json( );
};

struct tool_context {
  simdjson::ondemand::parser parser;
  simdjson::padded_string json_str;
  simdjson::fallback::ondemand::document_stream::iterator::value_type json;
  bool tool_call;
  std::string tool_arguments_json;
  std::string tool_name;
  std::string tool_id;
};

struct fetched_resource {
  std::string mime;
  std::string filename;
  std::string body;
  std::filesystem::path stored_path;
};

struct client_worker {
  http req;
  std::string header;
  std::string buffer;
  void* ctx;
};

struct edit_result {
  bool success = false;
  std::string file;
  std::string operation;
  size_t occurrence_offset = 0;
  size_t requested_count = 0;
  size_t replaced_count = 0;
  std::string error;
  char* json = nullptr;
  size_t len;
  std::string_view form_json( );
};

struct tool_result {
  char* json;
  size_t len;
  tool_result( ) : json( nullptr ), len( 0 ) { };
  size_t size( ) { return len; }
  void clear( ) {
    if ( json != nullptr ) free( json );
    len = 0;
  };
  ~tool_result( ) {
    if ( json != nullptr ) free( json );
  }
};

int write_cb( void* ptr, size_t len, void* ctx );

class tool_manager {
public:
  tool_manager( );
  static void create_tool( );
  ~tool_manager( );
public:
  void call_tool( tool_context* context, tool_result* ret );
  void load_tools( const std::filesystem::path& path );
  void parse_arguements( tool_context* context );
  std::vector<tool_json>* get_tools( );
public:
  void web_search( client_worker* web, std::string_view query );
  void read_file( std::string path, std::string& contents );
  void write_file( std::string path, const std::string& contents );
  void edit_file( const std::filesystem::path& file, std::string_view old_text, std::string_view new_text, edit_result* result, size_t r_count = 0, size_t offset = 0 );
  void list_directories( const std::filesystem::path& path, tool_result* ret );
  void fetch_url( client_worker* client, std::string_view url, fetched_resource* ret );
  void image_reverse_search( std::string_view data );
  void image_reverse_search( const std::filesystem::path& path );
  void image_reverse_search( void* data, size_t len );
  void get_local_time( std::string& data );
  void get_global_time( std::string region );

  void tool_open_browser_window_url( std::string url );
  void connect_to_tcp( const std::string& ip, short port );
  void send_data_tcp( std::string_view data, std::string& ret );

  std::string_view trim_web_result( std::string_view data );
  std::string_view trim_web_result_str( std::string_view data );
  std::string_view trim_html_result( std::string_view data, std::string& out );
  void extract_from_html( lxb_dom_node_t* node, std::string& out );
public:
  client cl;
  std::string buffer;
  std::vector<tool_json> tools;
  size_t index;
  size_t end;
  simdjson::ondemand::parser parser;
  std::string trimmed_web_result;
  size_t len;
  char* json_str;
  lxb_html_document_t* document;
};

#define _ANI_TOOL_MANAGER_H
#endif