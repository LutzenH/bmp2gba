#include "b2g_platform.h"

#if defined(B2G_WIN32)
#include "b2g_platform_win32.c"
#elif defined(B2G_LINUX)
#error Linux is currently not implemented!
#else
#error The platform you are compiling for is currently not implemented!
#endif
