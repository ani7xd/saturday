#ifdef _WIN32
#include "../include/sound.h"
#include <stdexcept>
#include <limits>
sound::sound() = default;
void sound::init() {
  if (handle) return;
  WAVEFORMATEX format{};
  format.wFormatTag = WAVE_FORMAT_IEEE_FLOAT;
  format.nChannels = 1;
  format.nSamplesPerSec = 22050;
  format.wBitsPerSample = 32;
  format.nBlockAlign = 4;
  format.nAvgBytesPerSec = format.nSamplesPerSec * format.nBlockAlign;
  audio_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
  if (!audio_event) throw std::runtime_error("Cannot create playback event");
  auto result = waveOutOpen(&handle, WAVE_MAPPER, &format, reinterpret_cast<DWORD_PTR>(audio_event), 0, CALLBACK_EVENT);
  if (result != MMSYSERR_NOERROR) {
    CloseHandle(audio_event); audio_event = nullptr; handle = nullptr;
    throw std::runtime_error("Cannot open Windows audio output: " + std::to_string(result));
  }
}
void sound::speak(const float* data, size_t count) {
  if (!count) return;
  if (count > std::numeric_limits<DWORD>::max() / sizeof(float)) throw std::runtime_error("Audio buffer too large");
  init();
  WAVEHDR header{};
  header.lpData = reinterpret_cast<LPSTR>(const_cast<float*>(data));
  header.dwBufferLength = static_cast<DWORD>(count * sizeof(float));
  if (waveOutPrepareHeader(handle, &header, sizeof(header))) throw std::runtime_error("Cannot prepare playback buffer");
  ResetEvent(audio_event);
  if (waveOutWrite(handle, &header, sizeof(header))) {
    waveOutUnprepareHeader(handle, &header, sizeof(header));
    throw std::runtime_error("Cannot play audio");
  }
  while (!(header.dwFlags & WHDR_DONE)) WaitForSingleObject(audio_event, INFINITE);
  waveOutUnprepareHeader(handle, &header, sizeof(header));
}
sound::~sound() {
  if (handle) { waveOutReset(handle); waveOutClose(handle); }
  if (audio_event) CloseHandle(audio_event);
}
#endif
