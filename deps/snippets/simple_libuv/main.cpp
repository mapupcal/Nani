#include <uv.h>

#include <iostream>

int main() {
  uv_loop_t loop;
  const int init_result = uv_loop_init(&loop);
  if (init_result != 0) {
    std::cerr << "uv_loop_init failed: " << uv_strerror(init_result) << '\n';
    return 1;
  }

  std::cout << "libuv version: " << uv_version_string() << '\n';

  const int close_result = uv_loop_close(&loop);
  if (close_result != 0) {
    std::cerr << "uv_loop_close failed: " << uv_strerror(close_result) << '\n';
    return 1;
  }

  return 0;
}
