#ifndef SPINE_SHAREDLIB_H
#define SPINE_SHAREDLIB_H

#ifdef _WIN32
#define DLLIMPORT __declspec(dllimport)
#define DLLEXPORT __declspec(dllexport)
#else
#ifndef DLLIMPORT
#define DLLIMPORT
#endif
#ifndef DLLEXPORT
#define DLLEXPORT
#endif
#endif

#ifdef SPINEPLUGIN_API
#define SP_API SPINEPLUGIN_API
#else
#define SP_API
#endif

#endif
