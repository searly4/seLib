
cmake_minimum_required(VERSION 3.6)

project(UpstreamLib NONE)

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_INCLUDE_CURRENT_DIR_IN_INTERFACE ON)

set(Upstream_VERSION 0.0.1)

#include(GenerateExportHeader)
#
#add_library(seLib SHARED climbingstats.cpp)
#generate_export_header(seLib)
#
#set_property(TARGET seLib PROPERTY VERSION ${Upstream_VERSION})
#set_property(TARGET seLib PROPERTY SOVERSION 0)
#set_property(TARGET seLib PROPERTY
#	INTERFACE_seLib_MAJOR_VERSION 0)
#set_property(TARGET seLib APPEND PROPERTY
#	COMPATIBLE_INTERFACE_STRING seLib_MAJOR_VERSION
#)
#
#install(TARGETS seLib EXPORT seLibTargets
#	LIBRARY DESTINATION lib
#	ARCHIVE DESTINATION lib
#	RUNTIME DESTINATION bin
#	INCLUDES DESTINATION include
#)
#install(
#	FILES
#		climbingstats.h
#		"${CMAKE_CURRENT_BINARY_DIR}/climbingstats_export.h"
#	DESTINATION
#		include
#	COMPONENT
#		Devel
#)

if (NOT EXISTS ${CMAKE_CURRENT_BINARY_DIR})
	file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR})
endif ()

include(CMakePackageConfigHelpers)
write_basic_package_version_file(
	"${CMAKE_CURRENT_BINARY_DIR}/seLib/seLibConfigVersion.cmake"
	VERSION ${Upstream_VERSION}
	COMPATIBILITY AnyNewerVersion
)


#export(EXPORT seLibTargets
#	FILE "${CMAKE_CURRENT_BINARY_DIR}/seLib/seLibTargets.cmake"
#	NAMESPACE Upstream::
#)
configure_file(seLibConfig.cmake
	"${CMAKE_CURRENT_BINARY_DIR}/seLib/seLibConfig.cmake"
	COPYONLY
)


#set(ConfigPackageLocation ${CMAKE_CURRENT_SOURCE_DIR}/../seLib)
set(ConfigPackageLocation ${CMAKE_CURRENT_SOURCE_DIR}/../..)#${seLib_ROOT})

#install(EXPORT seLibTargets
#	FILE
#		seLibTargets.cmake
#	NAMESPACE
#		Upstream::
#	DESTINATION
#		${ConfigPackageLocation}
#)

install(
	FILES
		seLibConfig.cmake
		"${CMAKE_CURRENT_BINARY_DIR}/seLib/seLibConfigVersion.cmake"
	DESTINATION
		${ConfigPackageLocation}
	COMPONENT
		Devel
)
