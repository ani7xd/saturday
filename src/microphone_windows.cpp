#ifdef _WIN32
#include "../include/microphone.h"
#include <algorithm>
#include <limits>
#include <stdexcept>
Mic::Mic() : channels(1), sample_rate(16000), frames(1024), current_frames(0), scale(1.0f / 32768.0f) {}
Mic::Mic(const std::string& device, uint32_t ch, uint32_t rate, uint32_t block) : Mic() {
  if (initialize(device, ch, rate, block)) throw std::runtime_error("Cannot open Windows microphone");
}
int Mic::initialize(const std::string& device) { return initialize(device, channels, sample_rate, frames); }
int Mic::initialize(const std::string& device, uint32_t ch, uint32_t rate, uint32_t block) {
  if (!ch || ch > 2 || !rate || !block) return -1;
  if (handle) { waveInReset(handle); waveInClose(handle); handle = nullptr; }
  if (audio_event) { CloseHandle(audio_event); audio_event = nullptr; }
  UINT id = WAVE_MAPPER;
  if (!device.empty() && device != "default") {
    try { id = static_cast<UINT>(std::stoul(device)); } catch (...) { return -1; }
  }
  WAVEFORMATEX format{};
  format.wFormatTag = WAVE_FORMAT_PCM;
  format.nChannels = static_cast<WORD>(ch);
  format.nSamplesPerSec = rate;
  format.wBitsPerSample = 16;
  format.nBlockAlign = static_cast<WORD>(ch * sizeof(int16_t));
  format.nAvgBytesPerSec = rate * format.nBlockAlign;
  audio_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (!audio_event) return -1;
  if (waveInOpen(&handle, id, &format, reinterpret_cast<DWORD_PTR>(audio_event), 0, CALLBACK_EVENT)) {
    CloseHandle(audio_event); audio_event = nullptr; handle = nullptr; return -1;
  }
  channels = ch; sample_rate = rate; frames = block;
  return 0;
}
void Mic::capture_frames(uint32_t count, int16_t* destination) {
  if (!handle) throw std::runtime_error("Microphone is not initialized");
  if (static_cast<uint64_t>(count) * channels * sizeof(int16_t) > std::numeric_limits<DWORD>::max())
    throw std::runtime_error("Recording buffer too large");
  WAVEHDR header{};
  header.lpData = reinterpret_cast<LPSTR>(destination);
  header.dwBufferLength = count * channels * sizeof(int16_t);
  if (waveInPrepareHeader(handle, &header, sizeof(header))) throw std::runtime_error("Cannot prepare recording buffer");
  ResetEvent(audio_event);
  if (waveInAddBuffer(handle, &header, sizeof(header)) || waveInStart(handle)) {
    waveInReset(handle); waveInUnprepareHeader(handle, &header, sizeof(header));
    throw std::runtime_error("Cannot start recording");
  }
  while (!(header.dwFlags & WHDR_DONE)) WaitForSingleObject(audio_event, INFINITE);
  waveInStop(handle);
  current_frames = header.dwBytesRecorded / (channels * sizeof(int16_t));
  waveInUnprepareHeader(handle, &header, sizeof(header));
}
void Mic::convert_int16_to_pcm(uint32_t count, const int16_t* src, float_t* dest) {
  for (uint32_t i = 0; i < count; ++i) dest[i] = src[i] * scale;
}
uint32_t Mic::seconds_to_frames(uint32_t seconds, uint32_t rate) {
  if (static_cast<uint64_t>(seconds) * rate > UINT32_MAX) throw std::runtime_error("Recording duration too large");
  return seconds * rate;
}
uint32_t Mic::frames_to_bytes(uint32_t count, uint32_t ch, uint32_t bytes) { return count * ch * bytes; }
const std::vector<float>& Mic::voice_in_parts(uint32_t seconds) {
  uint32_t remaining = seconds_to_frames(seconds, sample_rate);
  while (remaining) {
    uint32_t count = std::min(frames, remaining);
    buffer.resize(static_cast<size_t>(count) * channels);
    capture_frames(count, buffer.data());
    if (!current_frames) throw std::runtime_error("Microphone returned no audio");
    temp_buffer.resize(static_cast<size_t>(current_frames) * channels);
    convert_int16_to_pcm(static_cast<uint32_t>(temp_buffer.size()), buffer.data(), temp_buffer.data());
    pcm.insert(pcm.end(), temp_buffer.begin(), temp_buffer.end());
    remaining -= current_frames;
  }
  return pcm;
}
const std::vector<float>& Mic::voice(uint32_t seconds) { pcm.clear(); return voice_in_parts(seconds); }
void Mic::clear_voice() { pcm.clear(); }
void Mic::clear_cache() { temp_buffer.clear(); }
Mic::~Mic() {
  if (handle) { waveInReset(handle); waveInClose(handle); }
  if (audio_event) CloseHandle(audio_event);
}
#endif
