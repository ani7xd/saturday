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

  snd_pcm_hw_params_set_access( handle, params, SND_PCM_ACCESS_RW_INTERLEAVED );
  snd_pcm_hw_params_set_format( handle, params, SND_PCM_FORMAT_S16_LE );
  snd_pcm_hw_params_set_channels( handle, params, _channels );
  snd_pcm_hw_params_set_rate( handle, params, _sample_rate, 0 );

  snd_pcm_hw_params( handle, params );
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

const std::vector<float>& Mic::voice( uint32_t _seconds ) { 
  uint32_t _frames = seconds_to_frames( _seconds, sample_rate );
  uint32_t bytes = frames_to_bytes( _frames, channels, sizeof( int16_t ) );
  temp_buffer.resize( buffer.size( ) );
  pcm.reserve( _frames );

  for ( uint32_t i = 0; i < static_cast<uint32_t>( _frames / buffer.size( ) ); i++ ) {
    capture_frames( buffer.size( ), buffer.data( ) );
    temp_buffer.resize( current_frames );
    convert_int16_to_pcm( current_frames, buffer.data( ), temp_buffer.data( ) );
    pcm.insert( pcm.end( ), temp_buffer.begin( ), temp_buffer.end( ) );
  }
  if ( _frames % buffer.size( ) != 0 ) {
    capture_frames( _frames % buffer.size( ), buffer.data( ) );
    temp_buffer.resize( _frames % buffer.size( ) );
    convert_int16_to_pcm( _frames % buffer.size( ), buffer.data( ), temp_buffer.data( ) );
    pcm.insert( pcm.end( ), temp_buffer.begin( ), temp_buffer.end( ) );
  } 
 
  return pcm;
}