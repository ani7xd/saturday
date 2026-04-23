#include <alsa/asoundlib.h>
#include <cstdint>
#include <cmath>
#include <iostream>
#include <vector>
#include <string>

#if !defined(_ANI_MIC_H) 

class Mic {
public:
  Mic( const std::string& device, uint32_t channels, uint32_t sample_rate, uint32_t frames);
  Mic( );
  ~Mic( );
public:
  int initialize( const std::string& device );
  int initialize( const std::string& device, uint32_t channels, uint32_t sample_rate, uint32_t frames );
  inline void capture_frames( uint32_t _frames, int16_t* _buffer );
  inline void convert_int16_to_pcm( uint32_t _frames, const int16_t* src, float_t* dest );
  inline uint32_t seconds_to_frames( uint32_t _seconds, uint32_t _sample_rate );
  inline uint32_t frames_to_bytes( uint32_t _frames, uint32_t _channels, uint32_t _bytes_per_sample );
  const std::vector<float>& voice( uint32_t sec );
private:
  snd_pcm_hw_params_t* params;
  snd_pcm_t* handle;
  uint32_t channels;
  uint32_t sample_rate;
  uint32_t frames;
  uint32_t current_frames;
  std::vector<int16_t> buffer;
  std::vector<float_t> temp_buffer;
  std::vector<float_t> pcm;
  float_t scale;
};

#define _ANI_MIC_H
#endif