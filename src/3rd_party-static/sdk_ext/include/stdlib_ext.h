#ifndef __STDLIB_EXT_H
#define __STDLIB_EXT_H

char * __cdecl getenv(const char *name);
int    __cdecl _putenv(const char *);
int    __cdecl _wputenv(const wchar_t *);

#endif