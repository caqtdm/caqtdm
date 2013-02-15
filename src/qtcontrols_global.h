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

#ifndef QTCONGLOBAL_H
#define QTCONGLOBAL_H

#if defined(_MSC_VER)
		#if defined(QTCON_MAKEDLL)     // create a qtControls DLL library
			#define QTCON_EXPORT  __declspec(dllexport)
		#else                        // use a qtControls DLL library
			#define QTCON_EXPORT  __declspec(dllimport)
		#endif
#else
	#define QTCON_EXPORT
#endif


#endif //QTCONGLOBAL_H
