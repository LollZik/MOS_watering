cmake_minimum_required(VERSION 3.13)

execute_process(
    COMMAND git rev-parse --short HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_COMMIT_HASH
    OUTPUT_STRIP_TRAILING_WHITESPACE
    ERROR_QUIET
)

# Check if working directory is dirty
execute_process(
    COMMAND git diff --quiet --exit-code
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    RESULT_VARIABLE GIT_DIRTY_STATUS
)

if(GIT_DIRTY_STATUS EQUAL 0)
    set(GIT_DIRTY "clean")
else()
    set(GIT_DIRTY "dirty")
endif()

message(STATUS "Git Commit: ${GIT_COMMIT_HASH} (${GIT_DIRTY})")

project(watering_server LANGUAGES C CXX ASM VERSION 0.0.1)

set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(${CMAKE_SOURCE_DIR}/../common_data_import.cmake)

add_executable(${PROJECT_NAME}
  main.c
  logger.c
  serv.c
  recver.c
  controller.c
  creators.c
  cloud.c
  ${CMAKE_SOURCE_DIR}/../common/helpers.c
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/../common
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  CRC_SEED=${CRC_SEED}
  BOOTLOADER_ORIGIN=${BOOTLOADER_ORIGIN}
  SLOT0_ORIGIN=${SLOT0_ORIGIN}
  SLOT1_ORIGIN=${SLOT1_ORIGIN}
  SLOT2_ORIGIN=${SLOT2_ORIGIN}
)

target_include_directories(${PROJECT_NAME} PRIVATE
  ${CMAKE_SOURCE_DIR}/../common
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
  BOOTLOADER_ORIGIN=${BOOTLOADER_ORIGIN}
  SLOT0_ORIGIN=${SLOT0_ORIGIN}
  SLOT1_ORIGIN=${SLOT1_ORIGIN}
  SLOT2_ORIGIN=${SLOT2_ORIGIN}
  GIT_DIRTY="${GIT_DIRTY}"
  GIT_COMMIT_HASH="${GIT_COMMIT_HASH}"
  API_KEY="${API_KEY}"
  REDIS_PORT=${REDIS_PORT}
  REDIS_PASS="${REDIS_PASS}"
  ENDPOINT="${ENDPOINT}"
  WATERING_SSL_CERT_PATH="${WATERING_SSL_CERT_PATH}"
  PROJECT_VERSION_MAJOR=${PROJECT_VERSION_MAJOR}
  PROJECT_VERSION_MINOR=${PROJECT_VERSION_MINOR}
  PROJECT_VERSION_PATCH=${PROJECT_VERSION_PATCH}
)

find_package(CURL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE CURL::libcurl)

find_package(PkgConfig REQUIRED)
pkg_check_modules(HIREDIS REQUIRED hiredis)

target_link_libraries(${PROJECT_NAME} PRIVATE ${HIREDIS_LIBRARIES} hiredis_ssl)
target_include_directories(${PROJECT_NAME} PRIVATE ${HIREDIS_INCLUDE_DIRS} )

find_package(OpenSSL REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${OPENSSL_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})

find_package(cJSON REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE ${CJSON_LIBRARIES})
target_include_directories(${PROJECT_NAME} PRIVATE ${CJSON_INCLUDE_DIRS})
