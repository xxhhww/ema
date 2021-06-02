cmake_minimum_required(VERSION 2.8)
project(emilia)

SET(CMAKE_VERBOSE_MAKEFILE ON)
SET(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O3 -fPIC -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

include_directories(.)

include_directories(/usr/local/include)
include_directories(/usr/include)
include_directories(/usr/include/mysql)

link_directories(/usr/local/lib64)
link_directories(/lib64)
link_directories(/usr/lib64)
link_directories(/usr/lib64/mysql)

SET(LIB_SRC
    emilia/log/logstream.cc
    emilia/log/logformat.cc
    emilia/log/logappender.cc
    emilia/log/logger.cc
    emilia/log/logthread.cc

    emilia/config/config.cc
    emilia/config/logconfig.cc

    emilia/util/stringutil.cc
    emilia/util/generalutil.cc
    emilia/util/codeutil.cc

    emilia/base/mutex.cc
    emilia/base/thread.cc
    emilia/base/fiber.cc
    emilia/base/toast.cc
    emilia/base/timer.cc
    emilia/base/scheduler.cc
    emilia/base/iomanger.cc

    emilia/net/address.cc
    emilia/net/socket.cc
    emilia/net/sslsocket.cc
    emilia/net/socketstream.cc
    emilia/net/tcpserver.cc

    emilia/email/email.cc
    emilia/email/smtp_client.cc

    emilia/http/http_parser.cc
    emilia/http/http_serve.cc
    emilia/http/http_servlet.cc
    emilia/http/http_session.cc
    emilia/http/http.cc

    emilia/demo/chatroom/chatserve.cc

    emilia/db/e_sqlite3.cc
    )

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

add_library(emilia SHARED ${LIB_SRC})

SET(EXECUTABLE_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/bin )
SET(LIBRARY_OUTPUT_PATH ${CMAKE_SOURCE_DIR}/lib )

SET (LIBS
    emilia
    yaml-cpp
    pthread
    jsoncpp
    sqlite3
    ${OPENSSL_LIBRARIES}
    mysqlclient
    )

add_executable(test_log1 tests/log/test_log1.cc)
add_dependencies(test_log1 emilia)
target_link_libraries(test_log1 ${LIBS})

add_executable(test_thread tests/base/test_thread.cc)
add_dependencies(test_thread emilia)
target_link_libraries(test_thread ${LIBS})

add_executable(test_fiber tests/base/test_fiber.cc)
add_dependencies(test_fiber emilia)
target_link_libraries(test_fiber ${LIBS})

add_executable(test_timer tests/base/test_timer.cc)
add_dependencies(test_timer emilia)
target_link_libraries(test_timer ${LIBS})

add_executable(test_loop tests/base/test_loop.cc)
add_dependencies(test_loop emilia)
target_link_libraries(test_loop ${LIBS})

add_executable(test_socket tests/net/test_socket.cc)
add_dependencies(test_socket emilia)
target_link_libraries(test_socket ${LIBS})

add_executable(test_iomanger tests/base/test_iomanger.cc)
add_dependencies(test_iomanger emilia)
target_link_libraries(test_iomanger ${LIBS})

add_executable(test_iomanger2 tests/base/test_iomanger2.cc)
add_dependencies(test_iomanger2 emilia)
target_link_libraries(test_iomanger2 ${LIBS})

add_executable(test_tcpserver tests/net/test_tcpserver.cc)
add_dependencies(test_tcpserver emilia)
target_link_libraries(test_tcpserver ${LIBS})

add_executable(test_chatroom tests/demo/test_chatroom.cc)
add_dependencies(test_chatroom emilia)
target_link_libraries(test_chatroom ${LIBS})

add_executable(test_smtp tests/smtp/test_smtp.cc)
add_dependencies(test_smtp emilia)
target_link_libraries(test_smtp ${LIBS})

add_executable(test_sqlite3 tests/db/test_sqlite3.cc)
add_dependencies(test_sqlite3 emilia)
target_link_libraries(test_sqlite3 ${LIBS})

add_executable(test_config1 tests/config/test_config1.cc)
add_dependencies(test_config1 emilia)
target_link_libraries(test_config1 ${LIBS})

add_executable(test_logconfig tests/config/test_logconfig.cc)
add_dependencies(test_logconfig emilia)
target_link_libraries(test_logconfig ${LIBS})
