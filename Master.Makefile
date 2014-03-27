BUILD_DATE =  '"$(shell date +"%d-%m-%Y")"'
column:= :
uscore:= _
empty:=
space:= $(empty) $(empty)
BUILD_DATE:= $(subst $(column),$(uscore),$(BUILD_DATE))

MY_INCLUDES = $(HOME)/workarea/ACS/mezger/Include
ACS_BUILD_LIBRARY= /home/ACS/Control/Lib

caSubs = $(HOME)/workarea/ACS/mezger/epicsLib

EPICSBASE = $(EPICS)

CC = gcc -DBUILD=$(BUILD_DATE) -DUDP -I$(MY_INCLUDES) -I$(caSubs) -I/usr/include/libxml2 -I$(EPICSINCLUDE) -I$(EPICSINCLUDE)/os/Linux -g  -Wall -D_NO_PROTO -DINTRINSICS -DMOTIF -D_REENTRANT
CC_E = gcc -DBUILD=$(BUILD_DATE) -DUDP -I$(MY_INCLUDES) -I$(caSubs) -I/usr/include/libxml2 -I$(EPICSINCLUDE) -I$(EPICSINCLUDE)/os/Linux -g  -Wall -D_NO_PROTO -DINTRINSICS -DMOTIF -D_REENTRANT -DEPICS

FF = ifort -vms -132 -WB -g -save -traceback -assume nounderscore -I$(MY_INCLUDES) -DEPICS

x11 = /usr/X11R6/lib

# acs  control system
ACS_LIBS = -L$(ACS_BUILD_LIBRARY) -lDEV -lCDB -lInclude 
PROF_LIBS = -L$(ACS_BUILD_LIBRARY) -lProfAcc

# x11 and motif 
MYX_LIBS = -L$(ACS_BUILD_LIBRARY) -lIncludeX -lXC -lXbae
MOTIF_LIBS = -L$(x11) -lMrm -lX11  -lXm -lXt -lXp -lXext -lSM -lICE -lXpm
FERMI_LIBS = $(ACS_BUILD_LIBRARY)/libfermiWidgets.a  $(ACS_BUILD_LIBRARY)/libfermiUtils.a
IMAGE_LIB = $(ACS_BUILD_LIBRARY)/libImage.a
XMA_LIBS = $(ACS_BUILD_LIBRARY)/XmATerm.a
GRA_LIBS = $(GRAPHX)/lib/libgralb.a

SYS_LIBS = -L/lib/ -lpthread -lm


# xml libs
XML_LIBS = -L/lib/ -lstdc++ -lxml2

# epics libs
EPICS_LIBS = -L$${EPICSLIB} -Wl,-rpath,$${EPICSLIB} -lca
CASUBS_LIBS = -L$(ACS_BUILD_LIBRARY) -lCAsubs

# epics support through acs routines
EMUL_LIBS = -L$(ACS_BUILD_LIBRARY) -lCAsubs -lAcsEmul -lIncludeNew

# utilities
INCLUDE_LIBS = -L$(ACS_BUILD_LIBRARY) -lIncludeNew 

#oracle device sarch lib
DEVMULT_LIBS =  $(HOME)/workarea/ACS/mezger/epicsLib/libCAmult.a
ORACLE_LIBS   = -L $(ORACLE_LIB) -lclntsh
ORACLEEXT_LIB = $(OCILIB)/libocilib.a

# fortran libraries and acs support
FORTRAN_LIBS = -L/opt/intel/parallel/lib/ia32 -lifcore -lifport -lirc -limf
FORWRAP_LIBS = -L$(ACS_BUILD_LIBRARY) -lforDEV

# gnu scientific library
GSL_LIBS = -L/lib/ -lgsl -lgslcblas

# easymotif
EasyMotif = $(HOME)/workarea/ACS/library/EasyMotif

.f.o:
	$(FF) -c $<