find_package(unofficial-skia CONFIG REQUIRED)

nani_add_executable(nani_simple_skia main.cpp)

target_compile_features(nani_simple_skia PRIVATE cxx_std_20)

target_link_libraries(nani_simple_skia
  PRIVATE
    unofficial::skia::skia
)
set_target_properties(nani_simple_skia PROPERTIES
    FOLDER "deps/snippets"
)