#include "../../include/marionette/client.h"

ssize_t client::sendv( iovec* v, size_t n ) {
  this->ev.events = EPOLLOUT; 
  epoll_ctl( this->epoll, EPOLL_CTL_MOD, this->fd, &this->ev );
  s_bytes = 0;
  size_t index = 0;
  ssize_t bytes;
  while ( n > index ) {
    epoll_wait( epoll, &ev, 1, -1 );  
    bytes = ::writev( fd, v + index, n - index );
    if ( bytes > 0 ) {
      while ( index < n && bytes >= v[index].iov_len ) {
        bytes -= v[index].iov_len;
        index++;
      }
      v[index].iov_base = static_cast<char*>( v[index].iov_base ) + bytes;
      v[index].iov_len -= bytes;
      s_bytes += bytes;
    }
    else if ( bytes == -1 ) {
      if ( errno == EAGAIN || errno == EINTR ) continue;
      else {
        this->err = errno;
        std::cout << "error sendv: " << strerror( errno ) << "\n";
        break;
      }
    }
    else break;
  //   if ( bytes == -1 ) {
  //     if ( errno == EAGAIN || errno == EINTR ) continue;
  //     else {
  //       this->err = errno;
  //       std::cout << "error sendv: " << strerror( errno ) << "\n";
  //       return;
  //     }
  //   }
  //   else if ( bytes == 0 ) continue;
  //   else {
  //     while ( index < n && bytes >= v[index].iov_len ) {
  //       bytes -= v[index].iov_len;
  //       index++;
  //     }
  //     v[index].iov_base = static_cast<char*>( v[index].iov_base ) + bytes;
  //     v[index].iov_len -= bytes;
  //   }
  // }
  }
  return s_bytes;
}

// implement recvv
// and properly
ssize_t client::recvv( const struct iovec* vec, size_t nvec ) {
  // might need later
  // if ( vec == nullptr || nvec == 0 ) {
  //   u_err = "vec is NULL or vec count is 0";
  //   return -1;
  // }
  this->ev.events = EPOLLIN; 
  epoll_ctl( this->epoll, EPOLL_CTL_MOD, this->fd, &this->ev );
  ssize_t bytes;
  this->r_bytes = 0;
  while ( true ) {
    epoll_wait( epoll, &ret_ev, 1, -1 );
    bytes = ::readv( fd, vec, nvec );
    if ( bytes > 0 ) {
      r_bytes += bytes;
    }
    else if ( bytes == 0 ) {
      u_err = "connection shutdown";
      break;
    }
    else {
      err = errno;
      u_err = "recv v failed"; 
      return -1;
    }
  }
  return r_bytes;
}

ssize_t client::recv( ) {
  this->ev.events = EPOLLIN; 
  epoll_ctl( this->epoll, EPOLL_CTL_MOD, this->fd, &this->ev );
  ssize_t bytes;
  this->r_bytes = 0;
  while ( true ) {
    epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
    bytes = ::recv( this->fd, this->buffer.data( ), this->buffer.size( ), 0 );
    if ( bytes > 0 ) {
      if ( this->receive_cb( this->buffer.data( ), bytes, receive_cb_ctx ) == recv_status::finish ) break;
      else continue;
    } 
    else if ( bytes == 0 ) {
      u_err = "recv failed: connection closed";
      break;
    }
    else {
      if ( errno == EAGAIN || errno == EWOULDBLOCK ) continue;
      else {
        err = errno;
        u_err = "recv failed";
        // error( "recv failed" );
        return -1;
      }
    }
  };
  return r_bytes;
}

ssize_t client::send( std::string_view data ) {
  this->ev.events = EPOLLOUT;
  epoll_ctl( this->epoll, EPOLL_CTL_MOD, this->fd, &this->ev );
  ssize_t bytes = 0;
  this->s_bytes = 0;
  do {
    epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
    bytes = ::send( this->fd, data.data( ) + this->s_bytes, data.size( ) - this->s_bytes, 0 );
    if ( bytes > 0 ) {
      this->s_bytes += bytes;
    }
    else {
      if ( errno == EAGAIN || errno == EWOULDBLOCK )
        continue;
      else {
        err = errno;
        // error( "send failed" );
        u_err = "send failed";
        return -1;
      }
    }
  } while ( this->s_bytes != data.size( ) );
  return s_bytes;
}

ssize_t client::send( const void* data, size_t len ) {
  this->ev.events = EPOLLOUT;
  epoll_ctl( this->epoll, EPOLL_CTL_MOD, this->fd, &this->ev );
  ssize_t bytes = 0;
  this->s_bytes = 0;
  do {
    epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
    bytes = ::send( this->fd, ( ( char* ) data ) + this->s_bytes, len - this->s_bytes, 0 );
    if ( bytes > 0 ) {
      this->s_bytes += bytes;
    } 
    else {
      if ( errno == EAGAIN || errno == EWOULDBLOCK )
        continue;
      else {
        err = errno;
        u_err = "send failed";
        // error( "send failed" );
        return -1;
      }
    }
  } while ( this->s_bytes != len );
  return s_bytes;
}

int client::connect( const std::string& ip, uint32_t port ) {
  ev.events = EPOLLOUT;
  ev.data.fd = this->fd;
  epoll_ctl( this->epoll, EPOLL_CTL_ADD, this->fd, &this->ev );
  sockaddr_in addr = { };
  addr.sin_family = AF_INET;
  addr.sin_port = htons( port );
  int ret;
  if ( ( ret = inet_pton( AF_INET, ip.data( ), &addr.sin_addr ) ) != 1 ) {
    // error( "conversion failed" );
    this->u_err = "ip conversion to network byte failed";
    if ( ret == -1 ) err = errno;
    return -1;
  }
  else {
    if ( ::connect( this->fd, ( sockaddr* ) &addr, sizeof( addr ) ) == 0 ) return 0;
    else if ( errno == EINPROGRESS ) {
      int code;
      socklen_t len = sizeof( code );
      epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
      getsockopt( this->fd, SOL_SOCKET, SO_ERROR, &code, &len );
      if ( code == 0 ) return 0;
      else {
        // error( "connection failed", this->err );
        err = code;
        u_err = "connection failed";
        return -1;
      }
      // do {
      //   epoll_wait( this->epoll, &this->ret_ev, 1, -1 );
      //   getsockopt( this->fd, SOL_SOCKET, SO_ERROR, &code, &len );
      //   if ( code == 0 ) return 0;
      //   else {
      //     // error( "connection failed", this->err );
      //     err = code;
      //     u_err = "connection failed";
      //     return -1;
      //   }
      // } while ( code != 0 );
    }
    else {
      // error( "connection failed" );
      err = errno;
      u_err = "connection failed";
      return -1;
    }
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

char* client::error( ) {
  return strerror( err );
}

client::client( ) : err( 0 ) {
  this->ev = { };
  this->ret_ev = { };
  this->receive_cb = nullptr;
  this->receive_cb_ctx = nullptr;
}

client::~client( ) {

}