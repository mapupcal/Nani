find_package(GTest CONFIG REQUIRED)

nani_add_executable(runtime_utests
	main.cpp
	test_directory.cpp
)

target_include_directories(runtime_utests PRIVATE
	${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_compile_features(runtime_utests PRIVATE cxx_std_20)

target_link_libraries(runtime_utests PRIVATE
	runtime
	GTest::gtest_main
)

# Register test
enable_testing()
add_test(NAME canvas_tests COMMAND runtime_utests)
