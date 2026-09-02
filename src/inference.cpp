#include "../include/inference.h"

void inference::generate( std::string_view prompt ) {
  std::string formatted =
  "<start_of_turn>user\n" +
  std::string( prompt ) +
  "<end_of_turn>\n"
  "<start_of_turn>model\n";
  apply_chat_template( prompt, nullptr, 0 );
  std::vector<llama_token> tokens( prompt.size() + 32 );
  tokenize( formatted, tokens );
  llama_batch batch = llama_batch_get_one( tokens.data(), tokens.size() );
  if ( llama_decode( ctx, batch ) != 0 ) { std::cout << "Decode failed\n"; }
  std::string text;
  text.resize( 4096 );
  for ( int i = 0; i < 100; i++ ) {
    llama_token token = llama_sampler_sample( sampler, ctx, -1 );
    if ( llama_vocab_is_eog( vocab, token )) {
      break;
    }

    int32_t len = llama_token_to_piece( vocab, token, text.data( ), text.size( ), 0, true );
    if ( len > 0 ) std::cout.write( text.data( ), len );
    else {
      text.resize( -len );
      len = llama_token_to_piece( vocab, token, text.data( ), text.size( ), 0, true );
      std::cout.write( text.data( ), len );
    }
    llama_sampler_accept( sampler, token );
    batch = llama_batch_get_one( &token, 1 );
    if ( llama_decode(ctx, batch) != 0 ) break;
  }
}

void inference::apply_chat_template( std::string_view prompt, char* out, size_t len ) {
  
}

void inference::tokenize( std::string_view prompt, std::vector<llama_token>& tokens ) {
  int n_tokens = llama_tokenize(
    vocab,
    prompt.data(),
    prompt.size(),
    tokens.data(),
    tokens.size(),
    true,
    false
  );
  if ( n_tokens < 0 ) {
    tokens.resize( -n_tokens );
    n_tokens = llama_tokenize(
      vocab,
      prompt.data(),
      prompt.size(),
      tokens.data(),
      tokens.size(),
      true,
      false
    );
  }
  tokens.resize( n_tokens );
}

void inference::init( ) {
  set_logging( );
  llama_backend_init( );
  this->model_params = llama_model_default_params( );
  model_params.n_gpu_layers = 35;
}

void inference::load_from_file( const std::filesystem::path& path ) {
  this->model = llama_model_load_from_file( path.c_str( ), model_params );
  if ( !model ) {
    std::cerr << "Failed to load model\n";
    llama_backend_free( );
    return;
  }
  std::cout << "Model loaded successfully\n";
  llama_context_params ctx_params = llama_context_default_params();
  ctx_params.n_ctx = 8192;
  ctx_params.n_batch = 512;

  ctx = llama_init_from_model( model, ctx_params );
  if ( !ctx ) {
    std::cout << "Failed to create context\n";
    return;
  }
  vocab = llama_model_get_vocab( model );
  sampler_params = llama_sampler_chain_default_params();
  sampler = llama_sampler_chain_init( sampler_params );
  llama_sampler_chain_add( sampler, llama_sampler_init_greedy() );
  tmpl = llama_model_chat_template( model, nullptr );
  // std::cout << "tmpl is " << tmpl << "\n";
}

void inference::set_logging( ) {
  llama_log_set( logger, nullptr );
}

void logger( ggml_log_level level, const char * text, void * user_data ) {
  std::cerr << text;
  // if ( level >= GGML_LOG_LEVEL_ERROR ) {
  //   std::cerr << text;
  // }
}

inference::inference( ) {
  
}

inference::~inference( ) {
  if ( ctx ) {
    llama_free( ctx );
  }
  llama_sampler_free( sampler );
  llama_model_free( model );
  llama_backend_free( );
}