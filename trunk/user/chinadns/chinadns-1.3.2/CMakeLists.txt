cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)

project(chinadns VERSION "1.3.2")

set(SOURCE_FILES chinadns.c local_ns_parser.c local_ns_parser.h)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

