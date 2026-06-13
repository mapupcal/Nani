find_package(qjs CONFIG REQUIRED)

nani_add_executable(nani_simple_quickjs_ng main.cpp)

target_compile_features(nani_simple_quickjs_ng PRIVATE cxx_std_20)

target_link_libraries(nani_simple_quickjs_ng
  PRIVATE
    qjs
)
set_target_properties(nani_simple_quickjs_ng PROPERTIES
    FOLDER "deps/snippets"
)
