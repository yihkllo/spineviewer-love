#pragma once

#if defined(_WIN32)
#  if defined(SPINELOVE_SDK_BUILD)
#    define SL_SDK_API __declspec(dllexport)
#  elif defined(SPINELOVE_SDK_IMPORT)
#    define SL_SDK_API __declspec(dllimport)
#  else
#    define SL_SDK_API
#  endif
#elif defined(__GNUC__)
#  define SL_SDK_API __attribute__((visibility("default")))
#else
#  define SL_SDK_API
#endif

#define SL_PLUGIN_API_VERSION 1
#define SL_PLUGIN_API_SYMBOL "spinelove_plugin_api_version"
#define SL_PLUGIN_REGISTER_SYMBOL "spinelove_plugin_register"
