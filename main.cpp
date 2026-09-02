#include "include/model.h"

int main( ) {
  model qwen;
  qwen.initialize( );
  std::string prompt;
  std::string line;
  std::vector<std::string> images;
  while ( true ) {
    std::cout << "msg>>> ";
    while ( std::getline( std::cin, line ) ) {
      if ( line == "/end" ) break;
      if ( line.starts_with("img>>> ")) {
        images.emplace_back( line.substr( 7 ) );
      } 
      else {
        if ( !prompt.empty( ) )
          prompt += '\n';
        prompt += line;
      }
    }
    if ( prompt.empty( ) ) continue;
    qwen.send_prompt( prompt, images );
    images.clear( );
    prompt.clear( );
    line.clear( );
  }
  return 0;
}