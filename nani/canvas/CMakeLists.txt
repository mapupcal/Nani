find_package(OpenGL REQUIRED)
find_package(unofficial-skia CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)

nani_add_library(canvas SHARED
	defs.h

	basic/geometry_defs.h
	basic/pointf.cpp
	basic/pointf.h
	basic/rectf.cpp
	basic/rectf.h
	basic/sizef.cpp
	basic/sizef.h
	
	cursor.cpp
	cursor.h
	screen.cpp
	screen.h

    event.cpp
    event.h
	env.cpp
	env.h
	window.cpp
	window.h

	internal/env_p.cpp
	internal/env_p.h
	internal/platform.cpp
	internal/platform.h
	internal/window_p.cpp
	internal/window_p.h
)
target_include_directories(canvas PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(canvas PRIVATE NANI_IMPLEMENTATION)
target_compile_features(canvas PRIVATE cxx_std_20)
target_link_libraries(canvas PRIVATE
	OpenGL::GL
	unofficial::skia::skia
	glfw
)