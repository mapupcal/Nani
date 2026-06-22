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
	GTest::gtest
)

set_target_properties(runtime_utests PROPERTIES
    FOLDER "utests"
)

# Register test
enable_testing()
add_test(NAME runtime_utests COMMAND runtime_utests)
