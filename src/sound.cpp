#ifndef _WIN32
#include "../include/sound.h"

void sound::speak( const float* data, size_t n_samples ) {
  snd_pcm_writei( this->handle, data, n_samples );
}

void sound::init( ) {
  int ret = snd_pcm_open( &this->handle, "default", snd_pcm_stream_t::SND_PCM_STREAM_PLAYBACK, 0 );
  if ( ret < 0 ) {
    std::cerr << "Failed to open playback device: " << snd_strerror( ret ) << '\n';
    return;
  }
  snd_pcm_hw_params_t* params;
  snd_pcm_hw_params_alloca( &params );

  ret = snd_pcm_hw_params_any( this->handle, params );
  if ( ret < 0 ) {
    std::cerr << snd_strerror( ret ) << '\n';
    return;
  }
  snd_pcm_hw_params_set_access( this->handle, params, SND_PCM_ACCESS_RW_INTERLEAVED );
  snd_pcm_hw_params_set_format( this->handle, params, SND_PCM_FORMAT_FLOAT_LE );
  snd_pcm_hw_params_set_channels( this->handle, params, 1 );
  unsigned int rate = 22050;

  snd_pcm_hw_params_set_rate_near( this->handle, params, &rate, nullptr );
  snd_pcm_hw_params( this->handle, params );
  snd_pcm_prepare( this->handle );
}

sound::sound( ) {
  this->handle = nullptr;
}

sound::~sound( ) {

}
#endif
