#**************************************************************************
# Copyright (C) 2004 by
# Paul Scherrer Institut
# All rights reserved
#
#
# Project:      ACS
#
# Module:       
#
# Filename:     Makefile
#
# Author:       Gasche Martin
#     
# Description:  (LINUX makefile file)
#
# History:
# Date	      Dep	Name	Description
# -------------------------------------------------------------------------
# 2005-08-04  PSI/ACS	MGa	Initial version CVS directory layout
#
# @(#)$Id: Makefile,v 1.1 2005/09/15 11:46:50 gasche Exp $
#**************************************************************************

ifndef ACS_BUILD_INCLUDE
    $(error shell variable ACS_BUILD_INCLUDE is not defined!)
endif

ifndef ACS_BUILD_LIBRARY
    $(error shell variable ACS_BUILD_LIBRARY is not defined!)
endif

ifndef ACS_BUILD_BINARY
    $(error shell variable ACS_BUILD_BINARY is not defined!)
endif

#
# list of modules (directory name)
#
MODULES = DeviceRPC \
	  GraEdit \
	  sga \
	  AccStat \
	  Analog \
	  Analyze \
	  Disco \
	  Geiss \
	  HFdisp \
	  Intlock \
	  IQreset \
	  LogWWW \
	  Malus \
	  MEDM \
	  myServer \
	  Oraprogs \
	  pc104 \
	  Pfeil \
	  pidControl \
	  pidStrom \
	  Scan \
	  scl \
	  Spread \
	  StripTool \
	  Szent \
	  Tcl \
	  Tendis \
	  Touchp \
	  Transprofs \
	  Verdis

MAKE = make

all clean test install install_include install_lib install_bin install_data:
	for subdir in $(MODULES); do $(MAKE) -C $$subdir $@; done

buildandinstall:
	for subdir in $(MODULES); do $(MAKE) -C $$subdir && $(MAKE) -C $$subdir install ; done


																												


