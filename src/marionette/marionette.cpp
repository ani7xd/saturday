#include "../../include/marionette/marionette.h"

// void marionette::keyboard_type( std::string_view uuid, std::string_view content ) {
//   pack.create_doc_new( );
//   yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
//   {
    
//   }
//   send_command( webdriver::perform_actions, args, pack );
//   recv_reply( );
//   error::debug( parse_context( session, reply ) );
//   if ( reply.is_error( ) ) {
//     throw error::excpt( -1, "keyboard type error", reply.error );
//   } 
// }

void marionette::perform_actions( browser_action* actions, size_t n_actions, packet& p ) {
  p.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( p.doc );
  yyjson_mut_val* sources = set_browser_action( actions[0], p.doc, args );
  add_browser_action( actions[0], p.doc, sources );
  std::string_view previous = actions[0].source;
  for ( size_t index = 1; index < n_actions; index++ ) {
    if ( previous != actions[index].source ) {
      send_command( webdriver::perform_actions, args, p );
      recv_reply( );
      error::debug( parse_context( session, reply ) );
      if ( reply.is_error( ) ) {
        throw error::excpt( -1, "perform actions error", reply.error );
      }
      previous = actions[index].source;
      p.create_doc_new( );
      args = yyjson_mut_obj( p.doc );
      sources = set_browser_action( actions[index], p.doc, args );
      add_browser_action( actions[index], p.doc, sources );
    }
    else {
      add_browser_action( actions[index], p.doc, sources );
    }
  }
  send_command( webdriver::perform_actions, args, p );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    throw error::excpt( -1, "perform actions error", reply.error );
  }
}

yyjson_mut_val* marionette::set_browser_action( browser_action& action, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  yyjson_mut_val* sources = yyjson_mut_arr( doc );
  yyjson_mut_obj_add_val( doc, actions, "actions", sources );
  yyjson_mut_val* source_actions;
  if ( action.source == webdriver::browser_action_source::keyboard ) {
    source_actions = set_perform_keyboard( doc, sources );
  }
  else if ( action.source == webdriver::browser_action_source::mouse ) {
    source_actions = set_perform_mouse( doc, sources );
  } 
  return source_actions;
}

void marionette::add_browser_action( browser_action& action, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  if ( action.source == webdriver::browser_action_source::keyboard ) {
    if ( action.actions == webdriver::browser_action_type::press ) keyboard_key_press( action.key, doc, actions );
    else if ( action.actions == webdriver::browser_action_type::text ) keyboard_text( action.text, doc, actions );
    else if ( action.actions == webdriver::browser_action_type::up ) keyboard_key_up( action.key, doc, actions );
    else if ( action.actions == webdriver::browser_action_type::down ) keyboard_key_down( action.key, doc, actions );
  }
  else if ( action.source == webdriver::browser_action_source::mouse ) {
    if ( action.actions == webdriver::browser_action_type::move ) mouse_move( action.uuid, action.duration, action.x, action.y, doc, actions );
    else if ( action.actions == webdriver::browser_action_type::click ) mouse_button_click( action.uuid, action.button, doc, actions );
    else if ( action.actions == webdriver::browser_action_type::up ) mouse_button_up( action.button, doc, actions );
    else if ( action.actions == webdriver::browser_action_type::down ) mouse_button_down( action.button, doc, actions );
  }
}


void marionette::perform_actions( std::string_view uuid, packet& p ) {
  p.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( p.doc );
  yyjson_mut_val* sources = yyjson_mut_arr( p.doc );
  yyjson_mut_obj_add_val( p.doc, args, "actions", sources );
  {
    {
      // actions sources here

      // yyjson_mut_val* mouse_actions = set_perform_mouse( p.doc, sources );
      // mouse_move( uuid, 100, 0, 0, p.doc, mouse_actions );
      // mouse_click( 0, p.doc, mouse_actions );
      yyjson_mut_val* key_actions = set_perform_keyboard( p.doc, sources );
      keyboard_key_press( "A", p.doc, key_actions );
      keyboard_key_press( "n", p.doc, key_actions );
      keyboard_key_press( "i", p.doc, key_actions );
    }
  }
  send_command( webdriver::perform_actions, args, p );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    throw error::excpt( -1, "perform actions error", reply.error );
  }
}

yyjson_mut_val* marionette::set_perform_mouse( yyjson_mut_doc* doc, yyjson_mut_val* root ) {
  yyjson_mut_val* args = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strcpy( doc, args, "type", "pointer" );
  yyjson_mut_obj_add_strcpy( doc, args, "id", "mouse" );
  yyjson_mut_val* parameters = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_val( doc, args, "parameters", parameters );
  yyjson_mut_obj_add_strcpy( doc, parameters, "pointerType", "mouse" );
  yyjson_mut_val* actions = yyjson_mut_arr( doc );
  yyjson_mut_obj_add_val( doc, args, "actions", actions );
  if ( yyjson_mut_is_arr( root ) ) yyjson_mut_arr_add_val( root, args );
  return actions;
}

yyjson_mut_val* marionette::set_perform_keyboard( yyjson_mut_doc* doc, yyjson_mut_val* root ) {
  yyjson_mut_val* args = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strcpy( doc, args, "type", "key" );
  yyjson_mut_obj_add_strcpy( doc, args, "id", "keyboard" );
  yyjson_mut_val* actions = yyjson_mut_arr( doc );
  yyjson_mut_obj_add_val( doc, args, "actions", actions );
  if ( yyjson_mut_is_arr( root ) ) yyjson_mut_arr_add_val( root, args );
  return actions;
}

void marionette::keyboard_key_press( std::string_view key, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  if ( key.size( ) == 1 ) {
    keyboard_key_down( key, doc, actions );
    keyboard_key_up( key, doc, actions );
  }
  else {
    keyboard_key_down( key_map[key], doc, actions );
    keyboard_key_up( key_map[key], doc, actions );
  }
}

void marionette::keyboard_key_up( std::string_view key, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  yyjson_mut_val* action = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strcpy( doc, action, "type", "keyUp" );
  yyjson_mut_obj_add_strncpy( doc, action, "value", key.data( ), key.size( ) );
  yyjson_mut_arr_add_val( actions, action );
}

void marionette::keyboard_key_down( std::string_view key, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  yyjson_mut_val* action = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strcpy( doc, action, "type", "keyDown" );
  yyjson_mut_obj_add_strncpy( doc, action, "value", key.data( ), key.size( ) );
  yyjson_mut_arr_add_val( actions, action );
}

// maybe later
// void marionette::keyboard_key_up( char c, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {

// }

// void marionette::keyboard_key_down( char c, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
//   yyjson_mut_val* action = yyjson_mut_obj( doc );
//   yyjson_mut_obj_add_strcpy( doc, action, "type", "keyDown" );
//   yyjson_mut_obj_add_strncpy( doc, action, "value", c, 1 );
//   yyjson_mut_arr_add_val( actions, action );
// }

void marionette::keyboard_text( std::string_view text, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  for ( auto& c : text ) {
    keyboard_key_press( { (char*)&c, 1  }, doc, actions );
  }
}

void marionette::mouse_move( std::string_view uuid, uint64_t duration, int64_t x, int64_t y, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  yyjson_mut_val* origin = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strncpy( doc, origin, webdriver::element_key.data( ), uuid.data( ), uuid.size( ) );
  mouse_move( duration, x, y, origin, doc, actions );
}

void marionette::mouse_move( uint64_t duration, int64_t x, int64_t y, std::string_view origin, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  yyjson_mut_val* action = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strcpy( doc, action, "type", "pointerMove" );
  yyjson_mut_obj_add_uint( doc, action, "duration", duration );
  yyjson_mut_obj_add_int( doc, action, "x", x );
  yyjson_mut_obj_add_int( doc, action, "y", y );
  yyjson_mut_obj_add_strncpy( doc, action, "origin", origin.data( ), origin.size( ) );
  yyjson_mut_arr_add_val( actions, action );
}

void marionette::mouse_move( uint64_t duration, int64_t x, int64_t y, yyjson_mut_val* origin, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  yyjson_mut_val* action = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strcpy( doc, action, "type", "pointerMove" );
  yyjson_mut_obj_add_uint( doc, action, "duration", duration );
  yyjson_mut_obj_add_int( doc, action, "x", x );
  yyjson_mut_obj_add_int( doc, action, "y", y );
  yyjson_mut_obj_add_val( doc, action, "origin", origin );
  yyjson_mut_arr_add_val( actions, action );
}

void marionette::mouse_button_click( std::string_view uuid, uint64_t button, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  mouse_move( uuid, 0, 0, 0, doc, actions );
  mouse_button_down( button, doc, actions );
  mouse_button_up( button, doc, actions );
}

void marionette::mouse_button_click( uint64_t button, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  mouse_button_down( button, doc, actions );
  mouse_button_up( button, doc, actions );
}

void marionette::mouse_button_up( uint64_t button, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  yyjson_mut_val* action = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strcpy( doc, action, "type", "pointerUp" );
  yyjson_mut_obj_add_uint( doc, action, "button", button );
  yyjson_mut_arr_add_val( actions, action );
}

void marionette::mouse_button_down( uint64_t button, yyjson_mut_doc* doc, yyjson_mut_val* actions ) {
  yyjson_mut_val* action = yyjson_mut_obj( doc );
  yyjson_mut_obj_add_strcpy( doc, action, "type", "pointerDown" );
  yyjson_mut_obj_add_uint( doc, action, "button", button );
  yyjson_mut_arr_add_val( actions, action );
}

void marionette::observe( std::string_view js, std::vector<browser_element>& elements ) {
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    yyjson_mut_obj_add_strcpy( pack.doc, args, "using", "css selector" );
    yyjson_mut_obj_add_strcpy( pack.doc, args, "value", "a, button, input, textarea, select, [role], [tabindex]" );
  }
  send_command( webdriver::find_elements, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  simdjson::ondemand::parser uuid_parser;
  auto uuid_j_str = reply.json;
  auto uuid_json = uuid_parser.iterate( uuid_j_str );
  packet p;
  execute_js( js_script, reply.json, p );
  j_str = reply.json;
  json = parser.iterate( j_str );
  auto arr = json["value"].get_array( );
  auto uuid_it = uuid_json.get_array( ).begin( );
  size_t id = 0;
  for ( auto a : arr ) {
    if ( uuid_it.at_end( ) ) {
      break;
    }
    auto elem = *uuid_it;
    auto object = elem.get_object( );
    auto field_it = object.begin( );
    auto field = *field_it;
    std::string_view uuid = field.value( ).get_string( ).value( );
    browser_element e;
    e.id = id;
    e.uuid = uuid;
    e.tag = a["tag"].get_string( ).value( );
    e.text = a["text"].get_string( ).value( );
    e.role = a["role"].get_string( ).value( );
    e.type = a["type"].get_string( ).value( );
    e.name = a["name"].get_string( ).value( );
    e.value = a["value"].get_string( ).value( );
    e.placeholder = a["placeholder"].get_string( ).value( );
    e.aria_label = a["aria_label"].get_string( ).value( );
    e.href = a["href"].get_string( ).value( );

    e.disabled = a["disabled"].get_bool( );
    e.visible  = a["visible"].get_bool( );
    e.interactable  = a["interactable"].get_bool( );
    e.clickable  = a["clickable"].get_bool( );

    elements.push_back( std::move( e ) );
    id++;
    ++uuid_it;
  }
}

void marionette::screenshot( std::vector<std::byte>& out, bool full_page ) {
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    yyjson_mut_obj_add_bool( pack.doc, args, "full", full_page );
    yyjson_mut_obj_add_bool( pack.doc, args, "hash", false );
    yyjson_mut_obj_add_bool( pack.doc, args, "scroll", true );
  }
  send_command( webdriver::take_screenshot, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    // std::cout << "screenshot error: " << reply.error << "\n";
    throw error::excpt( -1, "screenshot error", reply.error );
  }
  else {
    j_str = reply.json;
    json = parser.iterate( j_str );
    std::string_view data = json["value"].get_string( ).value( );
    out.clear( );
    out.resize( ( ( data.size( ) / 4 ) * 3 ) + 1 ); 
    base64::decoder decoder;
    int ret = decoder.decode( data.data( ), data.size( ), reinterpret_cast<char*>( out.data( ) ) );
    out.resize( ret );
  }
}

void marionette::screenshot( std::string_view uuid, std::vector<std::byte>& out, bool full_page ) {

}

void marionette::element_click( std::string_view uuid ) {
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    yyjson_mut_obj_add_strn( pack.doc, args, "id", uuid.data( ), uuid.size( ) ); 
  }
  send_command( webdriver::element_click, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "element click error\n"; // :\n" << reply.error << "\n";
    throw error::excpt( -1, "element click error", reply.error );
  }
}
  
void marionette::fill( std::string_view uuid, std::string_view content ) {
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    yyjson_mut_obj_add_strn( pack.doc, args, "id", uuid.data( ), uuid.size( ) ); 
    yyjson_mut_obj_add_strn( pack.doc, args, "text", content.data( ), content.size( ) );
  }
  send_command( webdriver::element_send_keys, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    throw error::excpt( -1, "element fill error", reply.error );
  }
}

// need to make it better
void marionette::press_key( std::string_view uuid, std::string_view key ) {
  if ( key == "Enter" || key == "enter" ) fill( uuid, webdriver::key::enter );
  else {
    fill( uuid, key );
  }
}

void marionette::scroll_into_view( std::string_view uuid ) {
  std::string args = R"({"element-6066-11e4-a52e-4f735466cecf":")";
  args += uuid;
  args += R"("})";
  execute_js( R"(arguments[0].scrollIntoView({
    behavior: "instant",
    block: "center",
    inline: "center"
  });)",
    args,
    pack
);
}

void marionette::execute_js( std::string_view script, std::string_view script_args, packet& p ) {
  p.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( p.doc );
  {
    yyjson_mut_obj_add_strn( p.doc, args, "script", script.data( ), script.size( ) );
    yyjson_doc* doc = yyjson_read( script_args.data( ), script_args.size( ), 0 );
    yyjson_val* root = yyjson_doc_get_root( doc );
    yyjson_mut_val* script_args_arr = yyjson_mut_arr( p.doc );
    yyjson_mut_val* arg = yyjson_val_mut_copy( p.doc, root );
    yyjson_mut_arr_add_val( script_args_arr, arg );
    yyjson_mut_obj_add_val( p.doc, args, "args", script_args_arr );
  }
  send_command( webdriver::execute_script, args, p );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "execute js error\n";// << reply.error << "\n";
    throw error::excpt( -1, "element js error", reply.error );
  }
}

void marionette::get_attr_name( std::string_view tag ) {
  std::string args;
  // create_packet( ++msg_id, webdriver::get_element_attribute, args, pack );
}

void marionette::create_new_tab( ) {
  msg_id++;
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    yyjson_mut_obj_add_strcpy( pack.doc, args, "type", "tab" );
    yyjson_mut_obj_add_bool( pack.doc, args, "focus", true );
  }
  send_command( webdriver::new_window, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "create new tab error\n";
    throw error::excpt( -1, "create new tab error", reply.error );
  }
  this->j_str = this->reply.json;
  this->json = this->parser.iterate( this->j_str );
  this->tab_handle = this->json["handle"].get_string( ).value( );
  std::cout << "[marionette] created tab handle : " << this->tab_handle << "\n";
}

void marionette::navigate_window( std::string_view url ) {
  this->msg_id++;
  pack.create_doc_new( );
  yyjson_mut_val* arg = yyjson_mut_obj( pack.doc );
  {
    yyjson_mut_obj_add_strncpy( pack.doc, arg, "url", url.data( ), url.size( ) );
  }
  send_command( webdriver::navigate, arg, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "navigate error\n";
    throw error::excpt( -1, "navigate error", reply.error );
  }
}

void marionette::switch_to_window( std::string_view handle ) {
  // this->msg_id++;
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    yyjson_mut_obj_add_strncpy( pack.doc, args, "handle", handle.data( ), handle.size( ) );
  }
  send_command( webdriver::switch_to_window, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "switch to window error\n";
    throw error::excpt( -1, "switch to window error", reply.error );
  }
}

void marionette::window_go_back( ) {
  // this->msg_id++;
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    
  }
  send_command( webdriver::back, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "window_go_back error\n";
    throw error::excpt( -1, "window_go_back error", reply.error );
  }
  // if ( msg_id != reply.msg_id ) std::cout << "wrong response: msg id dont match\n";
}

void marionette::window_go_forward( ) {
  // this->msg_id++;
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  send_command( webdriver::forward, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "window forward error\n";
    throw error::excpt( -1, "window_go_forward error", reply.error );
  }
  // if ( msg_id != reply.msg_id ) std::cout << "wrong response: msg id dont match\n";
}

void marionette::window_refresh( ) {
  this->msg_id++;
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  send_command( webdriver::refresh, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "window refresh error\n";
    throw error::excpt( -1, "window refresh error", reply.error );
  }
  // if ( msg_id != reply.msg_id ) std::cout << "wrong response: msg id dont match\n";
}

void marionette::load_key_map( ) {
  key_map.emplace( "ENTER", webdriver::key::enter );
  key_map.emplace( "TAB", webdriver::key::tab );
  key_map.emplace( "BACKSPACE", webdriver::key::backspace );
  key_map.emplace( "ESC", webdriver::key::esc );
  key_map.emplace( "SHIFT", webdriver::key::shift );
  key_map.emplace( "CTRL", webdriver::key::ctrl );
  key_map.emplace( "ALT", webdriver::key::alt );
  key_map.emplace( "META", webdriver::key::meta );
  key_map.emplace( "LEFT", webdriver::key::left );
  key_map.emplace( "UP", webdriver::key::up );
  key_map.emplace( "RIGHT", webdriver::key::right );
  key_map.emplace( "DOWN", webdriver::key::down );
}

void marionette::init( ) {
  cl.init( );
  cl.set_receive_cb( write_cb );
  cl.set_receive_cb_ctx( &session );
  cl.connect( "127.0.0.1", 2828 );
  recv_reply( );
  load_key_map( );
}

void marionette::create_session( ) {
  msg_id = 0;
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    yyjson_mut_val* value = yyjson_mut_obj( pack.doc );
    yyjson_mut_obj_add_val( pack.doc, args, "capabilities", value );
  }
  send_command( webdriver::new_session, args, pack ); 
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) ) {
    std::cout << "create session error\n";
    throw error::excpt( -1, "create session error", reply.error );
  }
  this->j_str = this->reply.json;
  this->json = this->parser.iterate( this->j_str );
  this->session_id = this->json["sessionId"].get_string( ).value( );
  std::cout << "[marionette] created session id : " << this->session_id << "\n"; 
}

void marionette::get_window_handle( ) {
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  {
    
  }
  send_command( webdriver::get_window_handle, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  if ( reply.is_error( ) )
    throw error::excpt( -1, "get_window_handle error", reply.error );
  this->j_str = this->reply.json;
  this->json = this->parser.iterate( this->j_str );
  this->tab_handle = this->json["value"].get_string( ).value( );
}

void marionette::create_base_packet( size_t id, std::string_view name, packet* p ) {
  p->root = yyjson_mut_arr( p->doc );
  yyjson_mut_doc_set_root( p->doc, p->root );
  yyjson_mut_arr_add_uint( p->doc, p->root, 0 );
  yyjson_mut_arr_add_uint( p->doc, p->root, id );
  yyjson_mut_arr_add_strn( p->doc, p->root, name.data( ), name.size( ) );
}

void marionette::create_base_packet( size_t id, std::string_view name, packet& p ) {
  p.root = yyjson_mut_arr( p.doc );
  yyjson_mut_doc_set_root( p.doc, p.root );
  yyjson_mut_arr_add_uint( p.doc, p.root, 0 );
  yyjson_mut_arr_add_uint( p.doc, p.root, id );
  yyjson_mut_arr_add_strn( p.doc, p.root, name.data( ), name.size( ) );
}

// can be made a bit more sexy,
void marionette::create_args( const std::vector<std::pair<std::string_view, std::string_view>>& args, std::string_view& ret ) {
  yyjson_mut_doc* doc = yyjson_mut_doc_new( nullptr );
  yyjson_mut_val* root = yyjson_mut_obj( doc );
  for ( auto& pair : args ) {
    yyjson_mut_obj_add_strn( doc, root, pair.first.data( ), pair.second.data( ), pair.second.size( ) );
  }
  yyjson_mut_doc_set_root( doc, root );
  size_t len;
  char* str = yyjson_mut_write( doc, 0, &len );
  yyjson_mut_doc_free( doc );
}

void marionette::send_command( std::string_view cmd, yyjson_mut_val* args, packet* p ) {
  msg_id += 1; // maybe add this after confirming success of command, after sending the data that is
  create_base_packet( msg_id, cmd, p );
  yyjson_mut_arr_add_val( p->root, args );
  p->json = yyjson_mut_write( p->doc, 0, &p->len );
  char buf[32];
  auto [ptr, ec] = std::to_chars( buf, buf + 32, p->len );
  cl.send( buf, ptr - buf );
  cl.send( ":" );
  cl.send( p->json, p->len );
}

void marionette::send_command( std::string_view cmd, yyjson_mut_val* args, packet& p ) {
  msg_id += 1;
  create_base_packet( msg_id, cmd, p );
  yyjson_mut_arr_add_val( p.root, args );
  p.json = yyjson_mut_write( p.doc, 0, &p.len );
  char buf[32];
  auto [ptr, ec] = std::to_chars( buf, buf + 32, p.len );
  // use the send vector here, from client
  cl.send( buf, ptr - buf );
  cl.send( ":" );
  cl.send( p.json, p.len );
}

// check NTL library : for numbers theory

void marionette::recv_reply( std::string_view& reply ) {
  session.reset( );
  cl.recv( );
  reply = session.buffer;
}

void marionette::recv_reply( ) {
  session.reset( );
  cl.recv( );
}

void marionette::parse_context( client_context* ctx, server_response* res ) {
  this->j_str = ctx->json;
  this->json = this->parser.iterate( this->j_str );
  simdjson::ondemand::array arr = this->json.get_array( );
  auto it = arr.begin( );
  res->type = *( it );
  res->msg_id = *( ++it );
  auto err = ( *( ++it ) );
  if ( !err.error( ) ) {
    if ( err.is_null( ) ) {
      res->has_error = false;
      auto j = *( ++it );
      if ( !j.error( ) ) {
        res->json = j.get_object( ).raw_json( ).value( );
      }
    }
    else {
      res->error = err.get_string( ).value( );
      res->has_error = true;
    }
  }
  auto j = ( *( ++it ) );
  if ( !j.error( ) ) {
    if ( !j.is_null( ) ) res->json = j.get_string( ).value( );
  }
}

void marionette::parse_context( client_context& ctx, server_response& res ) {
  this->j_str = ctx.json;
  this->json = this->parser.iterate( this->j_str );
  simdjson::ondemand::array arr = this->json.get_array( );
  auto it = arr.begin( );
  res.type = *( it );
  res.msg_id = *( ++it );
  auto err = ( *( ++it ) );
  if ( !err.error( ) ) {
    if ( err.is_null( ) ) {
      res.has_error = false;
      auto j = *( ++it );
      if ( !j.error( ) ) {
        res.json = j.raw_json( ).value( );
      }
    }
    else {
      res.error = err.raw_json( ).value( );
      res.has_error = true;
    }
  }
}

int write_cb( void* ptr, size_t len, void* ctx ) {
  auto con = static_cast<client_context*>( ctx );
  con->buffer.append( static_cast<char*>( ptr ), len );
  if ( con->got_length ) {
    auto json = con->buffer.subview( con->pos + 1 );
    if ( json.size( ) == con->len ) {
      con->json = json;
      con->got_length = false;
      con->buffer.clear( );
      return recv_status::finish;
    }
    else return recv_status::cont;
  }
  else {
    con->pos = con->buffer.find( ':' );
    if ( con->pos == std::string::npos ) return recv_status::cont;
    else {
      con->len = std::stoul( con->buffer.substr( 0, con->pos ) );
      auto json = con->buffer.subview( con->pos + 1 );
      if ( json.size( ) == con->len ) {
        con->json = json;
        con->got_length = false;
        con->buffer.clear( );
        return recv_status::finish;
      }
      else {
        con->got_length = true;
        return recv_status::cont;
      }
    }
  }
}

void marionette::debug_current_url( ) {
  ++msg_id;
  pack.create_doc_new( );
  yyjson_mut_val* args = yyjson_mut_obj( pack.doc );
  send_command( webdriver::get_current_url, args, pack );
  recv_reply( );
  error::debug( parse_context( session, reply ) );
  std::cout << "[marionette] current url: " << reply.json;
}

marionette::marionette( ) {
  this->msg_id = 0;
}

marionette::~marionette( ) {

}

void packet::create_doc( ) {
  if ( doc ) { 
    yyjson_mut_doc_free( doc );
  }
  doc = yyjson_mut_doc_new( nullptr );
}

void packet::create_doc_new( ) {
  clear( );
  create_doc( );
}

void packet::clear( ) {
  if ( json ) {
    free( json );
    json = nullptr;
  }
  if ( doc ) {
    yyjson_mut_doc_free( doc );
    doc = nullptr;
    root = nullptr;
  }
  len = 0; 
}

void browser_element::display( ) const
{
  std::cout << "Element [" << id << "]\n"
    << "  UUID:         " << uuid << '\n'
    << "  Tag:          " << tag << '\n'
    << "  Text:         " << text << '\n'
    << "  Role:         " << role << '\n'
    << "  Type:         " << type << '\n'
    << "  Name:         " << name << '\n'
    << "  Value:        " << value << '\n'
    << "  Placeholder:  " << placeholder << '\n'
    << "  Aria label:   " << aria_label << '\n'
    << "  Href:         " << href << '\n'
    << "  Disabled:     " << std::boolalpha << disabled << '\n'
    << "  Visible:      " << visible << '\n'
    << "  Interactable: " << interactable << '\n'
    << "  Clickable:    " << clickable << '\n';
}

bool server_response::is_error( ) {
  return has_error;
}
