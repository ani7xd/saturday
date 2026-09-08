#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <string>
#include <string_view>
#include <iostream>

class context_stream {
public:
  void push(std::string_view data);
  bool consume();
  void close();
private:
  std::queue<std::string> stream;
  std::mutex mtx;
  std::condition_variable ready;
  bool closed = false;
};
