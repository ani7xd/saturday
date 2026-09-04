#include "util.h"
#include "http.h"
#include <yyjson.h>
#include <simdjson.h>
#include <lexbor/html/html.h>
#include <lexbor/selectors/selectors.h>
#include <lexbor/css/css.h>
#include "client.h"
#include "conversation.h"

#if !defined( _ANI_TOOL_MANAGER_H )

constexpr std::string_view RESET    = "\033[0m";
constexpr std::string_view TOOL     = "\033[1;34m";

struct tool_json {
  yyjson_doc* doc;
  yyjson_val* root;
  tool_json( ) : doc( nullptr ), root( nullptr ) { };
  tool_json( yyjson_doc* _doc, yyjson_val* _root ) : doc( _doc ), root( _root ) { };
  void release( );
  ~tool_json( );
};

struct fetched_resource {
  std::string mime;
  std::string filename;
  std::string body;
  std::filesystem::path stored_path;
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
  std::string str;
  fetched_resource* resource;
  edit_result* edit;
  char* json;
  size_t len;
  void* ctx;

  tool_result( ) : json( nullptr ), len( 0 ) { };
  size_t size( );
  char* data( );
  void clear( );
  ~tool_result( );
};

struct tool_context {
  simdjson::ondemand::parser parser;
  simdjson::padded_string json_str;
  simdjson::fallback::ondemand::document_stream::iterator::value_type json;
  bool tool_call;
  std::string tool_arguments_json;
  std::string tool_name;
  std::string tool_id;
  tool_result result;
  memory* context;
  void clear( ) {
    result.clear( );
  }
  tool_context( ) : tool_call( false ) { }
};

struct client_worker {
  http req;
  std::string header;
  std::string buffer;
  void* ctx;
};

int client_write_cb( void* ptr, size_t len, void* ctx );
size_t web_write_cb( char* ptr, size_t size, size_t nmemb, void* userdata );
void calling_tool( std::string_view str );

class tool_manager {
public:
  tool_manager( );
  void init( );
  static void create_tool( );
  ~tool_manager( );
public:
  void call_tool( tool_context* context, tool_result* ret );
  void load_tools( const std::filesystem::path& path );
  void load_tools_map( );
  void parse_arguements( tool_context* context );
  void store_tool_result( tool_result* result );
  std::vector<tool_json>* get_tools( );
public:
  void web_search( client_worker* web, std::string_view query );
  void web_search_google( client_worker* web, std::string_view query );
  void read_file( const std::string& path, std::string& contents );
  void write_file( const std::string& path, const std::string& contents );
  void write_file( const std::string& path, const std::string_view contents, std::string& ret );
  void edit_file( const std::filesystem::path& file, std::string_view old_text, std::string_view new_text, edit_result* result, size_t r_count = 0, size_t offset = 0 );
  void list_directories( const std::filesystem::path& path, tool_result* ret );
  void fetch_url( client_worker* client, std::string_view url, fetched_resource* ret );
  void image_reverse_search( std::string_view data );
  void image_reverse_search( const std::filesystem::path& path );
  void image_reverse_search( void* data, size_t len );
  void get_local_time( std::string& data );
  void get_global_time( std::string region );

  // void tool_open_browser_window_url( std::string url );
  void connect_to_tcp( const std::string& ip, short port, std::string& ret );
  void send_data_tcp( std::string_view data, std::string& ret );

  std::string_view trim_web_result_json( std::string_view data );
  std::string_view trim_web_result_str( std::string_view data, std::string& ret );
  std::string_view trim_html_result( std::string_view data, std::string& out );
  void extract_from_html( lxb_dom_node_t* node, std::string& out );
public:
  client cl;
  client_worker web_client;
  client_worker page_client;
  std::string buffer;
  std::vector<tool_json> tools;
  size_t index;
  size_t end;
  simdjson::ondemand::parser parser;
  std::string trimmed_web_result;
  size_t len;
  char* json_str;
  lxb_html_document_t* document;
  using handler = std::function<void(tool_context*)>;
  std::unordered_map<std::string_view, handler> tool_map;
};

#define _ANI_TOOL_MANAGER_H
#endif