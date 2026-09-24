#include <sys/wait.h>
#include <sys/epoll.h>
#include <sys/eventfd.h>
#include <string.h>
#include <queue>
#include <mutex>
#include <iostream>
#include <string>
#include <string_view>

#if !defined(_ANI_CLIENT_H)
  typedef int epoll_t;
#endif

#if !defined (_ANI_TEXT_STREAM_H )

constexpr std::string_view blue_dark  = "\033[38;2;30;60;120m";
constexpr std::string_view blue       = "\033[38;2;60;120;220m";
constexpr std::string_view blue_pro = "\033[38;2;70;130;240m";
constexpr std::string_view blue_bright = "\033[38;2;80;160;255m";
constexpr std::string_view reset_color = "\033[39m";

class context_stream {
public:
  void consume( );
  void consume_even_better( );
  void push( std::string_view );
public:
  context_stream( );
  ~context_stream( );
public:
  std::queue<std::string> stream;
  epoll_t epoll;
  epoll_event ev;
  epoll_event r_evs[5];
  std::mutex mtx;
  int event_fd;
  bool styling;
  bool pending_char;
};

#define _ANI_TEXT_STREAM_H
#endif