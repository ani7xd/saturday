#include <alsa/asoundlib.h>
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
  void capture( );
  void convert_to_pcm( );
  const std::vector<float>& voice( );
private:
  snd_pcm_hw_params_t* params;
  snd_pcm_t* handle;
  uint32_t channels;
  uint32_t sample_rate;
  uint32_t frames;
  int current_frames;
  std::vector<int16_t> buffer;
  std::vector<float> pcm;
};

#define _ANI_MIC_H
#endif