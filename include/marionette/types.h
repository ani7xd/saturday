#pragma once
#include <string>
#include <string_view>

#if !defined( _ANI_MARIONETTE_TYPE_H )

// typedef char byte_t;

struct browser_element {
  size_t id;
  std::string uuid;
  std::string name;
  std::string tag;
  std::string text;
  std::string role;
  std::string type;
  std::string value;
  std::string placeholder;
  std::string aria_label;
  std::string href;

  bool disabled;
  bool visible;
  bool interactable;
  bool clickable;
  void display( ) const;
};

namespace webdriver {
  constexpr std::string_view new_session = "WebDriver:NewSession";
  constexpr std::string_view navigate = "WebDriver:Navigate";
  constexpr std::string_view new_window = "WebDriver:NewWindow";
  constexpr std::string_view switch_to_window = "WebDriver:SwitchToWindow";
  constexpr std::string_view get_window_handle = "WebDriver:GetWindowHandle";
  constexpr std::string_view back = "WebDriver:Back";
  constexpr std::string_view forward = "WebDriver:Forward";
  constexpr std::string_view refresh = "WebDriver:Refresh";
  constexpr std::string_view find_element = "WebDriver:FindElement";
  constexpr std::string_view find_elements = "WebDriver:FindElements";
  constexpr std::string_view get_element_text = "WebDriver:GetElementText";
  constexpr std::string_view get_element_attribute = "WebDriver:GetElementAttribute";
  constexpr std::string_view get_element_property = "WebDriver:GetElementProperty";
  constexpr std::string_view get_element_tag_name = "WebDriver:GetElementTagName";
  constexpr std::string_view element_click = "WebDriver:ElementClick";
  constexpr std::string_view element_send_keys = "WebDriver:ElementSendKeys";
  constexpr std::string_view element_clear = "WebDriver:ElementClear";
  constexpr std::string_view execute_script = "WebDriver:ExecuteScript";
  constexpr std::string_view get_current_url = "WebDriver:GetCurrentURL";
  constexpr std::string_view take_screenshot = "WebDriver:TakeScreenshot";
  constexpr std::string_view element_key = "element-6066-11e4-a52e-4f735466cecf";
  constexpr std::string_view perform_actions = "WebDriver:PerformActions";
  constexpr std::string_view key_down = "keyDown";
  constexpr std::string_view key_up = "keyUp";
namespace key {
  constexpr std::string_view enter = "\xEE\x80\x87";
  constexpr std::string_view tab   = "\xEE\x80\x84";
  constexpr std::string_view backspace = "\xEE\x80\x83";
  constexpr std::string_view esc = "\xEE\x80\x8C";
  constexpr std::string_view shift = "\xEE\x80\x88";
  constexpr std::string_view ctrl = "\xEE\x80\x89";
  constexpr std::string_view alt = "\xEE\x80\x8A";
  constexpr std::string_view meta = "\xEE\x80\x8B";
  constexpr std::string_view left = "\xEE\x80\x92";
  constexpr std::string_view up = "\xEE\x80\x93";
  constexpr std::string_view right = "\xEE\x80\x94";
  constexpr std::string_view down = "\xEE\x80\x95";
}
namespace browser_action_source
{
  constexpr std::string_view keyboard = "keyboard";
  constexpr std::string_view mouse    = "mouse";
  constexpr std::string_view wheel    = "wheel";
}
namespace browser_action_type
{
  constexpr std::string_view press  = "press";
  constexpr std::string_view down   = "down";
  constexpr std::string_view up     = "up";
  constexpr std::string_view move   = "move";
  constexpr std::string_view click  = "click";
  constexpr std::string_view scroll = "scroll";
  constexpr std::string_view text =   "text";
}
}

#define _ANI_MARIONETTE_TYPE_H
#endif