#include "text_stream.h"
#include "config.h"
#include "client.h"
#include <thread>
#include <sstream>
#include <stdexcept>
#include <fstream>
#include <filesystem>
void check(bool ok) { if (!ok) throw std::runtime_error("Test assertion failed"); }
int main() {
  context_stream stream;
  std::ostringstream captured;
  auto previous = std::cout.rdbuf(captured.rdbuf());
  std::jthread consumer([&] { while(stream.consume()) {} });
  for (int i=0; i<1000; ++i) stream.push("x");
  stream.close(); consumer.join();
  std::cout.rdbuf(previous);
  check(captured.str() == std::string(1000, 'x'));
  context_stream idle;
  std::jthread waiter([&] { check(!idle.consume()); });
  idle.close(); waiter.join();
  { std::ofstream f("test.env"); f << "SATURDAY_TEST_VALUE=\"a=b c\"\n"; }
  utils::load_env("test.env");
  std::filesystem::remove("test.env");
  check(utils::env("SATURDAY_TEST_VALUE") == "a=b c");
  check(utils::env("SATURDAY_TEST_MISSING", "fallback") == "fallback");
  client network;
  network.init();
  check(network.connect("invalid.invalid", 1) != 0);
  check(stoul("123",3) == 123);
#ifdef _WIN32
  SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
  check(listener != INVALID_SOCKET);
  sockaddr_in address{}; address.sin_family = AF_INET; address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
  check(bind(listener, reinterpret_cast<sockaddr*>(&address), sizeof(address)) == 0);
  int length = sizeof(address);
  check(getsockname(listener, reinterpret_cast<sockaddr*>(&address), &length) == 0);
  check(listen(listener, 1) == 0);
  std::jthread server([&] {
    SOCKET peer = accept(listener, nullptr, nullptr);
    check(peer != INVALID_SOCKET);
    char input[4]; int used = 0;
    while (used < 4) { int n = ::recv(peer, input + used, 4 - used, 0); check(n > 0); used += n; }
    check(std::string_view(input, 4) == "ping");
    check(::send(peer, "pong", 4, 0) == 4);
    closesocket(peer);
  });
  check(network.connect("127.0.0.1", ntohs(address.sin_port)) == 0);
  std::string reply;
  network.set_receive_cb([](void* p, size_t n, void* state) -> int {
    auto& value = *static_cast<std::string*>(state);
    value.append(static_cast<char*>(p), n);
    return value.size() >= 4 ? recv_status::finish : recv_status::cont;
  });
  network.set_receive_cb_ctx(&reply);
  check(network.send("ping") == 0);
  check(network.recv() == 0);
  check(reply == "pong");
  server.join(); closesocket(listener);
#endif
  std::cout << "Platform tests passed\n";
}
