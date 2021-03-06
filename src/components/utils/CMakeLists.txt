# Copyright (c) 2015, Ford Motor Company
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
# Redistributions of source code must retain the above copyright notice, this
# list of conditions and the following disclaimer.
#
# Redistributions in binary form must reproduce the above copyright notice,
# this list of conditions and the following
# disclaimer in the documentation and/or other materials provided with the
# distribution.
#
# Neither the name of the Ford Motor Company nor the names of its contributors
# may be used to endorse or promote products derived from this software
# without specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
# IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
# ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
# LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.


set(UTILS_INCLUDE_DIR ${COMPONENTS_DIR}/utils/include)
set(UTILS_SRC_DIR ${COMPONENTS_DIR}/utils/src)

include_directories (
  ${UTILS_INCLUDE_DIR}
  ${COMPONENTS_DIR}/config_profile/include
  ${COMPONENTS_DIR}/media_manager/include
  ${COMPONENTS_DIR}/protocol_handler/include
  ${COMPONENTS_DIR}/include
  ${CMAKE_SOURCE_DIR}/include
  ${CMAKE_SOURCE_DIR}/src/3rd_party-static/libusbx-1.0.16/libusb
  ${CMAKE_SOURCE_DIR}/src/3rd_party/sqlite3
  ${LOG4CXX_INCLUDE_DIRECTORY}
  ${SQLITE3_INCLUDE_DIRECTORY}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  include_directories(
    ${CMAKE_SOURCE_DIR}/include/win32
    ${CMAKE_SOURCE_DIR}/src/3rd_party/apache-log4cxx-win32-0.10.0/apr/include
    ${CMAKE_SOURCE_DIR}/src/3rd_party/apache-log4cxx-win32-0.10.0/apr/include/arch/win32
    ${CMAKE_SOURCE_DIR}/src/3rd_party/apache-log4cxx-win32-0.10.0/apr/include
    ${CMAKE_SOURCE_DIR}/src/3rd_party-static/pthread-win32-2.9.1
  )
  if(BUILD_TESTS)
    include_directories(
      ${CMAKE_SOURCE_DIR}/src/3rd_party-static/gmock-1.7.0/gtest/include
	  ${CMAKE_SOURCE_DIR}/src/3rd_party-static/gmock-1.7.0/include
    )
  endif()
elseif(CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
  include_directories(
    ${CMAKE_SOURCE_DIR}/include/wince
    ${CMAKE_SOURCE_DIR}/src/3rd_party/apache-log4cxx-win32-0.10.0/apr/include
    ${CMAKE_SOURCE_DIR}/src/3rd_party/apache-log4cxx-win32-0.10.0/apr/include/arch/win32
    ${CMAKE_SOURCE_DIR}/src/3rd_party/apache-log4cxx-win32-0.10.0/apr/include
    ${CMAKE_SOURCE_DIR}/src/3rd_party-static/pthread-win32-2.9.1
    ${CMAKE_SOURCE_DIR}/src/3rd_party-static/sdk_ext/include
endif()
  )
  if(BUILD_TESTS)
    include_directories(
      ${CMAKE_SOURCE_DIR}/src/3rd_party-static/gmock-1.7.0/gtest/include
	  ${CMAKE_SOURCE_DIR}/src/3rd_party-static/gmock-1.7.0/include
    )
  endif()
endif()

set (SOURCES
    ${UTILS_SRC_DIR}/bitstream.cc
    ${UTILS_SRC_DIR}/conditional_variable_posix.cc
    ${UTILS_SRC_DIR}/file_system.cc
    ${UTILS_SRC_DIR}/threads/posix_thread.cc   
    ${UTILS_SRC_DIR}/threads/thread_delegate.cc
    ${UTILS_SRC_DIR}/threads/thread_validator.cc
    ${UTILS_SRC_DIR}/threads/async_runner.cc
    ${UTILS_SRC_DIR}/lock_posix.cc
    ${UTILS_SRC_DIR}/rwlock_posix.cc
    ${UTILS_SRC_DIR}/date_time.cc
    ${UTILS_SRC_DIR}/signals_linux.cc
    ${UTILS_SRC_DIR}/system.cc
    ${UTILS_SRC_DIR}/resource_usage.cc
    ${UTILS_SRC_DIR}/appenders_loader.cc
    ${UTILS_SRC_DIR}/gen_hash.cc
    ${UTILS_SRC_DIR}/convert_utils.cc
    ${UTILS_SRC_DIR}/custom_string.cc
    ${UTILS_SRC_DIR}/timer.cc
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
  list (APPEND SOURCES
    ${UTILS_SRC_DIR}/global.cc
  )
endif()

if(ENABLE_LOG)
  list(APPEND SOURCES
    ${UTILS_SRC_DIR}/push_log.cc
    ${UTILS_SRC_DIR}/log_message_loop_thread.cc
    ${UTILS_SRC_DIR}/logger_status.cc
    ${UTILS_SRC_DIR}/auto_trace.cc
    ${UTILS_SRC_DIR}/logger.cc
  )
endif()

if (BUILD_BACKTRACE_SUPPORT)
    list(APPEND SOURCES
    ${UTILS_SRC_DIR}/back_trace.cc
    )
endif()

if (CMAKE_SYSTEM_NAME STREQUAL "QNX")
    list(APPEND SOURCES
    ${UTILS_SRC_DIR}/threads/pulse_thread_delegate.cc
    )
endif()

add_library("Utils" ${SOURCES})

if (CMAKE_SYSTEM_NAME STREQUAL "QNX")
    # --- QDB Wrapper 
    add_subdirectory(./src/qdb_wrapper)
else ()
    # --- SQLite Wrapper
    add_subdirectory(./src/sqlite_wrapper)
endif ()

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
 list(APPEND LIBRARIES dl) 
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
  list(APPEND LIBRARIES sdk_ext)
endif()
if(ENABLE_LOG)
  list(APPEND LIBRARIES log4cxx -L${LOG4CXX_LIBS_DIRECTORY})
  list(APPEND LIBRARIES apr-1 -L${APR_LIBS_DIRECTORY})
  list(APPEND LIBRARIES aprutil-1 -L${APR_UTIL_LIBS_DIRECTORY})
  list(APPEND LIBRARIES ConfigProfile)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
else()
  add_dependencies(Utils install-3rd_party_logger)
endif()
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
  target_link_libraries("Utils" pthread ${RTLIB})
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
  target_link_libraries("Utils" pthreadVC2  ${RTLIB})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  target_link_libraries("Utils" kernel32)
elseif(CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
  target_link_libraries("Utils" toolhelp)
endif()

target_link_libraries("Utils" ${LIBRARIES})


if(BUILD_TESTS)
  add_subdirectory(test)
endif()

install(TARGETS "Utils"
  DESTINATION bin
  PERMISSIONS
    OWNER_READ OWNER_WRITE
    GROUP_READ
    WORLD_READ
)
