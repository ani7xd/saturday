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
  size_t ptr = 0, end = 0;
  std::string_view data;
  bool styling = false;
  while ( value > 0 ) {
    auto& chunk = stream.front( );
    ptr = chunk.find( "**", 0 );
    if ( ptr == std::string::npos ) {
      {
        std::lock_guard lock( mtx );
        std::cout.write( chunk.data( ), chunk.size( ) );
        stream.pop( );
      }
    }
    else {
      if ( styling ) {
        std::cout << "[reset]";
        chunk.replace( ptr, 2, reset_color.data( ), reset_color.size( ) );
        styling = false;
      }
      else {
        std::cout << "[blue]";
        chunk.replace( ptr, 2, blue_dark.data( ), blue_dark.size( ) );
        styling = true;
      }
      {
        std::lock_guard lock( mtx );
        std::cout.write( chunk.data( ), chunk.size( ) );
        stream.pop( );
      }
      ptr += 2;
    }
    value--;
  }
}

// needs correction, kinda sexy but ass
void context_stream::consume_even_better( ) {
  epoll_wait( epoll, r_evs, 5, -1 );
  eventfd_t value;
  eventfd_read( event_fd, &value );
  size_t ptr = 0, end = 0;
  while ( value-- ) {
    auto& chunk = stream.front( );
    size_t pos = 0;
    if ( pending_char ) {
      if ( !chunk.empty( ) && chunk[0] == '*' ) {
        if ( styling ) {
          std::cout.write( reset_color.data( ), reset_color.size( ) );
          styling = false;
        }
        else {
          std::cout.write( blue_dark.data( ), blue_dark.size( ) );
          styling = true;
        }
        pos = 1;
        pending_char = false;
      }
      else {
        std::cout.put('*');
        pending_char = false;
      }
    }
    while ( pos < chunk.size( ) ) {
      size_t ptr = chunk.find( "**", pos );
      if ( ptr == std::string::npos ) {
        if ( chunk.back( ) == '*' ) {
          if ( pos < chunk.size( ) - 1 ) {
            const auto data = chunk.subview( pos, chunk.size( ) - pos - 1 );
            std::cout.write( data.data( ), data.size( ) );
          }
          pending_char = true;
        }
        else {
          const auto data = chunk.subview( pos );
          std::cout.write( data.data( ), data.size( ) );
        }
        break;
      }

      if ( ptr > pos ) {
        const auto data = chunk.subview( pos, ptr - pos );
        std::cout.write( data.data( ), data.size( ) );
      }

      if ( styling ) {
        std::cout.write( reset_color.data( ), reset_color.size( ) );
        styling = false;
      }
      else {
        std::cout.write( blue_pro.data( ), blue_pro.size( ) );
        styling = true;
      }

      pos = ptr + 2;
    }
    stream.pop( );
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