#include <iostream>
#include <thread>
#include <fstream>
#include <unordered_map>
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

//cpr::Body create_body( const std::string& command ) {
//  return cpr::Body( "{\"model\": \"qwen3\",\"stream\": false,\"prompt\": \"" + command + "\"}" ); 
//}

cpr::Body create_body( const std::string& command ) {
  nlohmann::json body;
  body["model"] = "qwen3.5:9b";
  body["stream"] = false;
  body["messages"] = nlohmann::json::array({

    {
        {"role", "system"},
        {"content",
R"(You are an AI assistant with access to tools.

Available tools:

1. get_cpu_usage()
Returns current CPU utilization.

2. get_ram_usage()
Returns current memory usage.

3. read_file(path)
Reads a file from disk.

4. create_file(path)
Create a file on disk

5. write_file(content, path)
Open the file and write the content to it

6. exit()
Exits my application

When a tool is needed, respond ONLY with valid JSON:

{
"type": "tool_call",
"tool": "tool_name",
"arguments": {}
}

Examples:

{
"type": "tool_call",
"tool": "get_cpu_usage",
"arguments": {}
}

{
"type": "tool_call",
"tool": "read_file",
"arguments": {
"path": "/etc/passwd"
}
}

If no tool is required:

{
"type": "response",
"content": "your answer"
}

Never output markdown.
Never output explanations.
Never output text outside JSON.
)"}
    },

    {
        {"role", "user"},
        { "content", command }
    }
});

  return cpr::Body(body.dump());
}

// class WhisperEngine {
//   private:
//     struct whisper_context * ctx;
//     std::string model_path;
  
//   public:
//     WhisperEngine(std::string path) : model_path(path) {
//       ctx = whisper_init_from_file(model_path.c_str());
//       if (!ctx) {
//         fprintf(stderr, "error: failed to initialize whisper context\n");
//       }
//     }
  
//     ~WhisperEngine() {
//       whisper_free(ctx);
//     }
  
//     std::string transcribe( const std::vector<float>& pcm_data ) {
//       whisper_full_params params = whisper_full_default_params( WHISPER_SAMPLING_GREEDY );
//       params.print_progress = false;
//       params.print_special = false;
//       params.print_realtime   = false;
//       params.print_timestamps = false;
//       params.debug_mode = false;
//       params.language = "en";
  
//       if ( whisper_full( ctx, params, pcm_data.data(), pcm_data.size() ) != 0 ) {
//         return "";
//       }
  
//       std::string result = "";
//       int n_segments = whisper_full_n_segments( ctx );
//       for (int i = 0; i < n_segments; ++i) {
//          result += whisper_full_get_segment_text( ctx, i );
//       }
//       return result;
//     }
// };

static std::unordered_map<std::string, std::string> command_maps = {
  { "create_file", "touch" },
  { "read_file", "cat" }
};

pid_t create_file( const std::string& path ) {
  pid_t pid = fork();
  if ( pid == 0 )
  {
    char* argv[] = {
      ( char* ) "touch",
      ( char* ) path.c_str( ),
      nullptr
    };

    execvp( argv[0], argv );
    _exit( 1 );
  } else return pid;
}

pid_t write_file( const std::string& content, const std::string& path ) {
  pid_t pid = fork();
  if ( pid == 0 )
  {
    std::fstream file{ path, std::ios_base::app };
    if ( file.is_open( ) ) {
      file.write( content.data( ), content.size( ) );
      file.close( );
    }
    _exit( 1 );
  } else return pid;
}
int main() {
  //Mic* mic = new Mic;
  //mic->initialize( "pipewire", 1, 16000, 1024 );
  //std::string model_path = "/mnt/Extra/models/ggml-large-v3-turbo.bin";
  //WhisperEngine* whisper = new WhisperEngine( model_path );
  //whisper->transcribe(  )
  std::vector<std::vector<float>> voices;
  std::string command;
  std::vector<float> pcm;

  //std::cout << "Recording...\n";
  //sleep( 1 );

  //std::vector<float_t> frame = mic->voice_in_parts( 5 );
  
  // std::fstream file{ "audio.raw", std::ios_base::app | std::ios_base::binary };
  // file.write( (char*)frame.data( ), frame.size()*4 );
  // if ( file.is_open( ) ) file.close( );
  //command = whisper->transcribe( frame );
  // std::cout << "Command >> " << command << '\n';
  
  auto url = cpr::Url{ "http://localhost:11434/api/chat" };
  auto header = cpr::Header{{ "Content-Type", "application/json" }};
  cpr::Response response;

  //std::cout << "response -> \n" << response.text << "\n";
  while ( true ) {
    std::cout << "command -> ";std::getline( std::cin, command );
    response = cpr::Post(
      url,
      header,
      create_body( command )
    );
    auto json = nlohmann::json::parse( response.text );
    auto content = nlohmann::json::parse( json["message"]["content"].get<std::string>() );
  
    if ( content["type"] == "tool_call" ) {
      std::cout << "tool-> " << content["tool"] << '\n';
      std::cout << "arguement-> " << content["arguments"]["path"] << "\n";
    
      if ( content["tool"] == "create_file" ) {
        waitpid( create_file( content["arguments"]["path"].get<std::string>( ) ), nullptr, 0 );
      } else if ( content["tool"] == "write_file" ) {
        waitpid( write_file( content["arguments"]["content"].get<std::string>( ), content["arguments"]["path"].get<std::string>( ) ), nullptr, 0 );
      } else if ( content["tool"] == "exit" ) {
        break;
      }
    } else {
      std::cout << "message-> " << content["content"] << '\n';
    }
  }
  //std::cout << ">> "; std::getline( std::cin, command );
  // std::transform( command.begin( ), command.end( ), command.begin( ), []( u_char c ) { return std::tolower( c ); } );
  // command.erase( command.begin( ), std::find_if( command.begin( ), command.end(),
  //   []( unsigned char c ){ return !std::isspace( c ); } ) );
  // while ( command.compare( 0, 4, "exit" ) != 0 ) {
  //   response = cpr::Post(
  //     url,
  //     header,
  //     create_body( command )
  //   );
  //   auto json = nlohmann::json::parse( response.text );
  //   std::cout << "Response: " << json["response"] << "\n";
  //   std::cout << "Recording...\n";
  //   sleep( 1 );
  //   mic->clear_voice( );
  //   frame = mic->voice_in_parts( 5 );
  //   std::cout << "done recording...\n";
  //   command.clear( );
  //   command = whisper->transcribe( frame );
  //   std::cout << "Command >> " << command << '\n';
  //   std::transform( command.begin( ), command.end( ), command.begin( ), []( u_char c ) { return std::tolower( c ); } );
  //   command.erase( command.begin( ), std::find_if( command.begin( ), command.end(),
  //   []( unsigned char c ){ return !std::isspace( c ); } ) );
  // }
  std::cout << "exiting...\n";
  return 0;
}

