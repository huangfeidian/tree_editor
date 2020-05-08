cmake_minimum_required(VERSION 3.5)

project(tree_editor LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)



set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")
endif(MSVC)

INCLUDE_DIRECTORIES(${PROJECT_SOURCE_DIR}/include)


file(GLOB UI_FILES "${PROJECT_SOURCE_DIR}/resource/*.ui")

file(GLOB_RECURSE SHARE_SRC_FILES "${PROJECT_SOURCE_DIR}/src/common/*.cpp")
file(GLOB_RECURSE SHARE_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/tree_editor/common/*.h")

file(GLOB_RECURSE EDITOR_SRC_FILES "${PROJECT_SOURCE_DIR}/src/editor/*.cpp")
file(GLOB_RECURSE EDITOR_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/tree_editor/editor/*.h")

file(GLOB_RECURSE DEBUGER_SRC_FILES "${PROJECT_SOURCE_DIR}/src/debugger/*.cpp")
file(GLOB_RECURSE DEBUGER_HEADER_FILES "${PROJECT_SOURCE_DIR}/include/tree_editor/debugger/*.h")

set(CMAKE_AUTOUIC_SEARCH_PATHS ${PROJECT_SOURCE_DIR}/resource/)
set(AUTOGEN_BUILD_DIR ${PROJECT_SOURCE_DIR})
find_package(Qt5 COMPONENTS Widgets REQUIRED)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)



# add log support
find_package(spdlog CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${spdlog_include_dirs})

# add json support
find_package(nlohmann_json CONFIG REQUIRED)

# add enum support
find_package(magic_enum CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${magic_enum_include_dirs})

find_package(any_container CONFIG REQUIRED)

add_library(tree_editor ${UI_FILES}  ${SHARE_SRC_FILES} ${SHARE_HEADER_FILES} ${EDITOR_SRC_FILES} ${EDITOR_HEADER_FILES})


target_link_libraries(tree_editor PRIVATE Qt5::Widgets)
target_link_libraries(tree_editor PUBLIC nlohmann_json::nlohmann_json any_container spdlog::spdlog)

add_library(tree_debugger ${UI_FILES}  ${SHARE_SRC_FILES} ${SHARE_HEADER_FILES} ${DEBUGER_SRC_FILES} ${DEBUGER_HEADER_FILES})
find_package(http_server CONFIG REQUIRED)
INCLUDE_DIRECTORIES(${http_server_include_dirs})
target_link_libraries(tree_debugger PRIVATE Qt5::Widgets)
target_link_libraries(tree_debugger PUBLIC nlohmann_json::nlohmann_json any_container spdlog::spdlog)
target_link_libraries(tree_debugger PUBLIC http_server)

add_executable(path_require ${PROJECT_SOURCE_DIR}/include/tree_editor/common/dialogs/path_config_dialog.h ${PROJECT_SOURCE_DIR}/src/common/dialogs/path_config_dialog.cpp ${PROJECT_SOURCE_DIR}/test/path_require/path_require_test.cpp)
target_link_libraries(path_require PUBLIC nlohmann_json::nlohmann_json PRIVATE Qt5::Widgets)

file(GLOB MATH_HEADER_FILES "${PROJECT_SOURCE_DIR}/example/math_tree/*.h")
file(GLOB MATH_SRC_FILES "${PROJECT_SOURCE_DIR}/example/math_tree/*.cpp")
add_executable(math_tree ${UI_FILES}  ${SHARE_SRC_FILES} ${SHARE_HEADER_FILES} ${EDITOR_SRC_FILES} ${EDITOR_HEADER_FILES} ${MATH_HEADER_FILES} ${MATH_SRC_FILES})
target_link_libraries(math_tree PRIVATE Qt5::Widgets)
target_link_libraries(math_tree PUBLIC nlohmann_json::nlohmann_json any_container spdlog::spdlog)

add_executable(debug_server ${PROJECT_SOURCE_DIR}/example/debug_server/echo_debug_server.cpp)
target_link_libraries(debug_server PUBLIC nlohmann_json::nlohmann_json any_container http_server spdlog::spdlog)

foreach(p LIB INCLUDE)
	set(var CMAKE_INSTALL_${p}DIR)
	if(NOT IS_ABSOLUTE "${${var}}")
		set(${var} "${CMAKE_INSTALL_PREFIX}/${p}")
	endif()
endforeach()

export(TARGETS tree_debugger tree_editor
     FILE ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Targets.cmake)

file(WRITE
    ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
    "include(\${CMAKE_CURRENT_LIST_DIR}/${CMAKE_PROJECT_NAME}Targets.cmake)\n")

install(FILES
        ${CMAKE_BINARY_DIR}/${CMAKE_PROJECT_NAME}Config.cmake
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})

install(TARGETS tree_debugger tree_editor EXPORT ${CMAKE_PROJECT_NAME}Targets
   ARCHIVE  DESTINATION lib
   LIBRARY DESTINATION lib
  )

install(DIRECTORY ${CMAKE_SOURCE_DIR}/include/ DESTINATION include)

install(EXPORT ${CMAKE_PROJECT_NAME}Targets
        NAMESPACE tree_editor::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/${CMAKE_PROJECT_NAME})