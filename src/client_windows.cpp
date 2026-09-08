#ifdef _WIN32
#include "../include/client.h"
#include <algorithm>
#include <limits>
#include <stdexcept>

client::client() : fd(INVALID_SOCKET), s_bytes(0), r_bytes(0), err(0), receive_cb(nullptr), receive_cb_ctx(nullptr) {}
void client::init() {
  if (!winsock_started) {
    WSADATA data{};
    int code = WSAStartup(MAKEWORD(2,2), &data);
    if (code) throw std::runtime_error("WSAStartup failed: " + std::to_string(code));
    winsock_started = true;
  }
  buffer.resize(4096);
}
error_t client::connect(const std::string& ip, uint32_t port) {
  if (!winsock_started) init();
  if (fd != INVALID_SOCKET) { closesocket(fd); fd = INVALID_SOCKET; }
  addrinfo hints{}, *addresses = nullptr;
  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  auto service = std::to_string(port);
  err = getaddrinfo(ip.c_str(), service.c_str(), &hints, &addresses);
  if (err) { error_str = "Address lookup failed: " + std::to_string(err); return -1; }
  for (auto address = addresses; address; address = address->ai_next) {
    fd = socket(address->ai_family, address->ai_socktype, address->ai_protocol);
    if (fd == INVALID_SOCKET) continue;
    u_long nonblocking = 1;
    ioctlsocket(fd, FIONBIO, &nonblocking);
    int code = ::connect(fd, address->ai_addr, static_cast<int>(address->ai_addrlen));
    if (code == SOCKET_ERROR && WSAGetLastError() == WSAEWOULDBLOCK) {
      fd_set writes, errors;
      FD_ZERO(&writes); FD_ZERO(&errors); FD_SET(fd, &writes); FD_SET(fd, &errors);
      timeval timeout{15,0};
      code = select(0, nullptr, &writes, &errors, &timeout);
      int size = sizeof(err);
      if (code > 0) getsockopt(fd, SOL_SOCKET, SO_ERROR, reinterpret_cast<char*>(&err), &size);
      else err = code == 0 ? WSAETIMEDOUT : WSAGetLastError();
      code = err == 0 ? 0 : SOCKET_ERROR;
    } else if (code == SOCKET_ERROR) err = WSAGetLastError();
    if (code == 0) {
      nonblocking = 0;
      ioctlsocket(fd, FIONBIO, &nonblocking);
      DWORD timeout = 15000;
      setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
      setsockopt(fd, SOL_SOCKET, SO_SNDTIMEO, reinterpret_cast<const char*>(&timeout), sizeof(timeout));
      freeaddrinfo(addresses);
      error_str = "connected successfully";
      return 0;
    }
    closesocket(fd); fd = INVALID_SOCKET;
  }
  freeaddrinfo(addresses);
  error_str = "Connection failed: " + std::to_string(err);
  return -2;
}
error_t client::send(std::string_view data) { return send(data.data(), data.size()); }
error_t client::send(const void* data, size_t len) {
  s_bytes = 0;
  while (static_cast<size_t>(s_bytes) < len) {
    int count = static_cast<int>(std::min(len - s_bytes, static_cast<size_t>(INT_MAX)));
    int sent = ::send(fd, static_cast<const char*>(data) + s_bytes, count, 0);
    if (sent == SOCKET_ERROR || sent == 0) {
      err = WSAGetLastError(); error_str = "Send failed: " + std::to_string(err); return -2;
    }
    s_bytes += sent;
  }
  return 0;
}
error_t client::recv() {
  if (!receive_cb) { error_str = "Receive callback is not set"; return -1; }
  r_bytes = 0;
  for (;;) {
    int count = ::recv(fd, buffer.data(), static_cast<int>(buffer.size()), 0);
    if (count == 0) return 0;
    if (count == SOCKET_ERROR) {
      err = WSAGetLastError(); error_str = "Receive failed: " + std::to_string(err); return -2;
    }
    r_bytes += count;
    if (receive_cb(buffer.data(), count, receive_cb_ctx) == recv_status::finish) return 0;
  }
}
void client::set_receive_cb(client_callback_t cb) { receive_cb = cb; }
void client::set_receive_cb_ctx(void* ctx) { receive_cb_ctx = ctx; }
void client::error(std::string_view str) { error(str, WSAGetLastError()); }
void client::error(std::string_view str, int code) { std::cerr << str << ": " << code << '\n'; }
client::~client() {
  if (fd != INVALID_SOCKET) closesocket(fd);
  if (winsock_started) WSACleanup();
}
size_t stoul(const char* str, size_t len) {
  size_t result = 0;
  for (size_t i=0; i<len; ++i) result = result * 10 + static_cast<unsigned>(str[i]-'0');
  return result;
}
#endif
