#include "../include/text_stream.h"

void context_stream::push( std::string_view data ) {
  {
    std::lock_guard lock( mtx );
    stream.push( { data.data( ), data.size( ) } );
  }
  eventfd_write( event_fd, 1 );
}

void context_stream::consume( ) {
  epoll_wait( epoll, r_evs, 5, -1 );
  eventfd_t value;
  eventfd_read( event_fd, &value );
  while ( value > 0 ) {
    {
      std::lock_guard lock( mtx );
      std::cout << stream.front( );
      stream.pop( );
    }
    value--;
  }
}

context_stream::context_stream( ) {
  epoll = epoll_create1( EPOLL_CLOEXEC );
  if ( epoll == -1 ) {
    std::cout << strerror( errno ) << "\n";
    return;
  }
  event_fd = eventfd( 0, EFD_NONBLOCK | EFD_CLOEXEC );
  ev = { };
  ev.data.ptr = this;
  ev.events = EPOLLIN;
  epoll_ctl( epoll, EPOLL_CTL_ADD, event_fd, &ev );
}

context_stream::~context_stream( ) {
  close( event_fd );
  close( epoll );
}