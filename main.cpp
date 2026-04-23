#include <iostream>
#include <thread>
#include <fstream>
#include <unistd.h>
#include <alsa/asoundlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <poll.h>
#include <whisper.h>
#include <cpr/cpr.h>
#include <nlohmann/json.hpp>
#include "include/microphone.h"

std::string extract(const std::string& json) {
  auto start = json.find("\"response\":\"");
  if (start == std::string::npos) return "";

  start += 12;
  auto end = json.find("\"", start);
  return json.substr(start, end - start);
}

cpr::Body create_body( const std::string& command ) {
  return cpr::Body( "{\"model\": \"qwen3\",\"stream\": false,\"prompt\": \"" + command + "\"}" ); 
}

class WhisperEngine {
  private:
    struct whisper_context * ctx;
    std::string model_path;
  
  public:
    WhisperEngine(std::string path) : model_path(path) {
      ctx = whisper_init_from_file(model_path.c_str());
      if (!ctx) {
        fprintf(stderr, "error: failed to initialize whisper context\n");
      }
    }
  
    ~WhisperEngine() {
      whisper_free(ctx);
    }
  
    std::string transcribe( const std::vector<float>& pcm_data ) {
      whisper_full_params params = whisper_full_default_params( WHISPER_SAMPLING_GREEDY );
      params.print_progress = false;
      params.print_special = false;
      params.language = "en";
  
      if ( whisper_full( ctx, params, pcm_data.data(), pcm_data.size()) != 0 ) {
        return "";
      }
  
      std::string result = "";
      int n_segments = whisper_full_n_segments( ctx );
      for (int i = 0; i < n_segments; ++i) {
         result += whisper_full_get_segment_text( ctx, i );
      }
      return result;
    }
};

int main() {
  Mic* mic = new Mic;
  mic->initialize( "default", 1, 16000, 1024 );
  std::string model_path = "/mnt/Extra/models/ggml-large-v3-turbo.bin";
  WhisperEngine* whisper = new WhisperEngine( model_path );
  //whisper->transcribe(  )
  std::vector<std::vector<float>> voices;
  std::string command;
  std::vector<float> pcm;

  sleep( 2 );
  std::cout << "Recording...\n";
  sleep( 1 );

  auto& frame = mic->voice( 10 );
  
  std::fstream file{ "audio.raw", std::ios_base::app | std::ios_base::binary };
  file.write( (char*)frame.data( ), frame.size()*4 );
  if ( file.is_open( ) ) file.close( );
  command = whisper->transcribe( frame );
  std::cout << "Command >> " << command << '\n';
  /*
  auto url = cpr::Url{"http://localhost:11434/api/generate"};
  auto header = cpr::Header{{"Content-Type", "application/json"}};
  cpr::Response response; 
  std::string command;
  std::cout << ">> "; std::getline( std::cin, command );
  while ( command != "exit" ) {
    response = cpr::Post(
      url,
      header,
      create_body( command )
    );
    std::cout << "Status: " << response.status_code << "\n";
    auto json = nlohmann::json::parse( response.text );
    std::cout << "Response: " << json["response"];
    std::cout << "\n>> "; std::getline( std::cin, command );
  } 
  */
  return 0;
}