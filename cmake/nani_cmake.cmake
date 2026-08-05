# nani_cmake.cmake
# Automatically group source files by directory structure to generate VS project filters
# Usage:
#   nani_add_library(target_name SHARED|STATIC source1.cpp source2.cpp dir/source3.cpp ...)
#   nani_add_executable(target_name source1.cpp source2.cpp dir/source3.cpp ...)
#   nani_copy_target_assets(target_name source_dir [dest_subdir])
#   nani_set_gui_executable(target_name)

# Macro: Add library and group by directory
# Parameters: target_name - target name
#             lib_type - library type (SHARED or STATIC)
#             ... - list of source files
macro(nani_add_library target_name lib_type)
	set(SOURCES ${ARGN})
	
	# Create library first
	add_library(${target_name} ${lib_type} ${SOURCES})
	
	# Then generate source groups by directory
	_nani_create_source_groups("${SOURCES}")
endmacro()

# Macro: Add executable and group by directory
macro(nani_add_executable target_name)
	set(SOURCES ${ARGN})
	
	# Create executable first
	add_executable(${target_name} ${SOURCES})
	
	# Then generate source groups by directory
	_nani_create_source_groups("${SOURCES}")
endmacro()

# Macro: Configure an executable as a GUI app (no console window).
# - Windows: WIN32_EXECUTABLE (+ MSVC /ENTRY:mainCRTStartup for int main)
# - macOS: MACOSX_BUNDLE
# - Linux: no console subsystem change needed
macro(nani_set_gui_executable target_name)
	set_target_properties(${target_name} PROPERTIES
		WIN32_EXECUTABLE TRUE
		MACOSX_BUNDLE TRUE
	)
	if(WIN32 AND MSVC)
		target_link_options(${target_name} PRIVATE "/ENTRY:mainCRTStartup")
	endif()
endmacro()

# Macro: Copy an assets directory next to the target executable (POST_BUILD).
# Parameters:
#   target_name - target whose output directory receives the copy
#   source_dir  - absolute/source-tree directory to copy
#   dest_subdir - optional folder name under $<TARGET_FILE_DIR:...>; default "assets"
macro(nani_copy_target_assets target_name source_dir)
	if(${ARGC} GREATER 2)
		set(_nani_assets_dest "${ARGV2}")
	else()
		set(_nani_assets_dest "assets")
	endif()

	add_custom_command(TARGET ${target_name} POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E make_directory
			"$<TARGET_FILE_DIR:${target_name}>/${_nani_assets_dest}"
		COMMAND ${CMAKE_COMMAND} -E copy_directory
			"${source_dir}"
			"$<TARGET_FILE_DIR:${target_name}>/${_nani_assets_dest}"
		COMMENT "Copy ${target_name} assets beside executable"
		VERBATIM
	)
endmacro()

# Internal function: Create source groups for files
function(_nani_create_source_groups source_list)
	# Group source files by their directory
	set(source_groups)
	foreach(source ${source_list})
		get_filename_component(source_dir "${source}" DIRECTORY)
		if(source_dir)
			string(REPLACE "/" "\\" group_name "${source_dir}")
			list(APPEND source_groups ${group_name})
		endif()
	endforeach()

	# Remove duplicates and create groups
	if(source_groups)
		list(REMOVE_DUPLICATES source_groups)
		foreach(group ${source_groups})
			source_group("${group}")
		endforeach()
	endif()
	
	# Assign files to groups
	foreach(source ${source_list})
		get_filename_component(source_dir "${source}" DIRECTORY)
		if(source_dir)
			string(REPLACE "/" "\\" group_name "${source_dir}")
			source_group("${group_name}" FILES "${source}")
		else()
			source_group("" FILES "${source}")
		endif()
	endforeach()
endfunction()
