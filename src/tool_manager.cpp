#include "../include/tool_manager.h"


void tool_manager::get_local_time( std::string& data ) {
  auto now = std::chrono::system_clock::now( );
  auto zt = std::chrono::zoned_time( std::chrono::current_zone( ), now );
  data = std::format( "{:%F %T %Z}", zt );
}

void tool_manager::get_global_time( const std::string& region, std::string& data ) {
  auto zone = std::chrono::locate_zone( region );
  auto now = std::chrono::system_clock::now( );
  auto zt = std::chrono::zoned_time( zone, now );
  data = std::format( "{:%F %T %Z}", zt );
  data = "error: could not get time for region '" + region;
}

void tool_manager::connect_to_tcp( const std::string& ip, short port, std::string& ret ) {
  if ( cl.connect( ip, port ) == 0 ) {
    ret = "log: connected successfully";
    return;
  }
  ret = "error: failed to connect";
  // ret += cl.error_str;
}

void tool_manager::send_data_tcp( std::string_view data, std::string& ret ) {
  if ( cl.send( data ) == 0 ) {
    ret = "sent ";
    ret += std::to_string( data.size( ) );
    ret += " bytes";
  }
  else {
    ret = "send failed";
    // ret += cl.error_str;
  }
}

void tool_manager::call_tool( tool_context* context, tool_result* ret ) {
  try 
  {
    error::debug( parse_arguements( context ) );
    tool_map[context->tool_name](context);
  }
  catch ( const error::excpt& excpt ) 
  {
    simdjson::ondemand::parser parser;
    // auto j_str = excpt.err_str;
    // auto json = parser.iterate( j_str );
    // std::string_view err = json["error"].get_string( ).value( );
    // std::string_view message = json["message"].get_string( ).value( );
    context->result.str = "tool error\n";
    context->result.str += excpt.usr_str;
    context->result.str += "\nerror: ";
    context->result.str += excpt.msg;
    context->result.str += "\nmessage: ";
    context->result.str += excpt.err_str;
    context->context->store_tool_result( context->tool_name, context->tool_id, context->result.str );
    context->result.clear( );
  }
}

void tool_manager::parse_arguements( tool_context* context ) {
  context->json_str = simdjson::padded_string( context->tool_arguments_json );
  context->json = parser.iterate( context->json_str );
}

void tool_manager::web_search( client_worker* web, std::string_view query ) {
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  yyjson_mut_obj_add_strcpy( doc, root, "topic", "general" );
  yyjson_mut_obj_add_strcpy( doc, root, "search_depth", "advanced" );
  yyjson_mut_obj_add_int( doc, root, "max_results", 5 );
  yyjson_mut_obj_add_bool( doc, root, "include_answer", true );
  yyjson_mut_obj_add_bool( doc, root, "include_raw_content", false );
  yyjson_mut_obj_add_bool( doc, root, "include_images", false );
  yyjson_mut_obj_add_strncpy( doc, root, "query", query.data( ), query.size( ) );
  size_t len;
  char* q = yyjson_mut_write( doc, 0, &len );
  web->req.set_post_data( std::string_view( q, len ) );
  web->buffer.clear( );
  web->req.request( );
  free( q );
  yyjson_mut_doc_free( doc );
}

void tool_manager::web_search_google( client_worker* web, std::string_view query ) {
  
}

void tool_manager::image_reverse_search( std::string_view data ) {
  
}

void tool_manager::image_reverse_search( const std::filesystem::path& path ) {

}
  
void tool_manager::image_reverse_search( void* data, size_t len ) {

}

std::string_view tool_manager::trim_web_result_str( std::string_view data, std::string& ret ) {
  auto j_str = simdjson::padded_string( data );
  auto j = parser.iterate( j_str );
  ret.clear( );
  std::string_view str = j["query"].get_string( ).value( );
  ret += "Web Search\n\n";
  ret += "Query:\n";
  ret.append( str.data( ), str.size( ) );
  ret += "\n\nQuick Summary:\n";
  str = j["answer"].get_string( ).value( );
  ret.append( str.data( ), str.size( ) );
  ret += "\n\nSources:\n";
  auto rets = j["results"].get_array( );

  int i = 1;
  for ( auto r : rets ) {
    ret += "\n";
    ret += std::to_string( i++ );
    ret += ".\n";
    ret += "URL:\n";
    str = r["url"].get_string( ).value( );
    ret.append( str.data( ), str.size( ) );
    ret += "\nTitle:\n";
    str = r["title"].get_string( ).value( );
    ret.append( str.data(), str.size( ) );
    ret += "\nSnippet:\n";
    str = r["content"].get_string( ).value( );
    ret.append( str.data( ), str.size( ) );
    ret += "\n";
  }
  return ret;
}

std::string_view tool_manager::trim_web_result_json( std::string_view data ) {
  auto j_str = simdjson::padded_string( data );
  auto j = parser.iterate( j_str );
  std::string_view str;
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  str = j["query"].get_string( ).value( );
  yyjson_mut_obj_add_strncpy( doc, root, "query", str.data( ), str.size( ) );
  str = j["answer"].get_string( ).value( );
  yyjson_mut_obj_add_strncpy( doc, root, "answer", str.data( ), str.size( ) );
  yyjson_mut_val* results = yyjson_mut_arr( doc );
  yyjson_mut_obj_add_val( doc, root, "results", results );
  
  auto rets = j["results"].get_array( );
  for ( auto r : rets ) {
    yyjson_mut_val* result = yyjson_mut_obj( doc );
    str = r["title"].get_string( ).value( );
    yyjson_mut_obj_add_strncpy( doc, result, "title", str.data( ), str.size( ) );
    str = r["url"].get_string( ).value( );
    yyjson_mut_obj_add_strncpy( doc, result, "url", str.data( ), str.size( ) );
    str = r["content"].get_string( ).value( );
    yyjson_mut_obj_add_strncpy( doc, result, "content", str.data( ), str.size( ) );
    yyjson_mut_arr_add_val( results, result );
  }
  if ( this->json_str != nullptr ) free( this->json_str );
  this->json_str = yyjson_mut_write( doc, 0, &this->len );
  yyjson_mut_doc_free( doc );
  return std::string_view( this->json_str, this->len );
}

std::string_view edit_result::form_json( ) {
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  yyjson_mut_obj_add_bool( doc, root, "success", this->success );
  yyjson_mut_obj_add_strn( doc, root, "file", this->file.data( ), this->file.size( ) );
  yyjson_mut_obj_add_strn( doc, root, "operation", this->operation.data( ), this->operation.size( ) );
  yyjson_mut_obj_add_uint( doc, root, "occurrence_offset", this->occurrence_offset );
  yyjson_mut_obj_add_uint( doc, root, "requested_count", this->requested_count );
  yyjson_mut_obj_add_uint( doc, root, "replaced_count", this->replaced_count );
  if ( !this->error.empty( ) ) yyjson_mut_obj_add_strn( doc, root, "error", this->error.data( ), this->error.size( ) );
  this->json = yyjson_mut_write( doc, 0, &this->len );
  yyjson_mut_doc_free( doc );
  return std::string_view( this->json, this->len );
}

void tool_manager::edit_file( const std::filesystem::path& file, std::string_view old_text, std::string_view new_text, edit_result* result, size_t r_count, size_t offset ) {
  result->success = false;
  result->file = file;
  result->operation = "replace";
  result->occurrence_offset = offset;
  result->requested_count = r_count;
  result->replaced_count = 0;
  result->error.clear( );
  if ( old_text.empty( ) ) {
    result->error = "error: old text field empty";
    result->form_json( );
    return;
  }
  std::ifstream f{ file };
  if ( f.is_open( ) ) {
    std::string contents{ std::istreambuf_iterator<char>( f ), std::istreambuf_iterator<char>( ) };
    f.close( );
    size_t count = 0;
    size_t ptr = 0;
    while ( offset > 0 ) {
      if ( ( ptr = contents.find( old_text, ptr ) ) == std::string::npos ) {
        result->error = "error: offset out of range";
        result->form_json( );
        return;
      }
      else {
        offset--;
        ptr += old_text.size( );
      }
    }

    if ( r_count == 0 ) {
      while ( ( ptr = contents.find( old_text, ptr ) ) != std::string::npos ) {
        contents.replace( ptr, old_text.size( ), new_text );
        count++;
        ptr += new_text.size( );
      }
    }
    else {
      while ( ( ptr = contents.find( old_text, ptr ) ) != std::string::npos && r_count > 0 ) {
        r_count--;
        contents.replace( ptr, old_text.size( ), new_text );
        count++;
        ptr += new_text.size( );
      }
    }
    
    std::ofstream out{ file, std::ios::trunc };
    if ( out.is_open( ) ) {
      out.write( contents.data( ), contents.size( ) );          // add counter to check for og r_count == count, so to tell llm upto how much count edited
      result->success = true;                                        // maybe also add line and column of where edits happened
      result->replaced_count = count;
      result->form_json( );
      out.close( );                         
    }
    else {
      result->error = "error: failed to write changes to file";
      result->form_json( );
      return;
    }
  }
  else {
    result->error = "error: couldnt open file to read data";
    result->form_json( );
    return;
  }
}

void tool_manager::list_directories( const std::filesystem::path& path, tool_result* ret ) {
  auto& p = path.native( );
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  yyjson_mut_doc_set_root( doc, root );
  if ( !std::filesystem::exists( path ) ) {
    yyjson_mut_obj_add_bool( doc, root, "success", false );
    yyjson_mut_obj_add_strncpy( doc, root, "path", p.data( ), p.size( ) );
    yyjson_mut_obj_add_strcpy( doc, root, "error", "path doesn't exists" );
  }
  else if ( !std::filesystem::is_directory( path ) ) {
    yyjson_mut_obj_add_bool( doc, root, "success", false );
    yyjson_mut_obj_add_strncpy( doc, root, "path", p.data( ), p.size( ) );
    yyjson_mut_obj_add_strcpy( doc, root, "error", "path is not a directory" );
  }
  else {
    std::error_code err;
    auto dir = std::filesystem::directory_iterator( path, err );
    if ( err ) {
      yyjson_mut_obj_add_bool( doc, root, "success", false );
      yyjson_mut_obj_add_strncpy( doc, root, "path", p.data( ), p.size( ) );
      yyjson_mut_obj_add_strcpy( doc, root, "error", err.message( ).c_str( ) );
      ret->json = yyjson_mut_write( doc, 0, &ret->len );
      yyjson_mut_doc_free( doc );
      return;
    }
    err.clear( );
    yyjson_mut_obj_add_bool( doc, root, "success", true );
    yyjson_mut_obj_add_strncpy( doc, root, "path", p.data( ), p.size( ) );
    yyjson_mut_val* entries = yyjson_mut_arr( doc );
    size_t entry_count = 0;
    for ( const auto& entry : dir ) {
      yyjson_mut_val* e = yyjson_mut_obj( doc );  
      auto nat = entry.path( ).filename( ).native( );
      yyjson_mut_obj_add_strncpy( doc, e, "name", nat.data( ), nat.size( ) );
      auto status = entry.symlink_status( err );
      if ( err ) {
        yyjson_mut_obj_add_strcpy( doc, e, "type", "unknown" );
        yyjson_mut_obj_add_strcpy( doc, e, "error", err.message( ).c_str( ) );         
      }
      else {
        switch ( status.type( ) ) {
          case std::filesystem::file_type::regular:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "file" );
            break;
          case std::filesystem::file_type::directory:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "directory" );
            break;
          case std::filesystem::file_type::symlink:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "symlink" );
            break;
          case std::filesystem::file_type::socket:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "socket" );
            break;
          case std::filesystem::file_type::unknown:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "unknown" );
            break;
          default:
            yyjson_mut_obj_add_strcpy( doc, e, "type", "unknown" );
            break;
        };
      }
      yyjson_mut_arr_add_val( entries, e );
      entry_count++;
    }
    yyjson_mut_obj_add_uint( doc, root, "entry_count", entry_count );
    yyjson_mut_obj_add_val( doc, root, "entries", entries );
  }
  ret->json = yyjson_mut_write( doc, 0, &ret->len );
  yyjson_mut_doc_free( doc );
}

void tool_manager::read_file( const std::string& path, std::string& contents ) {
  if ( !std::filesystem::exists( path ) ) {
    contents = "error: file doesn't exist";
    return;
  }
  else {
    std::ifstream file{ path };
    if ( file.is_open( ) ) {
      file.seekg( 0, std::ios::end );
      size_t len = file.tellg( );
      file.seekg( 0, std::ios::beg );
      contents.resize( len );
      file.read( contents.data( ), contents.size( ) );
      file.close( );
      return;
    }
    else {
      contents = "error: couldn't open file";
      return;
    }
  }
}
  
void tool_manager::write_file( const std::string& path, const std::string& contents ) {
  auto f_path = std::filesystem::path( path );
  if ( !std::filesystem::exists( f_path ) ) {
    if ( f_path.has_parent_path( ) )
      std::filesystem::create_directories( f_path.parent_path( ) );
    std::ofstream file{ path, std::ios::app };
    if ( file.is_open( ) ) {
      file.write( contents.data( ), contents.size( ) );
      file.close( ); 
    }
  } else {
    std::ofstream file{ path, std::ios::app };
    if ( file.is_open( ) ) {
      file.write( contents.data( ), contents.size( ) );
      file.close( );
    }
  }
}

void tool_manager::write_file( const std::string& path, const std::string_view contents, std::string& ret ) {
  ret.clear( );
  ret.reserve( 100 );
  auto f_path = std::filesystem::path( path );
  if ( !std::filesystem::exists( f_path ) ) {
    ret.append( "log: file path doesnt exist\n" );
    if ( f_path.has_parent_path( ) ) {
      ret.append( "log: creating parent dirs\n" );
      std::filesystem::create_directories( f_path.parent_path( ) );
    }
    ret.append( "log: creating file " );
    ret.append( path );
    ret.append( "\n" );
    std::ofstream file{ path, std::ios::app };
    if ( file.is_open( ) ) {
      ret.append( "log: successfully created and opened the file\n" );
      file.write( contents.data( ), contents.size( ) );
      ret.append( "log: written " );
      ret.append( std::to_string( contents.size( ) ) );
      ret.append( " bytes\n" );
      file.close( );
      return;
    }
    else {
      ret.append( "error: failed to create or open file\n" );
      return;
    }
  } else {
    ret.append( "log: file exists, trying to append to the file\n" );
    std::ofstream file{ path, std::ios::app };
    if ( file.is_open( ) ) {
      ret.append( "log: successfully opened the file\n" );
      file.write( contents.data( ), contents.size( ) );
      ret.append( "log: written " );
      ret.append( std::to_string( contents.size( ) ) );
      ret.append( " bytes\n" );
      file.close( );
      return;
    }
    else {
      ret.append( "error: failed to open file\n" );
      return;
    }
  }
}

void tool_manager::fetch_url( client_worker* client, std::string_view url, fetched_resource* ret ) {
  client->req.set_url( url );
  client->header.clear( );
  client->buffer.clear( );
  client->req.request( );
  size_t ptr; 
  if ( ( ptr = client->header.find( "content-type: " ) ) != std::string::npos ) {
    ptr += 14;
    std::string_view type = client->header.subview( ptr, client->header.find( "\r\n", ptr ) - ptr );
    size_t semi = type.find( ';' );
    if ( semi != std::string_view::npos )
      type = type.subview( 0, semi );
    if ( type == "text/html" ) {
      ret->mime = type;
      ret->body.clear( );
      this->trim_html_result( client->buffer, ret->body );
    }
  }
}

void tool_manager::wait( uint64_t ms ) {
  std::this_thread::sleep_for( std::chrono::milliseconds( ms ) );
}

std::string_view tool_manager::trim_html_result( std::string_view data, std::string& out ) {
  lxb_html_document_parse( this->document, reinterpret_cast<const u_char*>( data.data( ) ), data.size( ) );
  this->extract_from_html( lxb_dom_interface_node( this->document ), out );
  return out;
}

void tool_manager::extract_from_html( lxb_dom_node_t* node, std::string& out ) {
  if ( !node )
      return;

  if ( node->type == LXB_DOM_NODE_TYPE_TEXT ) {
    auto* text = lxb_dom_interface_character_data( node );
    out.append( reinterpret_cast<const char*>( text->data.data ), text->data.length );
    out += ' ';
    return;
  }

  if ( node->type == LXB_DOM_NODE_TYPE_ELEMENT ) {
    switch ( lxb_dom_node_tag_id( node ) ) {
      case LXB_TAG_SCRIPT:
      case LXB_TAG_STYLE:
      case LXB_TAG_NOSCRIPT:
      case LXB_TAG_TEMPLATE:
          return;

      case LXB_TAG_P:
      case LXB_TAG_SECTION:
      case LXB_TAG_ARTICLE:
          out += '\n';
          break;

      case LXB_TAG_H1:
          out += "\n# ";
          break;

      case LXB_TAG_H2:
          out += "\n## ";
          break;

      case LXB_TAG_H3:
          out += "\n### ";
          break;

      case LXB_TAG_LI:
          out += "\n- ";
          break;

      case LXB_TAG_BR:
          out += '\n';
          break;

      default:
          break;
    }    
  }

  for ( lxb_dom_node_t* child = node->first_child; child != nullptr; child = child->next ) {
    this->extract_from_html( child, out );
  }
}

std::vector<tool_json>* tool_manager::get_tools( ) {
  return &this->tools;
}

void tool_manager::load_tools_map( ) {
  // a more sexy idea would be to add call_tool() with 
  // unordered map of arguements, and pass parameters to tool
  // after extracting to map from the json array
  // ****idea implemented*****

  tool_map.emplace( "web_search", [&](tool_context* context) {
    calling_tool( "searching web..." );
    web_search( &web_client, context->json["query"].get_string( ).value( ) );
    std::string_view trimmed_web_result_in_str = trim_web_result_str( web_client.buffer, context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      context->result.str 
    );
  });
    
  tool_map.emplace( "read_file", [&](tool_context* context) {
    std::string path{ context->json["path"].get_string( ).value( ) };
    calling_tool( "reading file ( " + path + " )..." );
    read_file( path, context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      context->result.str 
    );
  });

  tool_map.emplace( "get_current_datetime", [&](tool_context* context) {
    calling_tool( "fetching time..." );
    get_local_time( context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id,
      context->result.str 
    );
  });

  tool_map.emplace( "write_file", [&](tool_context* context) {
    std::string path{ context->json["path"].get_string( ).value( ) };
    calling_tool( "writing to file ( " + path + " )..." );
    write_file( 
      path, 
      context->json["content"].get_string( ).value( ),
      context->result.str 
    );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      context->result.str 
    );
  });
  
  tool_map.emplace( "fetch_url", [&](tool_context* context) { 
    std::string url{ context->json["url"].get_string( ).value( ) };
    calling_tool( "fetching url... ( " + url + " )" );
    fetch_url( &page_client, url, context->result.resource );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id,
      context->result.resource->body 
    );
    context->result.clear( );
  });
  
  tool_map.emplace( "edit_file", [&](tool_context* context) {
    // parse_arguements( context );
    std::string path{ context->json["file"].get_string( ).value( ) };
    ssize_t c = 1;
    size_t o = 0;
    std::string_view old_text, new_text;
    old_text = context->json["old_text"].get_string( ).value( );
    new_text = context->json["new_text"].get_string( ).value( );
    auto count = context->json["occurrence_count"];
    auto offset = context->json["occurrence_offset"];
    if ( !count.error( ) ) c = count.get_int64( );
    if ( !offset.error( ) ) o = offset.get_uint64( );
    calling_tool( "editing file... ( " + path + " ) ..." );
    edit_file( path, old_text, new_text, context->result.edit, c, o ); 
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      { context->result.edit->json, context->result.edit->len } 
    );
    context->clear( );
  });
  
  tool_map.emplace( "list_dir", [&](tool_context* context) {
    std::string path{ context->json["path"].get_string( ).value( ) };
    calling_tool( "listing dir ( " + path + " )..." );
    list_directories( 
      path,
      &context->result
    );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      { context->result.json, context->result.len } 
    );
    context->clear( );
  });
  
  tool_map.emplace( "connect_to_tcp", [&](tool_context* context) {
    std::string ip{ context->json["ip"].get_string( ).value( ) };
    short port = context->json["port"].get_uint32( ).value( );
    calling_tool( "connecting to ( " + ip + ":" + std::to_string( port ) + " )..." );
    connect_to_tcp( ip, port, context->result.str );
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id,
      context->result.str
    );
    context->result.clear( );
  });
  
  tool_map.emplace( "send_data_tcp", [&](tool_context* context) {
    try
    {
      std::string_view data = error::require( context->json["data"].get_string( ), "data" );
      calling_tool( "sending data..." );
      send_data_tcp( data, context->result.str );
      context->context->store_tool_result( 
        context->tool_name, 
        context->tool_id, 
        context->result.str 
      );
      context->result.clear( );
    }
    catch ( const error::excpt& e ) 
    {
      throw error::excpt( e.err_code, "send_tcp_data json arguement error", e.msg, e.err_str );
    }
  });

  tool_map.emplace( "browser_navigate", [&](tool_context* context) {
    try
    {
      std::string_view url = error::require( context->json["url"].get_string( ), "url" );
      calling_tool( "navigating browser to url..." );
      puppet.navigate_window( url );
      context->result.str = "opened url (";
      context->result.str += url;
      context->result.str += ") in tab";
      context->context->store_tool_result( 
        context->tool_name, 
        context->tool_id, 
        context->result.str
      );
      context->result.clear( );
    }
    catch ( const error::excpt& e ) 
    {
      throw error::excpt( e.err_code, "browser_navigate json arguement error", e.msg, e.err_str );
    }
  });

  tool_map.emplace( "browser_element_click", [&](tool_context* context) {
    try {
      auto id = error::require( context->json["id"].get_uint64( ), "id" );
      calling_tool( "clicking element (" + std::to_string( id ) + " )..." );
      if ( id >= elements.size( ) )
        throw error::excpt( -1, "invalid element id", "element id out of range" );
      puppet.element_click( elements[id].uuid );
      context->result.str = "clicked element (";
      context->result.str += std::to_string( id );
      context->result.str += ") in tab";
      context->context->store_tool_result( 
        context->tool_name, 
        context->tool_id, 
        context->result.str
      );
      context->result.clear( );
    }
    catch ( const error::excpt& e ) 
    {
      throw error::excpt( e.err_code, "browser_element_click json error", e.msg, e.err_str );
    }
  });

  tool_map.emplace( "browser_element_fill", [&](tool_context* context) {
    try 
    {
      auto id = error::require( context->json["id"].get_uint64( ), "id" );
      std::string_view text = error::require( context->json["text"].get_string( ), "text" );
      calling_tool( "filling element ( " + std::to_string( id ) + " ) with content..." );
      puppet.fill( elements[id].uuid, text );
      context->result.str = "filled element (";
      context->result.str += std::to_string( id );
      context->result.str += ") with text in tab";
      context->context->store_tool_result( 
        context->tool_name, 
        context->tool_id,
        context->result.str
      );
      context->result.clear( );
    }
    catch ( const error::excpt& e ) 
    {
      throw error::excpt( e.err_code, "browser_element_fill error", e.msg, e.err_str );
    }
  });

  tool_map.emplace( "browser_observe", [&](tool_context* context) {
    calling_tool( "observing page..." );
    puppet.observe( js_script, this->elements );
    context->result.str.clear( );
    for ( auto& e : elements ) { 
      to_agent( e, context->result.str );
    }
    context->context->store_tool_result( 
      context->tool_name,
      context->tool_id,
      context->result.str
    );
    context->result.clear( );
  });

  tool_map.emplace( "browser_screenshot", [&](tool_context* context) {
    calling_tool( "taking a screenshot of the page..." );
    std::vector<std::byte> data;
    puppet.screenshot( data, true );
    std::ofstream f{ "img.png" };
    if ( f.is_open( ) ) {
      f.write( reinterpret_cast<char*>( data.data( ) ), data.size( ) );
      f.close( );
    }
    context->result.str = "page screenshot successfull";
    std::string_view image = "img.png";
    context->context->store_tool_result( 
      context->tool_name, 
      context->tool_id, 
      context->result.str,
      image
    );
    context->result.clear( );
  });

  tool_map.emplace( "browser_actions", [&](tool_context* context) {
    try
    {
      calling_tool( "performing browser actions...." );
      actions.clear( );
      auto arr = error::require( context->json["actions"].get_array( ), "actions" );
      size_t i = 0;
      for ( auto item : arr ) {
        auto obj = item.get_object( ).value( );
        browser_action a;
        a.source = error::require( obj["source"].get_string( ), "actions[].source" );
        a.actions = error::require( obj["action"].get_string( ), "action[].action" );
        if ( auto v = obj["target"].get_uint64( ); !v.error( ) ) {
          a.uuid = elements[v.value( )].uuid;
        }
        if ( auto v = obj["x"].get_int64( ); !v.error( ) ) a.x = v.value( );
        if ( auto v = obj["y"].get_int64( ); !v.error( ) ) a.y = v.value( );
        if ( auto v = obj["button"].get_uint64( ); !v.error( ) ) a.button = static_cast<uint8_t>( v.value( ) );
        if ( auto v = obj["delta_x"].get_int64( ); !v.error( ) ) a.delta_x = v.value( );
        if ( auto v = obj["delta_y"].get_int64( ); !v.error( ) ) a.delta_y = v.value( );
        if ( auto v = obj["duration"].get_uint64( ); !v.error( ) ) a.duration = v.value( );
        if ( auto v = obj["key"].get_string( ); !v.error( ) ) a.key = v.value( );
        if ( auto v = obj["text"].get_string( ); !v.error( ) ) a.text = v.value( );
        actions.push_back( std::move( a ) );
        i++;
      }
      packet p;
      puppet.perform_actions( actions.data( ), actions.size( ), p );
      context->result.str = "successfully performed all actions";
      context->context->store_tool_result( 
        context->tool_name, 
        context->tool_id, 
        context->result.str
      );
      context->result.clear( );
    }
    catch ( const error::excpt& e ) 
    {
      throw error::excpt( e.err_code, "browser_actions json arguement error", e.msg, e.err_str );
    }
  });

  tool_map.emplace( "wait", [&](tool_context* context) {
    auto duration = error::require( context->json["duration"].get_uint64( ), "duration" );
    calling_tool( "Wating for " + std::to_string( duration ) + " ms" );
    wait( duration );
    context->result.str = "waited for ";
    context->result.str += std::to_string( duration );
    context->result.str += " milliseconds";
    context->context->store_tool_result( 
      context->tool_name,
      context->tool_id,
      context->result.str
    );
  });
}

void tool_manager::load_tools( const std::filesystem::path& path ) {
  load_tools_map( );
  std::ifstream f;
  auto dir = std::filesystem::recursive_directory_iterator( path );
  std::string tool_str;
  tool_str.reserve( 4096 );
  for ( const auto& entry : dir ) {
    if ( entry.is_regular_file( ) && entry.path( ).extension( ) == ".tool" ) {
      f.open( entry.path( ) );
      if ( f.is_open( ) ) {
        f.seekg( 0, std::ios::end );
        size_t len = f.tellg( );
        f.seekg( 0, std::ios::beg );
        if ( len > tool_str.size( ) ) tool_str.resize( len );
        f.read( tool_str.data( ), len );
        auto doc = yyjson_read( tool_str.data( ), len, 0 );
        auto root = yyjson_doc_get_root( doc );
        this->tools.emplace_back( doc, root );
        f.close( );
      }
    }
  }
}

void tool_manager::init( ) {
  cl.init( );
  cl.set_receive_cb( client_write_cb );
  cl.set_receive_cb_ctx( &buffer );
  web_client.req.initialize( );
  web_client.req.set_url( "https://api.tavily.com/search" );
  web_client.req.set_http_method_post( );
  web_client.req.set_custom_option_list( "Content-Type", "application/json" );
  std::string key = std::getenv( "TAVILY_API_KEY" );
  web_client.req.set_custom_option_list( "Authorization", "Bearer " + key );
  web_client.req.set_body_write_cb( web_write_cb );
  web_client.req.set_body_cb_data( &web_client.buffer );
  web_client.req.set_custom_options( );

  page_client.req.initialize( );
  page_client.req.set_body_write_cb( web_write_cb );
  page_client.req.set_body_cb_data( &page_client.buffer );
  page_client.req.set_header_cb_data( &page_client.header );
  page_client.req.set_header_write_cb( web_write_cb );
  page_client.req.set_follow_redirect( true );

  puppet.init( );
  puppet.create_session( );
  puppet.create_new_tab( );
  puppet.switch_to_window( puppet.tab_handle );
}

tool_manager::tool_manager( ) {
  this->index = 0;
  this->document = lxb_html_document_create( );
  
}

tool_manager::~tool_manager( ) {
  for ( auto& tool : tools )
    tool.release( );
}

void tool_json::release( ) {
  yyjson_doc_free( doc );
}

tool_json::~tool_json( ) {
  
}

size_t tool_result::size( ) { 
  return len; 
}
  
char* tool_result::data( ) { 
  return json; 
}
  
void tool_result::clear( ) {
  if ( json != nullptr ) {
    free( json );
    json == nullptr;
  }
  str.clear( );
  len = 0;
};

tool_result::~tool_result( ) {
  if ( json != nullptr ) free( json );
}

size_t web_write_cb( char* ptr, size_t size, size_t nmemb, void* userdata ) {
  auto* str = static_cast<std::string*>( userdata );
  str->append( ptr, size * nmemb );
  return size * nmemb;
}

int client_write_cb( void* ptr, size_t len, void* ctx ) {
  auto buffer = static_cast<std::string*>( ctx );
  buffer->append( static_cast<char*>( ptr ), len );
  return recv_status::finish;
}

void calling_tool( std::string_view str ) {
  std::cout << TOOL << "[tool-call] " << str << RESET << "\n";
}

void tool_manager::to_agent( browser_element& e, std::string& out ) {
  if ( !e.visible ) return;
  out += '[';
  out += std::to_string(e.id);
  out += "] ";
  out += e.tag;

  if (!e.role.empty()) {
      out += " | role=";
      out += e.role;
  }

  if (!e.type.empty()) {
      out += " | type=";
      out += e.type;
  }

  if (!e.name.empty()) {
      out += " | name=";
      out += e.name;
  }

  if (!e.text.empty()) {
      out += R"( | text=")";
      out += e.text;
      out += '"';
  }

  if (!e.value.empty()) {
      out += R"( | value=")";
      out += e.value;
      out += '"';
  }

  if (!e.placeholder.empty()) {
      out += R"( | placeholder=")";
      out += e.placeholder;
      out += '"';
  }

  if (!e.aria_label.empty()) {
      out += R"( | aria=")";
      out += e.aria_label;
      out += '"';
  }

  if (!e.href.empty()) {
      out += R"( | href=")";
      out += e.href;
      out += '"';
  }

  if (e.disabled)
      out += " | disabled";

  if (e.interactable)
      out += " | interactable";

  if (e.clickable)
      out += " | clickable";

  out += '\n';
}


