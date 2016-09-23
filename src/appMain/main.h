#ifndef SRC_APPMAIN_MAIN_H_
#define SRC_APPMAIN_MAIN_H_

#include "build_target_lib.h"

#ifdef BUILD_TARGET_LIB
# if defined(OS_WIN32) || defined(OS_WINCE)
#  define BUILD_DLL_EXPORT __declspec(dllexport)
#  define BUILD_DLL_EXPORT_C 
# else
#  define BUILD_DLL_EXPORT 
#  define BUILD_DLL_EXPORT_C 
# endif
#else 
# if defined(OS_WIN32) || defined(OS_WINCE)
#  define BUILD_DLL_EXPORT __declspec(dllimport)
#  define BUILD_DLL_EXPORT_C extern "C" 
# else
#  define BUILD_DLL_EXPORT 
#  define BUILD_DLL_EXPORT_C 
# endif
#endif // BUILD_TARGET_LIB

BUILD_DLL_EXPORT int sdl_start(int argc,char** argv);
BUILD_DLL_EXPORT void sdl_stop();
BUILD_DLL_EXPORT void sdl_set_videostream_callback(fun_SetMediaVideoStreamSendCallback func);


#endif // SRC_APPMAIN_MAIN_H_