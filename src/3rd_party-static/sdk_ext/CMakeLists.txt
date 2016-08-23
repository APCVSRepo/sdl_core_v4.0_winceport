set(SDK_WINCE_INCLUDE_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SDK_WINCE_INCLUDE_DIRECTORY ${SDK_WINCE_INCLUDE_DIRECTORY} PARENT_SCOPE)

include_directories (
  ./include/
)

set (SDK_WINCE_SOURCES
  ./src/time_ext.c
  ./src/timeb.cpp
  ./src/process.c
  ./src/env.cpp
  ./src/redir.cpp
  ./src/ts_string.cpp
  ./src/pipe.cpp
  ./src/ChildData.cpp
  ./src/conio.c
  ./src/stat.cpp
)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:wchar_t-")

add_library("sdk_ext" ${SDK_WINCE_SOURCES})