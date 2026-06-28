find_package(OpenGL REQUIRED)
find_package(unofficial-skia CONFIG REQUIRED)
find_package(glfw3 CONFIG REQUIRED)
find_package(yoga CONFIG REQUIRED)
find_package(pugixml CONFIG REQUIRED)

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
	basic/marginsf.cpp
	basic/marginsf.h
	basic/polygonf.cpp
	basic/polygonf.h
	basic/transformf.cpp
	basic/transformf.h

	elements/elements_defs.h
	elements/element.cpp
	elements/element.h
	elements/elements_layer.cpp
	elements/elements_layer.h
	elements/element_states.cpp
	elements/element_states.h
	elements/element_visibility.cpp
	elements/element_visibility.h
	elements/styles.cpp
	elements/styles.h

	visuals/visuals_defs.h
	visuals/visual.cpp
	visuals/visual.h
	visuals/view.cpp
	visuals/view.h

	events/event_defs.h
	events/event.cpp
	events/event.h
	events/event_filter.cpp
	events/event_filter.h
	events/event_target.cpp
	events/event_target.h
	events/target_ptr.hpp
	
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
	internal/yoga_utils.cpp
	internal/yoga_utils.h
	internal/computed_style.cpp
	internal/computed_style.h
	internal/computed_style_builder.cpp
	internal/computed_style_builder.h
)
target_include_directories(canvas PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
)
target_compile_definitions(canvas PRIVATE NANI_CANVAS_IMPLEMENTATION)
target_compile_features(canvas PRIVATE cxx_std_20)
target_link_libraries(canvas PRIVATE
	OpenGL::GL
	unofficial::skia::skia
	glfw
	yoga::yogacore
	pugixml::shared
	pugixml::pugixml
)