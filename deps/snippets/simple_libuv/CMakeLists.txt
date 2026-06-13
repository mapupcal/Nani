find_package(libuv CONFIG REQUIRED)

nani_add_executable(nani_simple_libuv main.cpp)

target_compile_features(nani_simple_libuv PRIVATE cxx_std_20)

target_link_libraries(nani_simple_libuv
  PRIVATE
    $<IF:$<TARGET_EXISTS:libuv::uv_a>,libuv::uv_a,libuv::uv>
)
set_target_properties(nani_simple_libuv PROPERTIES
    FOLDER "deps/snippets"
)
