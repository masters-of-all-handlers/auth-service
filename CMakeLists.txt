cmake_minimum_required(VERSION 3.12)
project(auth CXX)

include(third_party/userver/cmake/SetupEnvironment.cmake)
include(GNUInstallDirs)

add_subdirectory(third_party/userver)


# Common sources
add_library(${PROJECT_NAME}_objs OBJECT
	src/models/session.hpp
	src/models/user.hpp
	src/handlers/lib/auth.hpp
	src/handlers/lib/auth.cpp
	src/handlers/v1/register/view.hpp
	src/handlers/v1/register/view.cpp
	src/handlers/v1/login/view.hpp
	src/handlers/v1/login/view.cpp
	src/handlers/v1/check/view.h
	src/handlers/v1/check/view.cpp
	src/utils/check_of_input_data.h
	src/utils/check_of_input_data.cpp
	src/handlers/v2-ticket-handlers/redirect/view.hpp
    src/handlers/v2-ticket-handlers/redirect/view.cpp
	src/handlers/options/view.cpp
	src/handlers/options/view.hpp
)
target_link_libraries(${PROJECT_NAME}_objs PUBLIC userver-core userver-postgresql)


# The Service
add_executable(${PROJECT_NAME} src/main.cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE ${PROJECT_NAME}_objs)


# Unit Tests
#add_executable(${PROJECT_NAME}_unittest
#	unittests/basic.cpp
#)
#target_link_libraries(${PROJECT_NAME}_unittest PRIVATE ${PROJECT_NAME}_objs userver-utest)
#add_google_tests(${PROJECT_NAME}_unittest)
#

# Benchmarks
add_executable(${PROJECT_NAME}_benchmark
)
target_link_libraries(${PROJECT_NAME}_benchmark PRIVATE ${PROJECT_NAME}_objs userver-ubench)
add_google_benchmark_tests(${PROJECT_NAME}_benchmark)

# Functional Tests
add_subdirectory(tests)

if(DEFINED ENV{PREFIX})
	message(STATUS "Set install prefix: $ENV{PREFIX}")
	file(TO_CMAKE_PATH "$ENV{PREFIX}" PREFIX_PATH)
	set(CMAKE_INSTALL_PREFIX ${PREFIX_PATH})
endif()

set(CONFIG_VAR_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/config_vars.yaml)
set(CONFIG_FALLBACK_PATH ${CMAKE_INSTALL_PREFIX}/${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME}/dynamic_config_fallback.json)

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/configs/static_config.yaml.in ${CMAKE_CURRENT_SOURCE_DIR}/configs/static_config.yaml)

FILE(GLOB CONFIGS_FILES ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.yaml ${CMAKE_CURRENT_SOURCE_DIR}/configs/*.json)

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_BINDIR} COMPONENT ${PROJECT_NAME})
install(FILES ${CONFIGS_FILES} DESTINATION ${CMAKE_INSTALL_SYSCONFDIR}/${PROJECT_NAME} COMPONENT ${PROJECT_NAME})
