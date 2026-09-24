#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/uio.h>
#include <arpa/inet.h>
#include <iostream>
#include <string>
#include <string_view>
#include <string.h>

#if !defined( _ANI_CLIENT_H )

typedef int socket_t;
typedef int epoll_t;
typedef int ( *client_callback_t ) ( void* ptr, size_t len, void* ctx );

enum recv_status {
  finish = 1,
  cont = 0,
};

size_t stoul( const char* str, size_t len );

class client {
public:
  void init( );
  int connect( const std::string& ip, uint32_t port );
  void persist_connect( );
  ssize_t send( std::string_view data );
  ssize_t send( const void* data, size_t len );
  ssize_t sendv( iovec* v, size_t n );
  ssize_t recv( );
  ssize_t recvv(  const struct iovec* vec, size_t nvec );
  void set_receive_cb( client_callback_t cb );
  void set_receive_cb_ctx( void* ctx );
public:
  client( );
  static void error( std::string_view str );
  static void error( std::string_view str, int err );
  char* error( );
  ~client( );
public:
  socket_t fd;
  epoll_t epoll;
  ssize_t s_bytes;
  ssize_t r_bytes;
  std::string buffer;
  int err;
  std::string u_err;
  epoll_event ev;
  epoll_event ret_ev;
  client_callback_t receive_cb;
  void* receive_cb_ctx;
};

#define _ANI_CLIENT_H
#endif