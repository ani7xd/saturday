#include "../include/client.h"

void client::recv( ) {
  this->ev.events = EPOLLIN; 
  epoll_ctl( this->epoll, EPOLL_CTL_MOD, this->fd, &this->ev );
  ssize_t ret;
  this->r_bytes = 0;
  while ( true ) {
    epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
    ret = ::recv( this->fd, this->buffer.data( ), this->buffer.size( ), 0 );
    if ( ret > 0 ) {
      if ( this->receive_cb( this->buffer.data( ), ret, receive_cb_ctx ) == recv_status::finish ) break;
    } 
    else if ( ret == 0 ) {
      std::cout << "connection closed...\n";
      break;
    }
    else {
      if ( errno == EAGAIN || errno == EWOULDBLOCK ) continue;
      else {
        error( "recv failed" );
        break;
      }
    }
  };
}

void client::send( std::string_view data ) {
  this->ev.events = EPOLLOUT;
  epoll_ctl( this->epoll, EPOLL_CTL_MOD, this->fd, &this->ev );
  ssize_t ret = 0;
  this->s_bytes = 0;
  do {
    epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
    ret = ::send( this->fd, data.data( ) + this->s_bytes, data.size( ) - this->s_bytes, 0 );
    if ( ret > 0 ) {
      // std::cout << "sent " << ret << " bytes" << "\n";
      this->s_bytes += ret;
    } 
    else {
      error( "send failed" );
    }
  } while ( this->s_bytes != data.size( ) );
}

void client::send( const void* data, size_t len ) {
  this->ev.events = EPOLLOUT;
  epoll_ctl( this->epoll, EPOLL_CTL_MOD, this->fd, &this->ev );
  ssize_t ret = 0;
  this->s_bytes = 0;
  do {
    epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
    ret = ::send( this->fd, ( ( char* ) data ) + this->s_bytes, len - this->s_bytes, 0 );
    if ( ret > 0 ) {
      this->s_bytes += ret;
    } 
    else {
      error( "send failed" );
    }
  } while ( this->s_bytes != len );
}

void client::connect( const std::string& ip, uint32_t port ) {
  ev.events = EPOLLOUT;
  ev.data.fd = this->fd;
  epoll_ctl( this->epoll, EPOLL_CTL_ADD, this->fd, &this->ev );
  sockaddr_in addr = { };
  addr.sin_family = AF_INET;
  addr.sin_port = htons( port );
  if ( inet_pton( AF_INET, ip.data( ), &addr.sin_addr ) != 1 )
    error( "conversion failed" );
  else {
    if ( ::connect( this->fd, ( sockaddr* ) &addr, sizeof( addr ) ) == 0 ) {
      std::cout << "connected successfully...\n";
    } 
    else if ( errno == EINPROGRESS ) {
        do {
          epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
          socklen_t len = sizeof( err );
          getsockopt( this->fd, SOL_SOCKET, SO_ERROR, &err, &len );
          if ( this->err == 0 )
            std::cout << "connected successfully...\n";
          else {
            error( "connection failed", this->err );
          }
        } while ( this->err != 0 );
    }
    else error( "connection failed" );
  }
}

void client::set_receive_cb( client_callback_t cb ) {
  this->receive_cb = cb;
}

void client::set_receive_cb_ctx( void* ctx ) {
  this->receive_cb_ctx = ctx;
}

void client::init( ) {
  this->fd = socket( AF_INET, SOCK_STREAM | SOCK_NONBLOCK, 0 );
  if ( this->fd == -1 ) {
    error( "socket creation failed" );
    return;
  }
  this->epoll = epoll_create1( 0 );
  if ( this->epoll == -1 ) {
    error( "epoll instance creation failed" );
    return;
  }
  this->buffer.reserve( 8196 );
  this->buffer.resize( 4096 );
}

size_t stoul( const char* str, size_t len ) {
  size_t value = 0;
  for ( size_t i = 0; i < len; i++ ) 
    value += value * 10 + static_cast<unsigned>( str[i] - '0' );
  return value;
}

void client::error( std::string_view str ) {
  std::cout.write( str.data( ), str.size( ) ); std::cout << " : " << strerror( errno ) << "\n";
}

void client::error( std::string_view str, int err  ) {
  std::cout.write( str.data( ), str.size( ) ); std::cout << " : " << strerror( err ) << "\n";
}

client::client( ) : err( 0 ) {
  this->ev = { };
  this->ret_ev = { };
  this->receive_cb = nullptr;
  this->receive_cb_ctx = nullptr;
}

client::~client( ) {

}