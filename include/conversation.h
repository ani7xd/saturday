#include "database.h"
#include "json.h"
#include <simdjson.h>


#if !defined( _ANI_CONVERSE_H )

struct tool_data {
  yyjson_doc* doc;
  yyjson_val* root;
  tool_data( yyjson_doc* _doc, yyjson_val* _root ) 
    : doc( _doc ), root( _root ) { };
};

class memory {
public:
  void init( );
  void select_model(std::string name, bool tools, bool thinking);
  std::string_view create_tool_call( std::string_view name, std::string_view func_desc, std::string_view param_desc );
  std::string_view load_conversation_beta( );
  std::string_view load_conversation( );
  void load_system_prompt( yyjson_mut_doc* doc, yyjson_mut_val* msgs );
  bool load_tool_calls( yyjson_mut_doc* doc, yyjson_mut_val* msg, statement* stmt );
  void load_images( yyjson_mut_doc* doc, yyjson_mut_val* images );
  void load_tools_data( yyjson_mut_doc* doc, yyjson_mut_val* root );
  void set_brainwash_data( yyjson_mut_doc* doc, yyjson_mut_val* root );
  void encode_image( std::string_view data, std::string& out );
  void store( std::string_view role, std::string_view mtype, std::string_view content );
  void store( std::string_view role, std::string_view mtype, std::string_view content, std::string_view images );
  void store_system_prompt( std::string_view prompt );
  // void store_tool_result( std::string_view tool_name, std::string_view data );
  void store_tool_result( std::string_view tool_name, std::string_view tool_id, std::string_view data );
  void store_image( const std::vector<std::string_view>& path );
  void add_tools( tool_data* tools, size_t n_tools );
  void clear_tools( );
public:
  memory( );
  ~memory( );
private:
  std::string selected_model = "gemma3:12b";
  bool supports_tools = false;
  bool supports_thinking = false;
  database db;
  statement store_stmt;
  statement ret_stmt;
  statement system_prompt_store_stmt;
  statement system_prompt_retrieve_stmt;
  std::vector<tool_data> tools_info;
  char* json_str = nullptr;
  size_t len;
  std::string img_buffer;
  std::string role, message_type, content, images;
  std::string system_prompt;
  simdjson::ondemand::parser parser;
};

#define _ANI_CONVERSE_H
#endif

