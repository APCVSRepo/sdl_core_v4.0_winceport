# Copyright (c) 2013, Ford Motor Company
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

set(target Policy)
set(install_destination bin)
set(copy_destination ${CMAKE_BINARY_DIR}/src/appMain)
set(library_name ${CMAKE_SHARED_LIBRARY_PREFIX}${target}${CMAKE_SHARED_LIBRARY_SUFFIX})

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")


#Generation of policy table interface...
#include(${CMAKE_SOURCE_DIR}/tools/intergen/GenerateInterfaceLibrary.cmake)

include_directories (
  ./include
  ./usage_statistics/include
  ${CMAKE_SOURCE_DIR}/src/components/rpc_base/include
  ${JSONCPP_INCLUDE_DIRECTORY}
  #${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_SOURCE_DIR}/src/components/utils/include/
  ${CMAKE_SOURCE_DIR}/src/components/utils/include/utils
  ${CMAKE_SOURCE_DIR}/src/components
  ${CMAKE_SOURCE_DIR}/src/components/config_profile/include
  ${CMAKE_SOURCE_DIR}/src/components/policy/src/policy/table_struct
  ${COMPONENTS_DIR}/resumption/include
  ${LOG4CXX_INCLUDE_DIRECTORY}
)

if(CMAKE_SYSTEM_NAME STREQUAL "Windows")
	include_directories(
		${CMAKE_SOURCE_DIR}/include/win32
		${CMAKE_SOURCE_DIR}/src/3rd_party-static/pthread-win32-2.9.1
	)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
    include_directories(
      ${CMAKE_SOURCE_DIR}/include/wince
      ${CMAKE_SOURCE_DIR}/src/3rd_party-static/pthread-win32-2.9.1
      ${SQLITE3_INCLUDE_DIRECTORY}
    )
endif()

set(SOURCES
  ./src/policy_manager_impl.cc
  ./src/policy_helper.cc
  ./src/policy_table.cc
  ./src/sql_pt_queries.cc
  ./src/sql_pt_representation.cc
  ./src/update_status_manager.cc
  ./src/cache_manager.cc
)


add_subdirectory(usage_statistics)

include_directories(./policy_table/table_struct)
add_subdirectory(policy_table/table_struct)

set(LIBRARIES ConfigProfile policy_struct dbms jsoncpp Utils)
if (CMAKE_SYSTEM_NAME STREQUAL "QNX")
  # --- QDB Wrapper 
  include_directories (${COMPONENTS_DIR}/utils/include/utils)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
  # --- SQLite Wrapper
  include_directories(${COMPONENTS_DIR}/utils/include/utils)
  list(APPEND LIBRARIES sqlite3)
  list(APPEND LIBRARIES pthreadVC2)
elseif(CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
  include_directories(${COMPONENTS_DIR}/utils/include/utils/sqlite_wrapper)
  list(APPEND LIBRARIES sqlite3)
  list(APPEND LIBRARIES pthreadVC2)
  list(APPEND LIBRARIES ws2)
  list(APPEND LIBRARIES rpc_base)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Android")
  include_directories(sqlite_wrapper/include)
else ()
  # --- SQLite Wrapper
  include_directories (${COMPONENTS_DIR}/utils/include/utils)
  list(APPEND LIBRARIES sqlite3)
endif ()

if(MODIFY_FUNCTION_FLAGS)
if(ENABLE_LOG)
if(CMAKE_SYSTEM_NAME STREQUAL "Linux" OR CMAKE_SYSTEM_NAME STREQUAL "QNX" OR CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "Android" OR CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
  list(APPEND LIBRARIES log4cxx)
endif()
endif()
endif()

add_library(${target} SHARED ${SOURCES})
target_link_libraries(${target} ${LIBRARIES} )

if(ENABLE_LOG)
  target_link_libraries(${target} log4cxx -L${LOG4CXX_LIBS_DIRECTORY})
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Windows" OR CMAKE_SYSTEM_NAME STREQUAL "Darwin" OR CMAKE_SYSTEM_NAME STREQUAL "WindowsCE")
add_custom_target(copy_library_${target} ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_BUILD_TYPE}/${library_name}
    ${copy_destination}
    DEPENDS ${target}
    COMMENT "Copying library ${library_name}")
else()
add_custom_target(copy_library_${target} ALL
  COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_CURRENT_BINARY_DIR}/${library_name}
    ${copy_destination}
    DEPENDS ${target}
    COMMENT "Copying library ${library_name}")
endif()

install(TARGETS ${target}
  DESTINATION ${install_destination}
  PERMISSIONS OWNER_READ OWNER_WRITE OWNER_EXECUTE
    GROUP_READ GROUP_EXECUTE
    WORLD_READ WORLD_EXECUTE
)
