// -*- c++ -*-
// $Id: ToolsConfig.h,v 1.8 2012/07/04 09:08:47 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// kasemir@lanl.gov
//
// Basic configuration for the "Tools" in this directory,
// also included by ChannelArchiver sources.
//
// When I could not see a neat way to hide differences
// in operating systems or compilers,
// hacks based on #ifdef are used for these symbols:
// (very unclear what's defined. Try to use -D$(HOST_ARCH))
// WIN32, Linux, solaris, HP_UX

// Namespaces:
// After great problems with doing so initially,
// these sources try to avoid namespaces.
// But they use some of the STL/standard C++ library features.
// In principle, these should be found in the std:: namespace.
//
// Visual C++ behaves that way, requiring std:: namespace usage.
// GNU g++ does not require std::, even ignores the std:: namespace,
// for GNU it does not hurt to include the std::
// If your compiler prohibits the use of std::, you will
// get compilation errors.
// --------------------------------------------------------

// Conversions
//
// Little-Endian Architectures require conversions
// because the binary archive stores all data
// in the big-endian (aka network or Motorola) format.
// CONVERSION_REQUIRED is required on:
// Intel PCs (WIN32, Linux, OpenBSD, ...)
// It is usually not required on Sun Solaris machines.
//
// Note: The archiver assumes IEEE Floats
//       It won't work on VAX/VMS systems!
#define CONVERSION_REQUIRED 

// This seems to work for Mac OS X
#if defined(__POWERPC__) && defined(__APPLE__)
#undef CONVERSION_REQUIRED
#endif

#if defined(HP_UX)
#undef CONVERSION_REQUIRED
#endif

// C++ Tweaks
// ----------------------------------------------------------

// Support for standard C++ library
#include <stdString.h>

// On RedHat9 and R3.14.4, there's a conflict
// between /usr/include/assert.h and epicsAssert.h.
// We don't use any assert, to remove it:
#undef assert

// std::list or look-a-like:
#define stdList std::list
#include <list>

// std::vector or look-a-like:
#define stdVector std::vector
#include <vector>

// std::map or look-a-like:
#define stdMap std::map
#include <map>

// Is socklen_t defined?
// On e.g. RedHat7.0, the socket calls use socklen_t,
// while older systems don't have it.
// Win32 does not define socklen_t.
// At least Solaris8 and HP-UX11 also have socklen_t (T. Birke 10-19-2001)
#ifdef WIN32
typedef int socklen_t;
#endif

// If defined, the base class of all exceptions will
// be std::exception.
// Otherwise, we start with GenericException
#define USE_STD_EXCEPTION

// We use the GCC __attribute__((format....)
// to have the compiler check printf-type
// format strings,
// and to mark variables as unused:
//    __attribute__ ((unused))
// Define this one to disable when your compiler
// fails to understand __attribute__
#ifndef __GNUC__
#define __attribute__(x) /* */
#endif
 
