#pragma once

#ifdef _WIN32
#include <windows.h>
#endif

#ifndef _WIN32
#include <sys/time.h>
unsigned long GetTickCount()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL) != 0) return 0;
    return (tv.tv_sec * 1000) + (tv.tv_usec / 1000);
}
#endif

