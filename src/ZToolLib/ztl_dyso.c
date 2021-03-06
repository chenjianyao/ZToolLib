#include <stdlib.h>
#include <string.h>

#include "ztl_dyso.h"

#ifdef _MSC_VER

#include <Windows.h>
typedef HMODULE ztl_hlib_t;

#else

#include <errno.h>
#include <dlfcn.h>
typedef void* ztl_hlib_t;

#endif//_MSC_VER

/// exported types
struct ztl_dso_handle_st
{
    char path[256];
    ztl_hlib_t hlib;
};

#ifdef _MSC_VER
static ztl_hlib_t load_lib(const char* path)
{
    return LoadLibraryA(path);
}
static void unload_lib(ztl_hlib_t hlib)
{
    FreeLibrary(hlib);
}
static void* symbol_lib(ztl_hlib_t hlib, const char* symname)
{
    return GetProcAddress(hlib, symname);
}
#else// linux platform

static ztl_hlib_t load_lib(const char* path)
{
    return dlopen(path, RTLD_LAZY | RTLD_LOCAL);
}
static void unload_lib(ztl_hlib_t hlib)
{
    dlclose(hlib);
}
static void* symbol_lib(ztl_hlib_t hlib, const char* symname)
{
    return dlsym(hlib, symname);
}
#endif//_MSC_VER


ztl_dso_handle_t* ztl_dso_load(const char* libpath)
{
    ztl_dso_handle_t* dso = (ztl_dso_handle_t*)malloc(sizeof(ztl_dso_handle_t));
    if (dso != NULL)
    {
        dso->hlib = load_lib(libpath);
        strncpy(dso->path, libpath, sizeof(dso->path) - 1);
    }
    return dso;
}

void ztl_dso_unload(ztl_dso_handle_t* dso)
{
    if (dso)
    {
        if (dso->hlib)
            unload_lib(dso->hlib);
        free(dso);
    }
}

void* ztl_dso_symbol(ztl_dso_handle_t* dso, const char* symname)
{
    if (dso != NULL && dso->hlib != NULL)
    {
        return symbol_lib(dso->hlib, symname);
    }
    return NULL;
}

int ztl_dso_error(ztl_dso_handle_t* dso, char* buf, int bufsize)
{
	if (dso)
	{
#if defined(WINDOWS) || defined(WIN32)
        LPVOID lpMsgBuf;
        DWORD dw = GetLastError();
        FormatMessageA(
            FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
            NULL,
            dw,
            MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
            (LPSTR)&lpMsgBuf,
            0, NULL);
        strncpy(buf, lpMsgBuf, bufsize - 1);
        return dw;
#else
        extern int errno;
        int ret = errno;
        strncpy(buf, dlerror(), bufsize);
        return ret;
#endif//_WIN32
    }
    return -1;
}

