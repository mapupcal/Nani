find_package(CURL REQUIRED)

nani_add_executable(nani_simple_curl main.cpp)

target_compile_features(nani_simple_curl PRIVATE cxx_std_20)

target_link_libraries(nani_simple_curl
  PRIVATE
    CURL::libcurl
)
set_target_properties(nani_simple_curl PROPERTIES
    FOLDER "deps/snippets"
)
