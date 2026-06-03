find_package(glfw3 CONFIG REQUIRED)
find_package(OpenGL REQUIRED)

nani_add_executable(nani_simple_glfw3 main.cpp)

target_compile_features(nani_simple_glfw3 PRIVATE cxx_std_20)

target_link_libraries(nani_simple_glfw3 
    ${OPENGL_gl_LIBRARY}  # 或者 OpenGL::GL
    glfw
)
