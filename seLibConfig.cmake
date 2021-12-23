get_filename_component(seLib_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)

set(seLib_INCLUDE_DIRS "")


if(DEFINED ENV{seLib_ROOT})
	set(seLib_ROOT "$ENV{seLib_ROOT}" CACHE STRING "" FORCE)
endif()

file(GLOB seLib_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/include/seLib/*.h)
file(GLOB seLib_INCLUDES_EXP ${CMAKE_CURRENT_LIST_DIR}/include/seLib/experimental/*.h)
file(GLOB seLib_SOURCES ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB seLib_SOURCES_EXP ${CMAKE_CURRENT_LIST_DIR}/src/experimental/*.cpp)

add_library(seLib OBJECT
)

set_target_properties(seLib PROPERTIES LINKER_LANGUAGE CXX)

target_include_directories(seLib PUBLIC
	${CMAKE_CURRENT_LIST_DIR}/include
)

target_sources(seLib PUBLIC
	${seLib_INCLUDES}
	${seLib_INCLUDES_EXP}
	#${seLib_SOURCES}
	#${seLib_SOURCES_EXP}
)


if ("${TARGET_PLATFORM}" STREQUAL "WIN32")
	file(GLOB seLib_WIN32_INCLUDES ${CMAKE_CURRENT_LIST_DIR}/Win32/include/seLib/*.h)
	file(GLOB seLib_WIN32_INCLUDES_EXP ${CMAKE_CURRENT_LIST_DIR}/Win32/include/seLib/experimental/*.h)
	file(GLOB seLib_WIN32_SOURCES ${CMAKE_CURRENT_LIST_DIR}/Win32/src/*.cpp)
	file(GLOB seLib_WIN32_SOURCES_EXP ${CMAKE_CURRENT_LIST_DIR}/Win32/src/experimental/*.cpp)

	target_include_directories(seLib PUBLIC
		${CMAKE_CURRENT_LIST_DIR}/Win32/include
	)

	target_sources(seLib PUBLIC
		${seLib_WIN32_INCLUDES}
		${seLib_WIN32_INCLUDES_EXP}
		#${seLib_WIN32_SOURCES}
		#${seLib_WIN32_SOURCES_EXP}
	)
endif ()
