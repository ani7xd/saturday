#include "database.h"
#include "tool_manager.h"
#include "json.h"
#include <simdjson.h>
extern "C" {
  #include <b64/cencode.h>
}

#if !defined( _ANI_CONVERSE_H )

class memory {
public:
  struct tool_data {
    yyjson_doc* doc;
    yyjson_val* root;
    tool_data( yyjson_doc* _doc, yyjson_val* _root ) : doc( _doc ), root( _root ) { };
  };
public:
  void init( );
  std::string_view create_tool_call( std::string_view name, std::string_view func_desc, std::string_view param_desc );
  std::string_view load_conversation_beta( );
  std::string_view load_conversation( );
  bool load_tool_calls( yyjson_mut_doc* doc, yyjson_mut_val* msg, statement* stmt );
  void load_images( yyjson_mut_doc* doc, yyjson_mut_val* images );
  void load_tools_data( yyjson_mut_doc* doc, yyjson_mut_val* root );
  void set_brainwash_data( yyjson_mut_doc* doc, yyjson_mut_val* root );
  void encode_image( std::string_view data, std::string& out );
  void store( std::string_view role, std::string_view mtype, std::string_view content );
  void store( std::string_view role, std::string_view mtype, std::string_view content, std::string_view images );
  void store_tool_result( std::string_view tool_name, std::string_view data );
  void store_tool_result_str( std::string_view tool_name, std::string_view tool_id, std::string_view data );
  void store_image( const std::vector<std::string_view>& path );
public:
  memory( );
  ~memory( );
private:
  database db;
  statement store_stmt;
  statement ret_stmt;
  tool_manager tools_manager;
  std::vector<tool_json>* tools_data;
  char* json_str;
  size_t len;
  std::string img_buffer;
  std::string role, message_type, content, images;
  simdjson::ondemand::parser parser;
};

#define _ANI_CONVERSE_H
#endif

