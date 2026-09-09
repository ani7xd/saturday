#ifndef _WIN32
#include "../include/microphone.h"

Mic::Mic( ) : params( nullptr ), handle( nullptr ) {
  sample_rate = 16000;
  channels = 1;
  frames = 1024;
  scale = 1 / 32768.0f;
}

Mic::~Mic( ) {

}

Mic::Mic( const std::string& _device, uint32_t _channels, uint32_t _sample_rate, uint32_t _frames ) : params( nullptr ), handle( nullptr ) {
  scale = 1 / 32768.0f;
  this->initialize( _device, _channels, _sample_rate, _frames );
}

int Mic::initialize( const std::string& _device, uint32_t _channels, uint32_t _sample_rate, uint32_t _frames ) {
  sample_rate = _sample_rate;
  channels = _channels;
  frames = _frames;
  int rc = snd_pcm_open( &handle, _device.c_str( ), SND_PCM_STREAM_CAPTURE, 0 );
  if (rc < 0) {
    std::cerr << "cannot open audio device\n";
    return -1;
  }

  snd_pcm_hw_params_alloca( &params );
  snd_pcm_hw_params_any( handle, params );

  snd_pcm_nonblock( handle, 0 );
  snd_pcm_hw_params_set_access( handle, params, SND_PCM_ACCESS_RW_INTERLEAVED );
  snd_pcm_hw_params_set_format( handle, params, SND_PCM_FORMAT_S16_LE );
  snd_pcm_hw_params_set_channels( handle, params, _channels );
  snd_pcm_hw_params_set_rate( handle, params, _sample_rate, 0 );

  // experimenting some some, dunno what this will do
  snd_pcm_uframes_t period_size = 256;   // smaller = lower latency
  snd_pcm_uframes_t buffer_size = 1024;  // ~4 periods

  snd_pcm_hw_params_set_period_size_near(handle, params, &period_size, 0);
  snd_pcm_hw_params_set_buffer_size_near(handle, params, &buffer_size);

  snd_pcm_hw_params( handle, params );
  snd_pcm_prepare ( handle );
  buffer.resize( 32768 );
  return 0;
}

void Mic::capture_frames( uint32_t _frames, int16_t* _buffer ) {
  current_frames = snd_pcm_readi( handle, _buffer, _frames );
}

void Mic::convert_int16_to_pcm( uint32_t _frames, const int16_t* src, float_t* dest ) {
  for ( int i = 0; i < _frames; i++ )
    dest[i] = src[i] * scale;
}

uint32_t Mic::seconds_to_frames( uint32_t _seconds, uint32_t _sample_rate ) {
  return static_cast<uint32_t>( std::floor( _sample_rate * _seconds ) );
}

uint32_t Mic::frames_to_bytes( uint32_t _frames, uint32_t _channels, uint32_t _bytes_per_sample ) {
  return static_cast<uint32_t>( std::floor( _frames * _channels * _bytes_per_sample ) );
}

const std::vector<float>& Mic::voice_in_parts( uint32_t _seconds ) { 
  uint32_t _frames = seconds_to_frames( _seconds, sample_rate );
  // uint32_t bytes = frames_to_bytes( _frames, channels, sizeof( int16_t ) );
  if ( temp_buffer.size( ) != frames ) temp_buffer.resize( frames );
  if ( buffer.size( ) != frames ) buffer.resize( frames );

  uint32_t frames_received = 0;
  while ( frames_received < _frames ) {
    capture_frames( frames, buffer.data( ) );
    convert_int16_to_pcm( current_frames, buffer.data( ), temp_buffer.data( ) );
    pcm.insert( pcm.end( ), temp_buffer.begin( ), temp_buffer.end( ) );
    frames_received += current_frames;
  }
  // clear pcm or voice gets concatenated
  return pcm;
}

const std::vector<float>& Mic::voice( uint32_t _seconds ) {
  uint32_t _frames = seconds_to_frames( _seconds, sample_rate );
  uint32_t bytes = frames_to_bytes( _frames, channels, sizeof( int16_t ) );
  if ( buffer.size( ) != _frames ) buffer.resize( _frames );

  auto start = std::chrono::high_resolution_clock::now();
  this->capture_frames( _frames, buffer.data( ) );
  auto end = std::chrono::high_resolution_clock::now();

  std::cout << "Time Recorded -> " << std::chrono::duration_cast<std::chrono::seconds>( end - start ).count( ) << "\n";
  std::cout << "Current Frames -> " << current_frames << "\n";

  if ( pcm.size( ) != _frames ) pcm.resize( _frames );
  this->convert_int16_to_pcm( _frames, buffer.data( ), pcm.data( ) ); // can cause issues cause of reserve() used with size()
  std::fstream file{ "audio.raw", std::ios_base::app | std::ios_base::binary };
  if ( !file.is_open( ) )
    std::cout << "failed to open file...\n";
  else {
    file.write( reinterpret_cast<char*>( buffer.data( ) ), current_frames * 2 );
    file.close( );
  }
  return pcm;
}

void Mic::clear_voice( ) {
  this->pcm.clear( );
}

void Mic::clear_cache( ) {
  this->temp_buffer.clear( );
}
#endif
