#include "client.h"
#include "types.h"
#include "error.h"
#include <b64/decode.h>
#include <fstream>
#include <vector>
#include <yyjson.h>

#if !defined( _ANI_MARIONETTE_H )

constexpr std::string_view js_script = R"(
  return arguments[0].map((e) => {
    const style = getComputedStyle(e);

    const visible =
        !!(
            e.offsetWidth ||
            e.offsetHeight ||
            e.getClientRects().length
        ) &&
        style.display !== "none" &&
        style.visibility !== "hidden" &&
        style.visibility !== "collapse" &&
        style.opacity !== "0";

    const interactable =
        visible &&
        !e.disabled &&
        e.getAttribute("aria-disabled") !== "true" &&
        style.pointerEvents !== "none";

    const clickable =
        interactable &&
        (
            e.tagName === "BUTTON" ||
            e.tagName === "A" ||
            e.getAttribute("role") === "button" ||
            e.getAttribute("role") === "link" ||
            e.hasAttribute("onclick") ||
            e.tabIndex >= 0 ||
            style.cursor === "pointer"
        );

    return {
        tag: e.tagName.toLowerCase(),
        text: e.innerText?.trim() || "",
        role: e.getAttribute("role") || "",
        type: e.getAttribute("type") || "",
        name: e.getAttribute("name") || "",
        value: e.value || "",
        placeholder: e.getAttribute("placeholder") || "",
        aria_label: e.getAttribute("aria-label") || "",
        href: e.href || "",
        disabled: !!e.disabled,
        visible,
        interactable,
        clickable
    };
});
  )";

int write_cb( void* ptr, size_t len, void* ctx );

struct perform_source {
  std::string_view type;
  std::string id;
  int64_t x, y;

};

struct client_context {
  std::string buffer;
  std::string json;
  size_t len;
  size_t pos;
  bool got_length;
  client_context( ) : got_length( false ), len( 0 ), pos( 0 ) { };
  void reset( ) {
    this->got_length = false;
    this->len = 0;
    this->pos = 0;
    this->buffer.clear( );
    this->json.clear( );
  };
};

struct packet {
  char* json;
  size_t len;
  yyjson_mut_doc* doc;
  yyjson_mut_val* root;
  void create_doc( );
  void create_doc_new( );
  void clear( );
  packet( ) : json( nullptr ), len( 0 ), doc( nullptr ), root( nullptr ) { };
  packet( char* _json, size_t _len, yyjson_mut_doc* _doc )
    : json( _json ), len( _len ), doc( _doc ) { };
  ~packet( ) { };
};

struct server_response {
  size_t type;
  size_t msg_id;
  std::string error;
  std::string json;
  bool has_error;
  bool is_error( );
};

// enum class action_source : uint8_t
// {
//   keyboard,
//   mouse,
//   wheel
// };

// enum class action_type : uint8_t
// {
//   press,
//   key_down,
//   key_up,
//   move,
//   mouse_down,
//   mouse_up,
//   click,
//   scroll,
//   pause
// };

struct browser_action {
  std::string source;
  std::string actions;
  std::string uuid;
  int64_t x, y;
  uint8_t button;
  int64_t delta_x;
  int64_t delta_y;
  uint64_t duration;
  std::string key;
  std::string text;
  browser_action( ) 
    : uuid{ }, x( 0 ), y( 0 ), button( 0 ), delta_x( 0 ), delta_y( 0 ), duration( 0 ) { }
};

class marionette {
public:
  void init( );
  void create_session( );
  void parse_context( client_context* ctx, server_response* res );
  void parse_context( client_context& ctx, server_response& res );
  void create_base_packet( size_t id, std::string_view name, packet* p );
  void create_base_packet( size_t id, std::string_view name, packet& p );
  void create_args( const std::vector<std::pair<std::string_view,std::string_view>>& args, std::string_view& ret );
  void send_command( std::string_view cmd, yyjson_mut_val* args, packet* p );
  void send_command( std::string_view cmd, yyjson_mut_val* args, packet& p );
  void check_error( std::string_view reply );
  void recv_reply( std::string_view& reply );
  void recv_reply( );
  void debug_current_url( );
  void load_key_map( );
public:
  void create_new_tab( );
  void switch_to_window( std::string_view handle );
  void navigate_window( std::string_view url );
  void window_go_back( );
  void window_go_forward( );
  void window_refresh( );
  void observe( std::string_view js, std::vector<browser_element>& elements );
  void execute_js( std::string_view script, std::string_view args, packet& p );
  void element_click( std::string_view uuid );
  void fill( std::string_view uuid, std::string_view content );
  void press_key( std::string_view uuid, std::string_view key );
  void screenshot( std::vector<std::byte>& out, bool full_page = false );
  void screenshot( std::string_view uuid, std::vector<std::byte>& out, bool full_page = false );
  // for now just string view, struct next
  void perform_actions( std::string_view uuid, packet& p );
  // struct one
  void perform_actions( browser_action* actions, size_t n_actions, packet& p );

public:
  void get_attr_name( std::string_view tag );
  void get_element_name( std::string_view tag );
  std::vector<browser_element>* get_browser_elements( ); 
public:
  yyjson_mut_val* set_perform_mouse( yyjson_mut_doc* doc, yyjson_mut_val* root );
  yyjson_mut_val* set_perform_keyboard( yyjson_mut_doc* doc, yyjson_mut_val* root );
  yyjson_mut_val* set_browser_action( browser_action& action, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void add_browser_action( browser_action& action, yyjson_mut_doc* doc, yyjson_mut_val* actions );
public:
  void mouse_move( uint64_t duration, int64_t x, int64_t y, std::string_view origin, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void mouse_move( uint64_t duration, int64_t x, int64_t y, yyjson_mut_val* origin, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void mouse_move( std::string_view uuid, uint64_t duration, int64_t x, int64_t y, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void mouse_button_click( uint64_t button, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void mouse_button_click( std::string_view uuid, uint64_t button, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void mouse_button_up( uint64_t button, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void mouse_button_down( uint64_t button, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void keyboard_key_press( std::string_view key, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void keyboard_key_up( std::string_view key, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void keyboard_key_down( std::string_view key, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void keyboard_key_up( char c, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void keyboard_key_down( char c, yyjson_mut_doc* doc, yyjson_mut_val* actions );
  void keyboard_text( std::string_view text, yyjson_mut_doc* doc, yyjson_mut_val* actions );
public:
  marionette( );
  ~marionette( );
public:
  client cl;
  std::string session_id;
  std::string tab_handle; 
  size_t msg_id;
  client_context session;
  packet pack;
  server_response reply;
  simdjson::ondemand::parser parser;
  simdjson::padded_string j_str;
  simdjson::simdjson_result<simdjson::fallback::ondemand::document> json;
  std::vector<browser_element> browser_elements;
  std::unordered_map<std::string_view, std::string_view> key_map;
};

#define _ANI_MARIONETTE_H
#endif