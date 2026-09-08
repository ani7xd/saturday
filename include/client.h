#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
using socket_t = SOCKET;
using error_t = int;
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/epoll.h>
using socket_t = int;
#endif
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <string>
#include <string_view>
#include <string.h>

#if !defined( _ANI_CLIENT_H )


typedef int epoll_t;

enum recv_status {
  finish = 1,
  cont = 0,
};

typedef int ( *client_callback_t ) ( void* ptr, size_t len, void* ctx );

size_t stoul( const char* str, size_t len );

class client {
public:
  void init( );
  error_t connect( const std::string& ip, uint32_t port );
  void persist_connect( );
  error_t send( std::string_view data );
  error_t send( const void* data, size_t len );
  error_t recv( );
  void set_receive_cb( client_callback_t cb );
  void set_receive_cb_ctx( void* ctx );
public:
  client( );
  static void error( std::string_view str );
  static void error( std::string_view str, int err );
  void error_string( );
  ~client( );
public:
  socket_t fd;
  #ifndef _WIN32
  epoll_t epoll;
  #endif
  std::ptrdiff_t s_bytes;
  ssize_t r_bytes;
  std::string buffer;
  std::string error_str;
  int err;
  #ifndef _WIN32
  epoll_event ev;
  epoll_event ret_ev;
  #else
  bool winsock_started = false;
  #endif
  client_callback_t receive_cb;
  void* receive_cb_ctx;
};

#define _ANI_CLIENT_H
#endif