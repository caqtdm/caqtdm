// -*- c++ -*-
// --------------------------------------------------------
// $Id: ArchiverConfig.h,v 1.3 2012/07/04 09:08:46 jud Exp $
//
// Please refer to NOTICE.txt,
// included as part of this distribution,
// for legal information.
//
// Kay-Uwe Kasemir, kasemir@lanl.gov
// --------------------------------------------------------

#ifndef _ARCHIVER_CONFIG_H__
#define _ARCHIVER_CONFIG_H__

/// \mainpage Channel Archiver
///
/// Documentation of the ChannelArchiver's libraries
/// and classes.

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

// Some even more generic things are configured in ToolsConfig.h!

// ----------------------------------------------------------
// ArchiveEngine
// ----------------------------------------------------------

// Show full path to engine config in HTTPD?
#undef SHOW_DIR

// Use password mechanism
// (for stopping the engine over the web)
#undef USE_PASSWD

#define DEFAULT_USER    "engine"
#define DEFAULT_PASS    "password"   

// Define if sigaction() is available in signal.h
//
// On Win32, signal() is good enough.
// On Unix, this does not work in a multithreaded program,
// so sigaction() should be used.
//
// On solaris I couldn't get it to compile, struct sigaction
// isn't there.
// This might be a feature of our local installation, though.
#define HAVE_SIGACTION
#ifdef WIN32
#undef HAVE_SIGACTION
#endif
#ifdef solaris
#undef HAVE_SIGACTION
#endif

// ----------------------------------------------------------
// ArchiveManager
// ----------------------------------------------------------

// Should ArchiveManager use "MultiArchive" or "BinArchive"?
// (For specifics like listing data headers, creating new file, ...
//  BinArchive is used in any case)
#define MANAGER_USE_MULTI

// ----------------------------------------------------------
// CGIExport
// ----------------------------------------------------------

// Current options: BinArchive, MultiArchive
#define CGIEXPORT_ARCHIVE_TYPE MultiArchive

// Disable the DEBUG command if you feel it's dangerous
// since it might invite hackers:
//#define NO_CGI_DEBUG

#if (!defined(GNUPLOT_PROGRAM) || !defined(GNUPLOT_PIPE)) && defined (GNUPLOT)
# define GNUPLOT_PROGRAM GNUPLOT
# define GNUPLOT_PIPE GNUPLOT

#else
// Where is GNU-Plot?
// Linux, default for Unix:
#define GNUPLOT_PROGRAM "/usr/bin/gnuplot"
#define GNUPLOT_PIPE    "/usr/bin/gnuplot"

// Default for Win32
// Note: you need the pgnuplot.exe, the one that
// can be run from a pipe.
// With some GNUPlot binaries this is included,
// otherwise you have to get pgnuplot.c and compile
// it yourself. See the GNUPlot web sites for more on this
#ifdef WIN32
#undef GNUPLOT_PROGRAM
#undef GNUPLOT_PIPE
#define GNUPLOT_PROGRAM "c:\\Progra~1\\Gnuplot3.7\\wgnupl32.exe"
#define GNUPLOT_PIPE    "c:\\Progra~1\\Gnuplot3.7\\pgnuplot.exe"
#endif

// LEDA @ LANL
#ifdef solaris
#undef GNUPLOT_PROGRAM
#undef GNUPLOT_PIPE
#define GNUPLOT_PROGRAM "/opt/apache/htdocs/archive/cgi/gnuplot"
#define GNUPLOT_PIPE    "/opt/apache/htdocs/archive/cgi/gnuplot"
#endif
#endif

// The CGI tool adds cgi_body_start.txt and ..end.txt
// to each web page that it generates.
// They are searched for in the directory where the
// start page resides.
//
// The WIN32/MS web server launches the cgi script
// where that web page was, so we are already there.
// Other Web servers launch this program in the cgi
// directory which could be under the archive interface
// like this directory:
//
//    ..../Default.htm        <- page that starts this program
//    ..../cgi/CGIExport.cgi  <- that's us now
//    ..../cgi/tmp/.....      <- where we will create our tmp files
// This define needs to be set such that the CGI tool
// gets from its starting point to the directory where the web files are:
#ifndef WIN32
#define WEB_DIRECTORY ".."
#endif

// Temporary Directories:
// The CGI tool creates data files & scripts for GNUPlot,
// then GNUPlot creates an image file.

// 1) Where are those files physically on the disk?
//    Is the path relative to where the web files are?
#define USE_RELATIVE_PHYSICAL_PATH
#ifdef WIN32
#define PHYSICAL_PATH "\\cgi\\tmp\\"
#else
#define PHYSICAL_PATH "/cgi/tmp/"
#endif

// 2) Where are those files for the web server,
//    i.e. what is the URL name to send to the web browser?
//    (no WIN32 specific '\' here)
#define USE_RELATIVE_URL
#define URL_PATH "/tmp/"

// ----------------------------------------------------------
// ArchiveExport
// ----------------------------------------------------------
#define EXPORT_ARCHIVE_TYPE MultiArchive

// ----------------------------------------------------------
// WinBrowser
// ----------------------------------------------------------
#define WINBROWSER_ARCHIVE_TYPE MultiArchive

#endif
