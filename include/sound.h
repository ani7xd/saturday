#include <alsa/asoundlib.h>
#include <string>
#include <string_view>
#include <iostream>

#if !defined( _ANI_SOUND_H )

class sound {
public:
  void init( );
  void speak( const float* data, size_t n_samples );
public:
  sound( );
  ~sound( );
public:
  snd_pcm_t* handle;
};

#define _ANI_SOUND_H
#endif