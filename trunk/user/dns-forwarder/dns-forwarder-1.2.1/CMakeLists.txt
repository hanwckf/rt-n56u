cmake_minimum_required(VERSION 2.8)
cmake_policy(SET CMP0048 NEW)

project(dns-forwarder VERSION "1.2.1")

set(SOURCE_FILES hev-dns-forwarder.c hev-event-source.c hev-event-source-timeout.c hev-ring-buffer.c hev-dns-session.c hev-event-source-fds.c hev-main.c hev-slist.c hev-event-loop.c hev-event-source-signal.c hev-memory-allocator.c)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})

