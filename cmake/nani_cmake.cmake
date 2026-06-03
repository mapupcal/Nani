# nani_cmake.cmake
# Automatically group source files by directory structure to generate VS project filters
# Usage:
#   nani_add_library(target_name SHARED|STATIC source1.cpp source2.cpp dir/source3.cpp ...)
#   nani_add_executable(target_name source1.cpp source2.cpp dir/source3.cpp ...)

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
