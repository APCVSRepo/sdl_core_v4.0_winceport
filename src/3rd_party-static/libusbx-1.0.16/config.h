/* config.h.  Generated from config.h.in by configure.  */
/* config.h.in.  Generated from configure.ac by autoheader.  */

/* Default visibility */
#if defined(OS_WIN32) || defined(OS_WINCE)
#define DEFAULT_VISIBILITY
#else
#define DEFAULT_VISIBILITY __attribute__((visibility("default")))
#endif
/* Start with debug message logging enabled */
/* #undef ENABLE_DEBUG_LOGGING */

/* Message logging */
//#define ENABLE_LOGGING 1

/* Define to 1 if you have the <dlfcn.h> header file. */
#define HAVE_DLFCN_H 1

/* Define to 1 if you have the `gettimeofday' function. */
#define HAVE_GETTIMEOFDAY 1

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the `udev' library (-ludev). */
#define HAVE_LIBUDEV 1

/* Define to 1 if you have the <libudev.h> header file. */
#define HAVE_LIBUDEV_H 1

/* Define to 1 if you have the <linux/filter.h> header file. */
/* #undef HAVE_LINUX_FILTER_H */

/* Define to 1 if you have the <linux/netlink.h> header file. */
/* #undef HAVE_LINUX_NETLINK_H */

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <poll.h> header file. */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define HAVE_POLL_H 1
#endif

/* Define to 1 if you have the <signal.h> header file. */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define HAVE_SIGNAL_H 1
#endif

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if the system has the type `struct timespec'. */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define HAVE_STRUCT_TIMESPEC 1
#endif

/* Define to 1 if you have the <sys/stat.h> header file. */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define HAVE_SYS_STAT_H 1
#endif

/* Define to 1 if you have the <sys/time.h> header file. */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define HAVE_SYS_TIME_H 1
#endif

/* Define to 1 if you have the <sys/types.h> header file. */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define HAVE_SYS_TYPES_H 1
#endif

/* Define to 1 if you have the <unistd.h> header file. */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define HAVE_UNISTD_H 1
#endif

#if defined(OS_WINCE)
#define HAVE_MISSING_H
#endif

/* Define to the sub-directory in which libtool stores uninstalled libraries.
   */
#define LT_OBJDIR ".libs/"

/* Define to 1 if your C compiler doesn't accept -c and -o together. */
/* #undef NO_MINUS_C_MINUS_O */

/* Darwin backend */
/* #undef OS_DARWIN */

/* Linux backend */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define OS_LINUX 1
#endif

/* OpenBSD/NetBSD backend */
/* #undef OS_OPENBSD */

/* Windows backend */
/* #undef OS_WIN32 */

/* Name of package */
#define PACKAGE "libusbx"

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT "libusbx-devel@lists.sourceforge.net"

/* Define to the full name of this package. */
#define PACKAGE_NAME "libusbx"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING "libusbx 1.0.16"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME "libusbx"

/* Define to the home page for this package. */
#define PACKAGE_URL "http://libusbx.org"

/* Define to the version of this package. */
#define PACKAGE_VERSION "1.0.16"

/* type of second poll() argument */
#if defined(OS_WIN32) || defined(OS_WINCE)
#define POLL_NFDS_TYPE unsigned int
#else
#define POLL_NFDS_TYPE nfds_t
#endif

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Use POSIX Threads */
#if !defined(OS_WIN32) && !defined(OS_WINCE)
#define THREADS_POSIX 1
#endif

/* timerfd headers available */
#if defined(OS_LINUX) && !defined(OS_ANDROID)
#define USBI_TIMERFD_AVAILABLE 1
#endif

/* Use udev for device enumeration/hotplug */
#if defined(OS_LINUX) && !defined(OS_ANDROID)
#define USE_UDEV 1
#endif

/* Version number of package */
#define VERSION "1.0.16"

/* Use GNU extensions */
#define _GNU_SOURCE 1

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif
