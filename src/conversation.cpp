#include "../include/conversation.h"

std::string_view memory::load_conversation( ) {
  ret_stmt.result.bind( 0, role );
  ret_stmt.result.bind( 1, message_type );
  ret_stmt.result.bind( 2, content );
  ret_stmt.result.bind( 3, images );
  db.bind_result( &this->ret_stmt );
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );

  // brainwash the nigga here
  this->set_brainwash_data( doc, root );

  yyjson_mut_val* msgs = yyjson_mut_arr( doc );
  yyjson_mut_obj_add_val( doc, root, "messages", msgs );
  db.execute( &this->ret_stmt );
  int ret = 0;
  while ( ( ret = this->db.fetch( &this->ret_stmt ) ) == 0 )
  {
    yyjson_mut_val* msg = yyjson_mut_obj( doc );
    yyjson_mut_obj_add_strncpy( doc, msg, "role", role.data( ), this->ret_stmt.result.lengths[0] );
    if ( !this->load_tool_calls( doc, msg, &this->ret_stmt ) ) {
      yyjson_mut_obj_add_strncpy( doc, msg, "content", content.data( ), this->ret_stmt.result.lengths[2] );
    }
    this->load_images( doc, msg );
    yyjson_mut_arr_add_val( msgs, msg );
  }
  if ( this->json_str != nullptr ) free( this->json_str );
  this->json_str = yyjson_mut_write( doc, 0, &this->len );
  yyjson_mut_doc_free( doc );
  return std::string_view( this->json_str, this->len ); 
}

void memory::set_brainwash_data( yyjson_mut_doc* doc, yyjson_mut_val* root ) {
  yyjson_mut_obj_add_str( doc, root, "model", /*"qwen3.5:9b"*/ "gemma4:12b" /*"sorc/qwen3.5-claude-4.6-opus:9b"*/ );
  yyjson_mut_obj_add_bool( doc, root, "stream", true );
  yyjson_mut_obj_add_bool( doc, root, "think", true );
  yyjson_mut_obj_add_int( doc, root, "keep_alive", -1 );
  yyjson_mut_val* options = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_val( doc, root, "options", options );
  yyjson_mut_obj_add_int( doc, options, "num_predict", -1 );
  yyjson_mut_obj_add_int( doc, options, "num_ctx", 32768 ); 
  this->load_tools_data( doc, root );
}

bool memory::load_tool_calls( yyjson_mut_doc* doc, yyjson_mut_val* msg, statement* stmt ) {
  if ( std::string_view( role.data( ), stmt->result.lengths[0] ) == "tool" ) {
    auto j_str = simdjson::padded_string( content.data( ), stmt->result.lengths[2] );
    auto j = this->parser.iterate( j_str );
    std::string_view tool_name = j["tool_name"].get_string( ).value( );
    std::string_view tool_id = j["tool_call_id"].get_string( ).value( );
    std::string_view tool_result = j["content"].get_string( ).value( );
    yyjson_mut_obj_add_strncpy( doc, msg, "tool_name", tool_name.data( ), tool_name.size( ) );
    yyjson_mut_obj_add_strncpy( doc, msg, "tool_call_id", tool_id.data( ), tool_id.size( ) ); 
    yyjson_mut_obj_add_strncpy( doc, msg, "content", tool_result.data( ), tool_result.size( ) );
    return true;
  }
  else if ( std::string_view( message_type.data( ), stmt->result.lengths[1] ) == "tool_call" ) {
    yyjson_mut_val* tool_calls = yyjson_mut_arr( doc );
    yyjson_mut_obj_add_val( doc, msg, "tool_calls", tool_calls );
    yyjson_mut_obj_add_strcpy( doc, msg, "content", "" );
    yyjson_doc* tool_str = yyjson_read( content.data( ), stmt->result.lengths[2], 0 );
    yyjson_val* tool_val = yyjson_doc_get_root( tool_str );
    yyjson_mut_val* tool_call = yyjson_val_mut_copy( doc, tool_val );
    yyjson_mut_arr_add_val( tool_calls, tool_call );
    return true;
  } else return false;
}

void memory::load_images( yyjson_mut_doc* doc, yyjson_mut_val* msg ) {
  if ( std::string_view( role.data( ), this->ret_stmt.result.lengths[0] ) == "user" ) {
    yyjson_doc* imgs_doc = yyjson_read( this->images.data( ), this->ret_stmt.result.lengths[3], 0 );
    yyjson_val* imgs_root = yyjson_doc_get_root( imgs_doc );
    if ( yyjson_arr_size( imgs_root ) > 0 ) {
      size_t idx, max;
      yyjson_val* val;
      yyjson_mut_val* imgs = yyjson_mut_arr( doc );
      yyjson_mut_obj_add_val( doc, msg, "images", imgs );
      std::string b64;
      std::ifstream f;
      yyjson_arr_foreach( imgs_root, idx, max, val ) {
        std::string_view path = yyjson_get_str( val );
        b64.clear( );
        f.open( std::string( path ), std::ios::binary );
        if ( f.is_open( ) ) {
          f.seekg( 0, std::ios::end );
          size_t len = f.tellg( );
          this->img_buffer.resize( len );
          f.seekg( 0, std::ios::beg );
          f.read( this->img_buffer.data( ), this->img_buffer.size( ) );
          encode_image( this->img_buffer, b64 );
          yyjson_mut_arr_add_strncpy( doc, imgs, b64.data( ), b64.size( ) );
          f.close( );
        }
      }
    }
    yyjson_doc_free( imgs_doc );
  }
}

void memory::load_tools_data( yyjson_mut_doc* doc, yyjson_mut_val* root ) {
  yyjson_mut_val* tools = yyjson_mut_arr( doc );
  yyjson_mut_obj_add_val( doc, root, "tools", tools );
  yyjson_mut_val* tool;
  for ( auto& i : *tools_data ) {
    tool = yyjson_val_mut_copy( doc, i.root );
    yyjson_mut_arr_add_val( tools, tool );
  }
}

void memory::encode_image( std::string_view data, std::string& out ) {
  base64_encodestate state; 
  base64_init_encodestate( &state );
  out.resize( data.size( ) * 2 );
  size_t len = base64_encode_block( data.data( ), data.size( ), out.data( ), &state );
  len += base64_encode_blockend( out.data( ) + len , &state );
  out.resize( len );
}


void memory::store_tool_result( std::string_view tool_name, std::string_view data ) {
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  yyjson_mut_obj_add_strn( doc, root, "tool_name", tool_name.data( ), tool_name.size( ) );
  yyjson_doc* web_doc = yyjson_read( data.data( ), data.size( ), 0 );
  yyjson_val* web_root = yyjson_doc_get_root( web_doc );
  yyjson_mut_val* ret = yyjson_val_mut_copy( doc, web_root );
  yyjson_mut_obj_add_val( doc, root, "content", ret );
  size_t len;
  char* j = yyjson_mut_write( doc, 0, &len );
  this->store( "tool", "tool_result", std::string_view( j, len ) );
  free( j );
  // free( this->_json_str );
  yyjson_doc_free( web_doc );
  yyjson_mut_doc_free( doc );
}

void memory::store_tool_result_str( std::string_view tool_name, std::string_view tool_id, std::string_view data ) {
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  yyjson_mut_obj_add_strncpy( doc, root, "tool_name", tool_name.data( ), tool_name.size( ) );
  yyjson_mut_obj_add_strncpy( doc, root, "tool_call_id", tool_id.data( ), tool_id.size( ) );
  yyjson_mut_obj_add_strncpy( doc, root, "content", data.data( ), data.size( ) );
  size_t len;
  char* j = yyjson_mut_write( doc, 0, &len );
  this->store( "tool", "tool_result", std::string_view( j, len ) );
  free( j );
  yyjson_mut_doc_free( doc );
}

void memory::store_image( const std::vector<std::string_view>& path ) {

}


std::string_view memory::load_conversation_beta( ) {
  return std::string_view( nullptr, 0 );
}

std::string_view memory::create_tool_call( std::string_view name, std::string_view func_desc, std::string_view param_desc ) {
  json root;
  root.init_json( nullptr );
  json tools;
  tools.init_arr( root.doc );
  root.add_arr( "tools", &tools );
  json tool;
  tool.init_json( root.doc );
  tool.add_str( "type", "function" );
  json function;
  function.init_json( root.doc );
  function.add_str( "name", "web_search" );
  function.add_str( "description", "");
  json parameters;
  parameters.init_json( root.doc );
  parameters.add_str( "type", "object" );
  json properties;
  properties.init_json( root.doc );
  json query;
  query.init_json( root.doc );
  query.add_str( "type", "string" );
  query.add_str( "description", "the search query to excecute" );
  properties.add_json( "query", &query );
  parameters.add_json( "properties", &properties );
  json required;
  required.init_arr( root.doc );
  required.add_str( "query", "" );
  parameters.add_arr( "required", &required );
  function.add_json( "parameters", &parameters );
  tool.add_json( "function", &function );
  tools.add_json( &tool );
  size_t len;
  char* data = root.get_json( len );
  return { data, len };
}

void memory::store( std::string_view role, std::string_view mtype, std::string_view content, std::string_view images ) {
  this->store_stmt.param.bind( 0, role );
  this->store_stmt.param.bind( 1, mtype );
  this->store_stmt.param.bind( 2, content );
  this->store_stmt.param.bind( 3, images );
  this->db.bind_params( &this->store_stmt );
  this->db.execute( &this->store_stmt ); 
}

void memory::store( std::string_view role, std::string_view mtype, std::string_view content ) {
  this->store_stmt.param.bind( 0, role );
  this->store_stmt.param.bind( 1, mtype );
  this->store_stmt.param.bind( 2, content );
  std::string_view image = "[]";
  this->store_stmt.param.bind( 3, image );
  this->db.bind_params( &this->store_stmt );
  this->db.execute( &this->store_stmt ); 
}
// void memory::put_image_in( yyjson_mut_doc* doc, yyjson_mut_val* root, const std::string& file ) {

// }

void memory::init( ) {
  db.initialize( );
  db.connect( "localhost", "agent", "agent123", "agent" );
  std::string store_sql, ret_sql;
  db.load_stmt_file( "sql/store.sql", store_sql );
  db.load_stmt_file( "sql/retrieve.sql", ret_sql );
  db.prepare_statement( &this->store_stmt, store_sql );
  db.prepare_statement( &this->ret_stmt, ret_sql );
  db.set_autocommit( true );
  this->store_stmt.param.init<std::string, std::string, std::string, std::string>( );
  this->ret_stmt.result.init<std::string, std::string, std::string, std::string>( );
  this->json_str = nullptr;

  this->role.resize( 10 );
  this->message_type.resize( 20 );
  this->content.resize( 5 * 1024 * 1024 );
  this->images.resize( 64 * 1024 );

  this->tools_manager.load_tools( "tools/" );
  this->tools_data = this->tools_manager.get_tools( );
}

memory::memory( ) {

}

memory::~memory( ) {

}