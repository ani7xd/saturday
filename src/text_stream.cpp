#include "../include/text_stream.h"

void context_stream::push(std::string_view data) {
  {
    std::lock_guard lock(mtx);
    if (closed) return;
    stream.emplace(data);
  }
  ready.notify_one();
}
bool context_stream::consume() {
  std::unique_lock lock(mtx);
  ready.wait(lock, [&] { return closed || !stream.empty(); });
  if (stream.empty()) return false;
  auto text = std::move(stream.front());
  stream.pop();
  lock.unlock();
  std::cout << text << std::flush;
  return true;
}
void context_stream::close() {
  { std::lock_guard lock(mtx); closed = true; }
  ready.notify_all();
}
