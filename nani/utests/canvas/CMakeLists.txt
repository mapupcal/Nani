find_package(GTest CONFIG REQUIRED)

nani_add_executable(canvas_utests
	main.cpp
	test_color.cpp
	test_elements.cpp
	test_window.cpp
)

target_include_directories(canvas_utests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(canvas_utests PRIVATE cxx_std_20)

target_link_libraries(canvas_utests PRIVATE
	canvas
	GTest::gtest_main
)

# Register test
enable_testing()
add_test(NAME canvas_tests COMMAND canvas_utests)
