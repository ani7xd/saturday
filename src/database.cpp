#include "../include/database.h"

statement* database::prepare_statement( statement* stmt, std::string_view statement ) {
  auto stmt_p = mysql_stmt_init( this->conn );
  if ( !stmt_p ) {
    std::cout << "mysql_stmt_init failed\n";
    return nullptr;
  }
  if ( mysql_stmt_prepare( stmt_p, statement.data( ), statement.size( ) ) ) {
    std::cout << "prepare failed: " << mysql_stmt_error( stmt_p ) << '\n';
  }
  unsigned long count = mysql_stmt_param_count( stmt_p );
  stmt->param.count = count;
  stmt->stmt = stmt_p;
  return stmt;
}

void database::connect( std::string_view host, std::string_view user, std::string_view password, std::string_view database ) {
  if ( !mysql_real_connect( this->conn, host.data( ), user.data( ), password.data( ), database.data( ), 3306, nullptr, 0 ) ) {
    std::cout << mysql_error( this->conn );
  }
}

void database::bind_params( statement* stmt ) {
  mysql_stmt_bind_param( stmt->stmt, stmt->param.buffer.data( ) );
}

void database::bind_result( statement* stmt ) {
  mysql_stmt_bind_result( stmt->stmt, stmt->result.buffer.data( ) );
}

void database::execute( statement* stmt ) {
  if ( mysql_stmt_execute( stmt->stmt ) != 0 )
    std::cout << "failed execute: " << mysql_stmt_error( stmt->stmt ) << '\n';
}

int database::fetch( statement* stmt ) {
  return mysql_stmt_fetch( stmt->stmt );
}

void database::commit( ) {
  mysql_commit( this->conn );
}

uint64_t database::get_last_insert_id( ) {
  return mysql_insert_id( this->conn );
}

void database::set_autocommit( bool value ) {
  mysql_autocommit( this->conn, value ); 
}

void database::load_stmt_file( const std::string& file, std::string& out ) {
  std::ifstream f{ file };
  if ( f.is_open( ) ) {
    f.seekg( 0, std::ios::end );
    size_t pos = f.tellg( );
    f.seekg( 0 );
    out.resize( pos );
    f.read( out.data( ), pos );
    f.close( );
  } else std::cout << "failed to open file...\n";
}

void database::initialize( ) {
  conn = mysql_init( nullptr );
  if ( !conn )
    std::cout << "failed to init database\n";
}

database::database( ) {

}

database::~database( ) {
  if ( this->conn ) mysql_close( this->conn );
}

void series::display( ) {
  std::cout
    << "========================================\n"
    << "Title           : " << title << '\n'
    << "Alt Titles      : " << alternate_titles << '\n'
    << "Slug            : " << slug << '\n'
    << "URL             : " << url << '\n'
    << "Cover URL       : " << cover_url << '\n'
    << "Author          : " << author << '\n'
    << "Artist          : " << artist << '\n'
    << "Status          : " << status << '\n'
    << "Type            : " << type   << '\n'
    << "Rating          : " << rating << '\n'
    << "Rating Count    : " << rating_count << '\n'
    << "Total Chapters  : " << total_chapters << '\n';

  std::cout << "Genres          : ";

  if (genres.empty()) {
    std::cout << "None";
  } else {
    for (size_t i = 0; i < genres.size(); ++i) {
      std::cout << genres[i];

      if (i + 1 < genres.size())
        std::cout << ", ";
    }
  }

  std::cout
    << "\n\nDescription:\n"
    << description
    << "\n========================================\n";
}

std::string_view series::cover_name( ) {
  return cover_url.subview( cover_url.find_last_of( '/' ) + 1 );
}


