#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <mmreg.h>
#else
#include <alsa/asoundlib.h>
#endif
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
#ifdef _WIN32
  HWAVEOUT handle = nullptr;
  HANDLE audio_event = nullptr;
#else
  snd_pcm_t* handle;
#endif
};

#define _ANI_SOUND_H
#endif