#include <curl/curl.h>

#include <iostream>

int main() {
  const CURLcode init_result = curl_global_init(CURL_GLOBAL_DEFAULT);
  if (init_result != CURLE_OK) {
    std::cerr << "curl_global_init failed: " << curl_easy_strerror(init_result) << '\n';
    return 1;
  }

  curl_version_info_data* version = curl_version_info(CURLVERSION_NOW);
  if (version == nullptr) {
    std::cerr << "curl_version_info failed.\n";
    curl_global_cleanup();
    return 1;
  }

  std::cout << "libcurl version: " << version->version << '\n';

  curl_global_cleanup();
  return 0;
}
