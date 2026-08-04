nani_add_executable(canvas_snippets
	main.cpp
)

target_include_directories(canvas_snippets PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../..
)
target_compile_features(canvas_snippets PRIVATE cxx_std_20)
target_link_libraries(canvas_snippets PRIVATE
	canvas
)

# GUI app without a console window:
# - Windows: /SUBSYSTEM:WINDOWS (MSVC) or -mwindows (MinGW)
# - macOS: produce a .app bundle (no Terminal when launched from Finder)
# - Linux: no console subsystem; windowed process is already GUI-only
set_target_properties(canvas_snippets PROPERTIES
	FOLDER "snippets"
	WIN32_EXECUTABLE TRUE
	MACOSX_BUNDLE TRUE
)

if(WIN32 AND MSVC)
	# Keep the portable int main(argc, argv) entry point under WINDOWS subsystem.
	target_link_options(canvas_snippets PRIVATE "/ENTRY:mainCRTStartup")
endif()
