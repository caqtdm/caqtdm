//******************************************************************************
// Copyright (c) 2012 Paul Scherrer Institut PSI), Villigen, Switzerland
// Disclaimer: neither  PSI, nor any of their employees makes any warranty
// or assumes any legal liability or responsibility for the use of this software
//******************************************************************************
//******************************************************************************
//
//     Author : Anton Chr. Mezger
//
//******************************************************************************

#ifndef CAQTDM_LIB_GLOBAL_H
#define CAQTDM_LIB_GLOBAL_H

#include <QtCore/qglobal.h>

#if defined(CAQTDM_LIB_LIBRARY)
#  define CAQTDM_LIBSHARED_EXPORT Q_DECL_EXPORT
#else
#  define CAQTDM_LIBSHARED_EXPORT Q_DECL_IMPORT
#endif

#endif // CAQTDM_LIB_GLOBAL_H
