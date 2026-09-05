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


#if !defined (_ANI_TEXT_STREAM_H)

class context_stream {
public:
  void consume( ); 
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
};

#define _ANI_TEXT_STREAM_H
#endif