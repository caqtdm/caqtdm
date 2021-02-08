# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

%global __provides_exclude_from /opt/caqtdm/lib/qt[45]
%global epicslibs              libCom
%global epicslibs %{epicslibs}|libca
%global epicslibs %{epicslibs}|libnt
%global epicslibs %{epicslibs}|libpvData
%global epicslibs %{epicslibs}|libpvAccess
%global epicslibs %{epicslibs}|libpvaClient
%global privlibs  libqtcontrols
%global privlibs  %{privlibs}|libedlParser
%global privlibs  %{privlibs}|libadlParser
%global privlibs  %{privlibs}|libcaQtDM_Lib
%global __requires_exclude ^(%{epicslibs}|%{privlibs})\\.so

# build qt4 support (or not)
%if 0%{?rhel} <  8
%global qt4 1
%endif
# build qt5 support (or not)
%global qt5 1

Name:    caQtDM
Summary: Qt Widgets for Technical Applications
Version: 4.3.0
Release: 3%{?dist}

License: GPLv2
URL:     https://github.com/caqtdm/caqtdm
Source:  https://github.com/caqtdm/caqtdm/%{name}/%{name}-%{version}.tar.gz


%if 0%{?qt5}
Requires: caqtdm_archiver
BuildRequires: pkgconfig(Qt5Concurrent) pkgconfig(Qt5PrintSupport) pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5OpenGL) pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Designer)
%endif
%if 0%{?qt4}
BuildRequires: pkgconfig(QtGui) pkgconfig(QtSvg)
BuildRequires: pkgconfig(QtDesigner)
%{?_qt4_version:Requires: qt4%{?_isa} >= %{_qt4_version}}
%endif


Provides: caqtdm = %{version}-%{release}
Provides: caqtdm%{_isa} = %{version}-%{release}

%description
caQtDM makes panel development faster and easier. It's made for folks of all skill levels, devices of all shapes, and projects of all sizes.


%package devel
Summary:  Development files for %{name}
Provides: caqtdm-devel = %{version}-%{release}
Provides: caqtdm-devel%{_isa} = %{version}-%{release}
Requires: %{name}%{?_isa} = %{version}-%{release}
%description devel
%{summary}.

%package doc
Summary: Developer documentation for %{name}
BuildArch: noarch
%description doc
%{summary}.

%if 0%{?qt4}
%package bin-qt4
Summary: caQtDM built against Qt 4.
Requires: qt-devel
Requires: qt-assistant
Requires: qwt zeromq
Obsoletes:  caqtdm
%description bin-qt4
%define qt_vers qt4

%package qt4
Summary: Make %{name}-bin-qt4 default
Requires: %{name}-bin-qt4 = %{version}-%{release} 
Requires: %{name}-doc = %{version}-%{release}
Conflicts: %{name}-qt5
Obsoletes:  caqtdm
%description qt4
%define qt_vers qt4
%{summary}


%endif



%if 0%{?qt5}
%package qt5
Summary: Qt5 Widgets for Technical Applications
Provides: caqtdm-qt5 = %{version}-%{release}
Provides: caqtdm-qt5%{_isa} = %{version}-%{release}
%description qt5
%{summary}.

%package bin-qt5
Summary: caQtDM built against Qt 5.
Requires: qt5-assistant
Requires: qt5-designer
Requires: qwt-qt5 zeromq
Obsoletes:  caqtdm
%description bin-qt5
%define qt_vers qt5




%endif


%prep

%setup -q

#%patch50 -p1 -b .pkgconfig
#%patch51 -p1 -b .qt_install_paths
#%patch52 -p1 -b .qt5
#%patch53 -p1 -b .no_rpath


%build
mkdir -p %{buildroot}/opt/caqtdm/lib
%if 0%{?qt5}
mkdir -p %{_target_platform}-qt5
pushd %{_target_platform}-qt5
mkdir -p %{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt5
export CAQTDM_COLLECT=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt5
export QTDM_RPATH=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt5
export QTCONTROLS_LIBS=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt5
export QTBASE=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt5
export QTHOME=/usr
export QWTHOME=/usr
export QWTLIB=/usr/lib
export QWTINCLUDE=/usr/include/qt5/qwt
export QWTVERSION=6.1
export QWTLIBNAME=qwt-qt5
export EPICS_BASE=/usr/local/epics/base
export EPICSINCLUDE=${EPICS_BASE}/include
export EPICSLIB=${EPICS_BASE}/lib/$EPICS_HOST_ARCH
export PYTHONVERSION=2.7
export PYTHONINCLUDE=/usr/include/python$PYTHONVERSION
export PYTHONLIB=/usr/lib/
export ZMQINC=/usr/include
export ZMQLIB=/usr/lib64

export QMAKESPEC=/usr/lib64/qt5/mkspecs/linux-g++-64

export CAQTDM_CA_ARCHIVELIBS=/opt/caqtdm-archiver/lib
export CAQTDM_LOGGING_ARCHIVELIBS=/opt/caqtdm-archiver/lib

%{?qmake_qt5}%{?!qmake_qt5:%{_qt5_qmake}} ../all.pro 

%make_build
%make_install
make clean
popd
%endif

%if 0%{?qt4}
mkdir -p %{_target_platform}-qt4
pushd %{_target_platform}-qt4
mkdir -p %{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt4
export CAQTDM_COLLECT=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt4
export QTDM_RPATH=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt4
export QTCONTROLS_LIBS=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt4
export QTBASE=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt4
export QTHOME=/usr
export QWTHOME=/usr
export QWTLIB=/usr/lib
export QWTINCLUDE=/usr/include/qwt
export QWTVERSION=6.1
export QWTLIBNAME=qwt
export EPICS_BASE=/usr/local/epics/base
export EPICSINCLUDE=${EPICS_BASE}/include
export EPICSLIB=${EPICS_BASE}/lib/$EPICS_HOST_ARCH
export PYTHONVERSION=2.7
export PYTHONINCLUDE=/usr/include/python$PYTHONVERSION
export PYTHONLIB=/usr/lib/
export ZMQINC=/usr/include
export ZMQLIB=/usr/lib64

export QMAKESPEC=/usr/lib64/qt4/mkspecs/linux-g++-64

export CAQTDM_CA_ARCHIVELIBS=/opt/caqtdm-archiver/lib
export CAQTDM_LOGGING_ARCHIVELIBS=/opt/caqtdm-archiver/lib

%{?qmake_qt4}%{?!qmake_qt4:%{_qt4_qmake}} ../all.pro

%make_build
%make_install
popd
%endif


%install
	mkdir -p %{buildroot}/opt/caqtdm/doc
	mkdir -p %{buildroot}/usr/local/bin
        mkdir -p %{buildroot}/usr
        mkdir -p %{buildroot}/usr/local
        mkdir -p %{buildroot}/usr/local/include
        mkdir -p %{buildroot}/usr/local/include/caqtdm
        mkdir -p %{buildroot}/usr/local/include/caqtdm/plugins
        mkdir -p %{buildroot}/usr/local/include/caqtdm/caQtDM_Plugins

        cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/*.h     %{buildroot}/usr/local/include/caqtdm
        cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/plugins/*.h  %{buildroot}/usr/local/include/caqtdm/plugins
        cp %{_builddir}/%{name}-%{version}/caQtDM_Lib/src/*.h     %{buildroot}/usr/local/include/caqtdm
        cp %{_builddir}/%{name}-%{version}/caQtDM_Lib/caQtDM_Plugins/*.h     %{buildroot}/usr/local/include/caqtdm/caQtDM_Plugins
        cp %{_builddir}/%{name}-%{version}/caQtDM_Viewer/src/*.h     %{buildroot}/usr/local/include/caqtdm
	
	cp -R %{_builddir}/%{name}-%{version}/build/* %{buildroot}

	echo "echo \"CAQTDM_DISPLAY_PATH=$CAQTDM_DISPLAY_PATH\"" >>  %{buildroot}/usr/local/bin/caqtdm
	echo " " >>  %{buildroot}/usr/local/bin/caqtdm
        echo "if [ -n \"$SSH_CLIENT\" ]; then" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "  echo \"start from remote session\"" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "  caQtDM -style plastique -graphicssystem native \"$@\" &" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "else" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "  caQtDM -style plastique \"$@\" &" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "fi" >>  %{buildroot}/usr/local/bin/caqtdm
        echo " " >>  %{buildroot}/usr/local/bin/caqtdm

%files doc
/opt/caqtdm/doc

%files devel
/usr/local/include/caqtdm
%defattr(644, root, root, -)


%files bin-qt4
/opt/caqtdm/lib/qt4

%files qt4
/usr/local/bin
%defattr(755,root,root)
/usr/local/bin/caqtdm


%post qt4

ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt4/adl2ui
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt4/edl2ui
ln -sfv /opt/caqtdm/lib/qt4/caqtdm /usr/local/bin/caQtDM
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt4/caqtdm_designer
echo "check update"
if [ "$1" = "2" ] ; then # upgrade
        echo "running update"
        %{__rm} -rf /opt/caqtdm/bin/*
fi


%preun qt4

%files bin-qt5
/opt/caqtdm/lib/qt5

%files qt5
/usr/local/bin

%post qt5
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt5/adl2ui
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt5/edl2ui
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt5/caqtdm
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt5/caqtdm_designer

%preun qt5

%postun doc
if [ "$1" = "0" ] ; then # last uninstall
        if [ -z "$(ls -A /opt/caqtdm)" ]; then
           %{__rm} -rf /opt/caqtdm
        fi
fi

%postun qt4
if [ "$1" = "0" ] ; then # last uninstall
        %{__rm} -rf /opt/caqtdm/bin
        %{__rm} -f  /usr/local/bin/adl2ui
        %{__rm} -f  /usr/local/bin/edl2ui
        %{__rm} -f  /usr/local/bin/caQtDM
        %{__rm} -f  /usr/local/bin/caqtdm_designer

        if [ -z "$(ls -A /opt/caqtdm)" ]; then
           %{__rm} -rf /opt/caqtdm
        fi
fi

%postun qt5
if [ "$1" = "0" ] ; then # last uninstall
        %{__rm} -rf /opt/caqtdm/bin

        %{__rm} -f  /usr/local/bin/adl2ui
        %{__rm} -f  /usr/local/bin/edl2ui
        %{__rm} -f  /usr/local/bin/caqtdm
        %{__rm} -f  /usr/local/bin/caqtdm_designer

        if [ -z "$(ls -A /opt/caqtdm)" ]; then
           %{__rm} -rf /opt/caqtdm
        fi
fi
if [ "$1" = "1" ] ; then # last uninstall
        %{__rm} -rf /opt/caqtdm/bin
fi

%postun bin-qt4
if [ "$1" = "0" ] ; then # last uninstall
        if [ -z "$(ls -A /opt/caqtdm/lib)" ]; then
           %{__rm} -rf /opt/caqtdm/lib
        fi
        if [ -z "$(ls -A /opt/caqtdm)" ]; then
           %{__rm} -rf /opt/caqtdm
        fi
fi

%postun bin-qt5
if [ "$1" = "0" ] ; then # last uninstall
        if [ -z "$(ls -A /opt/caqtdm/lib)" ]; then
           %{__rm} -rf /opt/caqtdm/lib
        fi
        if [ -z "$(ls -A /opt/caqtdm)" ]; then
           %{__rm} -rf /opt/caqtdm
        fi
fi

%changelog

