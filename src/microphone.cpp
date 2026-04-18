#include "../include/microphone.h"

Mic::Mic( ) : params( nullptr ), handle( nullptr ) {
  sample_rate = 16000;
  channels = 1;
  frames = 1024;
}

Mic::Mic( const std::string& _device, uint32_t _channels, uint32_t _sample_rate, uint32_t _frames ) : params( nullptr ), handle( nullptr ) {
  this->initialize( _device, _channels, _sample_rate, _frames );
}

int Mic::initialize( const std::string& _device, uint32_t _channels, uint32_t _sample_rate, uint32_t _frames ) {
  sample_rate = _sample_rate;
  channels = _channels;
  frames = _frames;
  int rc = snd_pcm_open( &handle, "default", SND_PCM_STREAM_CAPTURE, 0 );
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
  buffer.resize( _frames );
  pcm.resize( _frames );
  return 0;
}

void Mic::capture( ) {
  current_frames = snd_pcm_readi( handle, buffer.data( ), buffer.size( ) );
}

void Mic::convert_to_pcm( ) {
  for ( int i = 0; i < current_frames; i++ )
    pcm[i] = buffer[i] / 32768.0f;
}

const std::vector<float>& Mic::voice( ) {
  capture( );
  if ( current_frames < 0 ) {
    snd_pcm_prepare( handle );
    return;
  }
  convert_to_pcm( );
  return pcm;
}