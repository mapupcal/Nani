find_package(unofficial-skia CONFIG REQUIRED)
find_package(libuv CONFIG REQUIRED)
find_package(qjs CONFIG REQUIRED)
find_package(CURL REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

add_library(nani INTERFACE)
add_library(nani::nani ALIAS nani)

target_compile_features(nani INTERFACE cxx_std_20)

target_link_libraries(nani
  INTERFACE
    unofficial::skia::skia
    $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
    qjs
    CURL::libcurl
    glfw
)
