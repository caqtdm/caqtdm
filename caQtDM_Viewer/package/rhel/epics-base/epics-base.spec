# SPDX-License-Identifier: MIT
Name:           epics-base-7.0.9
Version:        1.0.0
Release:        1%{?dist}
Summary:        Experimental Physics and Industrial Control System base
License:        EPICS Open License
URL:            https://epics-controls.org
# EPICS base is not strictly 'noarch' as it contains architecture-specific
# binaries and libraries built by the EPICS build system.
# BuildArch: noarch

Source0:        https://epics-controls.org/download/base/base-7.0.9.tar.gz
# Source1 is the GPG signature, handled by build system infrastructure if configured.
Source1:        https://epics-controls.org/download/base/base-7.0.9.tar.gz.asc
Patch0:         01_install_permissions.patch
Patch1:         02_no_rpath.patch
Patch2:         03_new_gcc_version_fix.patch
# Source6 is the LICENSE file installed
# from the extracted source tree. We reference it here explicitly for clarity.
Source6:        LICENSE

BuildRequires:  gcc
BuildRequires:  make
BuildRequires:  patch
BuildRequires:  perl
BuildRequires:  readline-devel
BuildRequires:  ncurses-devel

Requires:       readline
Requires:       ncurses
Requires:       perl

Provides:       epics-base-7.0.9 = %{version}-%{release}

%define EPICS_BASE /usr/local/epics/base-7.0.9

%description
The Experimental Physics and Industrial Control System (EPICS) is a set of
software tools and distributed databases used to create distributed control
systems for scientific instruments such as particle accelerators, telescopes,
and other large-scale experiments.

This package provides the base libraries and tools for EPICS.

%package devel
Summary:        Development files for EPICS base
Group:          Development/Tools
Requires:       %{name} = %{version}-%{release}
# The devel package is not strictly 'noarch' as it contains architecture-specific
# binaries and libraries built by the EPICS build system.
# BuildArch: noarch
Provides:      %{name}-devel = %{version}-%{release}
%description devel
The development files for the Experimental Physics and Industrial Control
System (EPICS) base package. This includes header files and libraries needed
to develop applications that use EPICS.
This package is intended for developers who want to build applications
that use EPICS.

%prep
# Setup macro extracts Source0 into a directory named 'base-7.0.9'
%setup -q -n base-7.0.9

# Apply patches
%patch 0 -p1
%patch 1 -p1
%patch 2 -p1

# Create the staging directory *within* the build directory
# The EPICS build system installs here during the `%build` phase.
mkdir -p %{_builddir}/base-7.0.9/staging%{EPICS_BASE}

# Configure INSTALL_LOCATION to point to this staging area
echo "INSTALL_LOCATION=%{_builddir}/base-7.0.9/staging%{EPICS_BASE}" > configure/CONFIG_SITE.local
# Configure FINAL_LOCATION to the final install path in the RPM
echo "FINAL_LOCATION=%{EPICS_BASE}" >> configure/CONFIG_SITE.local

# Get the EPICS_HOST_ARCH and save it to a file in the build directory
perl -CSD src/tools/EpicsHostArch.pl > %{_builddir}/EPICS_HOST_ARCH

%build
# Load the saved EPICS_HOST_ARCH into an environment variable for the build
export EPICS_HOST_ARCH=$(cat %{_builddir}/EPICS_HOST_ARCH)

# Export the library path for the linker at build time, pointing to the staging area
export LD_LIBRARY_PATH="%{_builddir}/base-7.0.9/staging%{EPICS_BASE}/lib/${EPICS_HOST_ARCH}"

# export PERL5LIB="%{_builddir}/base-7.0.9/staging%{_libdir}/epics/lib/perl"

# Build and install to the staging area defined in CONFIG_SITE.local
make -s -j8

%install
# Set buildroot
# The %setup macro changed directory to %{_builddir}/base-7.0.9
# We need to change to the build-time staging directory for installation
cd %{_builddir}/base-7.0.9/staging%{EPICS_BASE}

# Get the EPICS_HOST_ARCH again for installation
EPICS_HOST_ARCH=$(cat %{_builddir}/EPICS_HOST_ARCH)

# Define the final EPICS_BASE location within the file system for symlinks
EPICS_BASE="%{EPICS_BASE}"
EPICS_STAGING=%{_builddir}/base-7.0.9/staging${EPICS_BASE}

EPICS_INSTALL=%{buildroot}${EPICS_BASE}
# Create the final install directory
mkdir -p ${EPICS_INSTALL}

EPICS_INCLUDE="${EPICS_INSTALL}/include"
EPICS_LICENSES=%{buildroot}/usr/share/licenses/epics-base-7.0.9
EPICS_DOC=%{buildroot}/usr/share/doc/epics-base-7.0.9
SYS_BIN=%{buildroot}/usr/bin
SYS_LIB=%{buildroot}/usr/%{_lib}
SYS_INCLUDE=%{buildroot}/usr/include
SHARE_EPICS=%{buildroot}/usr/share/epics-base-7.0.9

mkdir -p ${EPICS_INCLUDE}
mkdir -p ${EPICS_LICENSES}
mkdir -p ${EPICS_DOC}
mkdir -p ${SYS_BIN}
mkdir -p ${SYS_LIB}
mkdir -p ${SYS_INCLUDE}

cp -a ${EPICS_STAGING}/include/* ${EPICS_INCLUDE}
cp -a ${EPICS_STAGING}/lib ${EPICS_INSTALL}
cp -a ${EPICS_STAGING}/bin ${EPICS_INSTALL}
cp -a ${EPICS_STAGING}/cfg ${EPICS_STAGING}/configure ${EPICS_STAGING}/db ${EPICS_STAGING}/dbd ${EPICS_STAGING}/templates ${EPICS_INSTALL}

# Patch makeRPath.py to have a python3 shebang
sed -i '1s|^.*$|#!/usr/bin/env python3|' ${EPICS_INSTALL}/bin/${EPICS_HOST_ARCH}/makeRPath.py

# Copy the license file to the appropriate location
cp -a %{_builddir}/base-7.0.9/LICENSE ${EPICS_LICENSES}/LICENSE
# Copy the license file into the source directory
cp -a %{_builddir}/base-7.0.9/LICENSE %{_builddir}/../../SOURCES/
# Copy the documentation to the appropriate location
cp -a ${EPICS_STAGING}/html ${EPICS_DOC}

# Create symlinks for binaries
for bin in caget caput cainfo camonitor caRepeater casw pvget pvinfo pvlist pvput softIoc softIocPVA; do \
    ln -s %{EPICS_BASE}/bin/${EPICS_HOST_ARCH}/${bin} ${SYS_BIN}/${bin}; \
done

# Create symlink for include files
ln -s %{EPICS_BASE}/include ${SYS_INCLUDE}/epics-base-7.0.9

# Create symlink for db dbd and templates in /usr/share/epics
mkdir -p ${SHARE_EPICS}
for dir in db dbd templates; do \
    ln -s %{EPICS_BASE}/${dir} ${SHARE_EPICS}/${dir}; \
done

# Create symlinks for all .so files in lib
for lib in ${EPICS_INSTALL}/lib/${EPICS_HOST_ARCH}/*.so*; do \
    libname=$(basename $lib); \
    ln -s ${EPICS_BASE}/lib/${EPICS_HOST_ARCH}/${libname} ${SYS_LIB}/${libname}; \
done

# Create symlink for the pkgconfig files
mkdir -p ${SYS_LIB}/pkgconfig
for pcfile in ${EPICS_INSTALL}/lib/pkgconfig/*.pc; do \
    pcname=$(basename $pcfile); \
    ln -s ${EPICS_BASE}/lib/pkgconfig/${pcname} ${SYS_LIB}/pkgconfig/${pcname}; \
done

# Create environment.d file
mkdir -p %{buildroot}/usr/lib/environment.d
cat << EOF > %{buildroot}/usr/lib/environment.d/10-epics-base.conf
# EPICS_BASE environment variables
EPICS_BASE=${EPICS_BASE}
EPICS_HOST_ARCH=${EPICS_HOST_ARCH}
EOF

# Create CONFIG_SITE.local in the build root
rm -f ${EPICS_INSTALL}/configure/CONFIG_SITE.local
cat << EOF > ${EPICS_INSTALL}/configure/CONFIG_SITE.local
EPICS_HOST_ARCH=${EPICS_HOST_ARCH}
INSTALL_LOCATION=.
FINAL_LOCATION=${EPICS_BASE}
EOF

%files
%license LICENSE
%doc %{EPICS_DOC}/html

# The core EPICS_BASE directory and contents (excluding devel stuff)
%dir %{EPICS_BASE}
%{EPICS_BASE}/*
# Exclude include directory from main package, it's for devel
%exclude %{EPICS_BASE}/include

# System symlinks (runtime only)
%{_bindir}/caget
%{_bindir}/caput
%{_bindir}/cainfo
%{_bindir}/camonitor
%{_bindir}/caRepeater
%{_bindir}/casw
%{_bindir}/pvget
%{_bindir}/pvinfo
%{_bindir}/pvlist
%{_bindir}/pvput
%{_bindir}/softIoc
%{_bindir}/softIocPVA

# Shared libraries (runtime only)
%{_libdir}/*.so*

# Share files
%dir %{_datadir}/epics-base-7.0.9
%{_datadir}/epics-base-7.0.9/db
%{_datadir}/epics-base-7.0.9/dbd
%{_datadir}/epics-base-7.0.9/templates

# Environment file
%{_prefix}/lib/environment.d/10-epics-base.conf

%files devel
# Header files and devel symlinks
%{EPICS_BASE}/include
%{_includedir}/epics-base-7.0.9
%{_libdir}/pkgconfig/*.pc

%changelog
* Thu May 22 2025 Julian Houba <julian.houba@psi.ch> - 7.0.9-1
- Convert from Arch Linux PKGBUILD.
- Replicate staging build and manual install/linking logic.
- Add environment.d file.
- Fix paths for Fedora FHS compatibility (%{_libdir}, %{_bindir}, etc).