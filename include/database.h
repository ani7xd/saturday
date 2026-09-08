#include <iostream>
#include <fstream>
#include <string>
#include <cstring>
#include <string_view>
#include <vector>
#include <type_traits>
#include <deque>
#include <stdexcept>
#include <cassert>
#include <mysql.h>

#if !defined( _ANI_DATABASE_H )

typedef float float_t;

// also need to remove this, dont know how the scrapper
// struct got in here
struct series {
  std::string title;
  std::string alternate_titles;
  std::string slug;
  std::string description;
  std::string url;
  std::string cover_url;
  std::string cover_path;
  std::string author;
  std::string artist;
  std::string status;
  std::string type;
  std::vector<std::string> genres; 
  float_t rating;
  size_t rating_count;
  size_t total_chapters;
  void display( );
  std::string_view cover_name( );
};

// have to make this bro into a class next
struct statement {
  MYSQL_STMT* stmt;
  struct param_group {
    template<typename... Ts> void init( );
    template<typename T> void init_impl( size_t index );
    template<typename T> void bind( size_t index, T&& value );
    std::vector<MYSQL_BIND> buffer;
    std::vector<unsigned long> lengths;
    friend class database;
    private:
    uint64_t count;
  } param;
  struct result_group {
    std::vector<MYSQL_BIND> buffer;
    std::vector<unsigned long> lengths;
    using null_flag = std::remove_pointer_t<decltype(MYSQL_BIND{}.is_null)>;
    std::deque<null_flag> is_null;
    template<typename... Ts> void init( );
    template<typename T> void init_impl( size_t index );
    template<typename T> void bind( size_t index, T& value );
  } result;
  int error_code;
  void bind_params( );
  void bind_result( );
  bool free_result( );
  statement( );
  statement(const statement&) = delete;
  statement& operator=(const statement&) = delete;
  ~statement( );
};

class database {
public:
  void initialize( );
  void connect( std::string_view host, std::string_view user, std::string_view password, std::string_view database );
  statement* prepare_statement( statement* stmt, std::string_view statement );
  int execute( statement* stmt );
  int execute( const statement& stmt );
  int fetch( statement* stmt );
  int fetch( const statement& stmt );
  void commit( );
  void load_stmt_file( const std::string& file, std::string& out );
  void free_stmt( statement* stmt );
  uint64_t get_last_insert_id( );
  uint64_t get_affected_rows( );
  void set_autocommit( bool value );
public:
  // might remove this, after i make statement a class
  void bind_params( statement* stmt );
  void bind_result( statement* stmt );
  void bind_params( statement& stmt );
  void bind_result( statement& stmt );
public:
  database( );
  ~database( );
public:
  MYSQL* conn;
  int error_code;
  std::vector<statement*> stmts;
};

template<typename T>
void statement::result_group::bind( size_t index, T& value ) {
  auto& b = this->buffer[index];
  using U = std::remove_cvref_t<T>;
  if constexpr ( std::is_same_v<U, std::string> ) {
    b.buffer = value.data( );
    b.buffer_length = value.size( );
  }
  else if constexpr ( std::is_same_v<U, float> ) {
    b.buffer = &value;
  }
  else if constexpr ( std::is_same_v<U, uint64_t> ) {
    b.buffer = &value;
  }
}

template<typename... Ts>
void statement::result_group::init( ) {
  constexpr size_t N = sizeof...( Ts );
  this->buffer.resize( N );
  this->lengths.resize( N );
  this->is_null.resize( N );
  std::memset( buffer.data( ), 0, N * sizeof( MYSQL_BIND ) );
  [&]<std::size_t... Is>( std::index_sequence<Is...> ) {
        ( init_impl<Ts>( Is ), ... );
  } ( std::index_sequence_for<Ts...>{ } );
}

template<typename T>
void statement::result_group::init_impl( size_t index ) {
  using U = std::remove_cvref_t<T>;
  auto& b = this->buffer[index];
  b.is_null = &this->is_null[index];
  if constexpr ( std::is_same_v<U, std::string> ) {
    b.buffer_type = MYSQL_TYPE_STRING;
    b.length = &this->lengths[index];
  }
  else if constexpr ( std::is_same_v<U, float> ) {
    b.buffer_type = MYSQL_TYPE_FLOAT;
  }
  else if constexpr ( std::is_same_v<U, uint64_t> ) {
    b.buffer_type = MYSQL_TYPE_LONGLONG;
    b.is_unsigned = true;
  }
}


template<typename T>
void statement::param_group::bind( size_t index, T&& value ) {
  auto& b = this->buffer[index];
  using U = std::remove_cvref_t<T>;
  if constexpr ( std::is_same_v<U, std::string> ) {
    b.buffer = value.data( );
    b.buffer_length = value.size( );
    *b.length = value.size( );
  }
  else if constexpr ( std::is_same_v<U, std::string_view> ) {
    b.buffer = const_cast<char *>( value.data( ) );
    b.buffer_length = value.size( );
    *b.length = value.size( );
  }
  else if constexpr ( std::is_same_v<U, float> ) {
    b.buffer = &value;
  }
  else if constexpr ( std::is_same_v<U, uint64_t> ) {
    b.buffer = &value;
  }
}

template<typename... Ts>
void statement::param_group::init( ) {
  constexpr size_t N = sizeof...( Ts );
  assert( count == N );
  if ( count != N )
    throw std::logic_error( "Parameter count mismatch" );
  this->buffer.resize( N );
  this->lengths.resize( N );
  std::memset( this->buffer.data( ), 0, N * sizeof( MYSQL_BIND ) );
  [&]<std::size_t... Is>( std::index_sequence<Is...> ) {
        ( init_impl<Ts>( Is ), ... );
  } ( std::index_sequence_for<Ts...>{ } );
}

template<typename T>
void statement::param_group::init_impl( size_t index ) {
  using U = std::remove_cvref_t<T>;
  auto& b = this->buffer[index];
  
  if constexpr ( std::is_same_v<U, std::string> ) {
    b.buffer_type = MYSQL_TYPE_STRING;
    b.length = &this->lengths[index];
  }
  else if constexpr ( std::is_same_v<U, std::string_view> ) {
    b.buffer_type = MYSQL_TYPE_STRING;
    b.length = &this->lengths[index];
  }
  else if constexpr ( std::is_same_v<U, float> ) {
    b.buffer_type = MYSQL_TYPE_FLOAT;
  }
  else if constexpr ( std::is_same_v<U, uint64_t> ) {
    b.buffer_type = MYSQL_TYPE_LONGLONG;
    b.is_unsigned = true;
  }
}

#define _ANI_DATABASE_H
#endif
