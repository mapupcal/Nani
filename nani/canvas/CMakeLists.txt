find_package(OpenGL REQUIRED)
find_package(unofficial-skia CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(yoga CONFIG REQUIRED)

nani_add_library(canvas SHARED
	defs.h

	basic/basic_defs.h
	basic/color.cpp
	basic/color.h
	basic/geometry_defs.h
	basic/pointf.cpp
	basic/pointf.h
	basic/rectf.cpp
	basic/rectf.h
	basic/sizef.cpp
	basic/sizef.h

	elements/elements_defs.h
	elements/element.cpp
	elements/element.h
	elements/elements_layer.cpp
	elements/elements_layer.h
	elements/properties.cpp
	elements/properties.h

	events/event_defs.h
	events/event.cpp
	events/event.h
	events/event_filter.cpp
	events/event_filter.h
	events/event_target.cpp
	events/event_target.h
	
	cursor.cpp
	cursor.h
	screen.cpp
	screen.h

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
	yoga::yogacore
)