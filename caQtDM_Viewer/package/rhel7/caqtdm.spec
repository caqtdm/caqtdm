# trim changelog included in binary rpms
%global _changelog_trimtime %(date +%s -d "1 year ago")

#############################################################################
# special EPICS things
%define EPICS_TARGET_VERSION -7.0.6
#############################################################################

# build qt4 support (or not)
%if 0%{?rhel} <  8
%global qt4 1
%endif
# build qt5 support (or not)
%global qt5 1
#############################################################################
Name:    caQtDM
Summary: Qt Widgets for Technical Applications
Version: 4.4.1
Release: 1%{?dist}
#############################################################################
License: GPLv2
URL:     https://github.com/caqtdm/caqtdm
Source:  https://github.com/caqtdm/caqtdm/%{name}/%{name}-%{version}.tar.gz


%if 0%{?qt5}
# Requires: caqtdm_archiver
BuildRequires: pkgconfig(Qt5Concurrent) pkgconfig(Qt5PrintSupport) pkgconfig(Qt5Widgets)
BuildRequires: pkgconfig(Qt5OpenGL) pkgconfig(Qt5Svg)
BuildRequires: pkgconfig(Qt5Designer)
%endif
%if 0%{?qt4}
BuildRequires: pkgconfig(QtGui) pkgconfig(QtSvg)
BuildRequires: pkgconfig(QtDesigner)
%{?_qt4_version:Requires: qt4%{?_isa} >= %{_qt4_version}}
%endif

BuildRequires: epics-base%{EPICS_TARGET_VERSION}-devel
Requires: epics-base%{EPICS_TARGET_VERSION}-devel-static
#####################################
#EPICS Libs
BuildRequires: epics-base-7.0.6-devel
Requires: epics-base-7.0.6
#####################################
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
Requires: epics-base%{EPICS_TARGET_VERSION}
%description bin-qt4
%define qt_vers qt4

%package qt4
Summary: Make %{name}-bin-qt4 default
Requires: %{name}-bin-qt4 = %{version}-%{release} 
Requires: %{name}-doc = %{version}-%{release}
Conflicts: %{name}-qt5
Provides: %{name}%{?_isa}
%description qt4
%define qt_vers qt4
%{summary}


%endif



%if 0%{?qt5}
%package qt5
Summary: Qt5 Widgets for Technical Applications
Provides: caqtdm-qt5 = %{version}-%{release}
Provides: caqtdm-qt5%{_isa} = %{version}-%{release}
Provides: %{name}%{?_isa}
Conflicts: %{name}-qt4
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
%if 0%{?rhel} <  8
Requires: python==3.9
%endif
Requires: epics-base%{EPICS_TARGET_VERSION}

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
export CAQTDM_MODBUS=1
export CAQTDM_GPS=1
export CAQTDM_COLLECT=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt5
export QTDM_RPATH=/opt/caqtdm/lib/qt5
export QTCONTROLS_LIBS=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt5
export QTBASE=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt5
export QTHOME=/usr
export QWTHOME=/usr
export QWTLIB=/usr/lib
export QWTINCLUDE=/usr/include/qt5/qwt
export QWTVERSION=6.1
export QWTLIBNAME=qwt-qt5
export EPICS_BASE=/usr/local/epics/base%{EPICS_TARGET_VERSION}
export EPICSINCLUDE=${EPICS_BASE}/include
export EPICSLIB=${EPICS_BASE}/lib/$EPICS_HOST_ARCH
%if 0%{?rhel} >  7
export PYTHONVERSION=3.9
%else
export PYTHONVERSION=2.7
%endif
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
export QTDM_RPATH=/opt/caqtdm/lib/qt4
export QTCONTROLS_LIBS=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt4
export QTBASE=%{_builddir}/%{name}-%{version}/build/opt/caqtdm/lib/qt4
export QTHOME=/usr
export QWTHOME=/usr
export QWTLIB=/usr/lib
export QWTINCLUDE=/usr/include/qwt
export QWTVERSION=6.1
export QWTLIBNAME=qwt
export EPICS_BASE=/usr/local/epics/base%{EPICS_TARGET_VERSION}
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
	
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caApplyNumeric   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caBitnames   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caByte   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caCalc   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caCamera   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caCartesianPlot   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caChoice   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caFrame   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caGauge   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caGraphics   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caImage   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caLabel   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caLed   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caLineEdit   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caMenu   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caMessageButton   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caMimeDisplay   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caMultiLineString   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caNumeric   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caSlider   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caSpinbox   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caStripPlot   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caTextEntry   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caThermo   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caToggleButton   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/caWaterfallPlot   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/EApplyButton   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/EArrow   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/EFlag   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/EGauge   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/ELabel   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/ESimpleLabel   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/EWidget   %{buildroot}/usr/local/include/caqtdm
	cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/src/QtControls   %{buildroot}/usr/local/include/caqtdm
	
        cp %{_builddir}/%{name}-%{version}/caQtDM_QtControls/plugins/*.h  %{buildroot}/usr/local/include/caqtdm/plugins

        cp %{_builddir}/%{name}-%{version}/caQtDM_Lib/src/*.h     %{buildroot}/usr/local/include/caqtdm
        
	cp %{_builddir}/%{name}-%{version}/caQtDM_Lib/caQtDM_Plugins/*.h     %{buildroot}/usr/local/include/caqtdm/caQtDM_Plugins
	
        cp %{_builddir}/%{name}-%{version}/caQtDM_Viewer/src/*.h     %{buildroot}/usr/local/include/caqtdm
	
	cp -R %{_builddir}/%{name}-%{version}/build/* %{buildroot}

	echo "echo \"CAQTDM_DISPLAY_PATH=\$CAQTDM_DISPLAY_PATH\"" >>  %{buildroot}/usr/local/bin/caqtdm
	echo " " >>  %{buildroot}/usr/local/bin/caqtdm
        echo "if [ -n \"\$SSH_CLIENT\" ]; then" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "  echo \"start from remote session\"" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "  caQtDM -style plastique -graphicssystem native \"\$@\" &" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "else" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "  caQtDM -style plastique \"\$@\" &" >>  %{buildroot}/usr/local/bin/caqtdm
        echo "fi" >>  %{buildroot}/usr/local/bin/caqtdm
        echo " " >>  %{buildroot}/usr/local/bin/caqtdm

        echo "#!/bin/bash" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "SOURCE=\"\${BASH_SOURCE[0]}\"" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "while [ -h \"\$SOURCE\" ]; do # resolve $SOURCE until the file is no longer a symlink" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "  DIR=\"\$( cd -P \"\$( dirname \"\$SOURCE\" )\" && pwd )\"" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "  SOURCE=\"\$(readlink \"\$SOURCE\")\"" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "  [[ \$SOURCE != /* ]] && SOURCE=\"\$DIR/\$SOURCE\" # if $SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "done" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "DIR=\"\$( cd -P \"\$( dirname \"\$SOURCE\" )\" && pwd )\"" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "CAQTDM_HOME=\$DIR/../.." >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "# Register help" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "assistant-qt4 -register \$CAQTDM_HOME/doc/caQtDM.qch" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "export QT_PLUGIN_PATH=\$CAQTDM_HOME/lib/qt4" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer
        echo "designer-qt4 \$@" >>  %{buildroot}/opt/caqtdm/lib/qt4/caqtdm_designer

        echo "#!/bin/bash" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "SOURCE=\"\${BASH_SOURCE[0]}\"" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "while [ -h \"\$SOURCE\" ]; do # resolve \$SOURCE until the file is no longer a symlink" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "  DIR=\"\$( cd -P \"\$( dirname \"\$SOURCE\" )\" && pwd )\"" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "  SOURCE=\"$(readlink \"\$SOURCE\")\"" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "  [[ \$SOURCE != /* ]] && SOURCE=\"\$DIR/\$SOURCE\" # if \$SOURCE was a relative symlink, we need to resolve it relative to the path where the symlink file was located" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "done" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "DIR=\"$( cd -P \"$( dirname \"\$SOURCE\" )\" && pwd )\"" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "CAQTDM_HOME=\$DIR/../.." >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "# Register help" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "assistant-qt5 -register $CAQTDM_HOME/doc/caQtDM.qch" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "export QT_PLUGIN_PATH=\$CAQTDM_HOME/lib/qt5" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer
        echo "designer-qt5 \$@" >>  %{buildroot}/opt/caqtdm/lib/qt5/caqtdm_designer



%files doc
/opt/caqtdm/doc

%files devel
/usr/local/include/caqtdm
%defattr(644, root, root, -)

%if 0%{?qt4}
%files bin-qt4
/opt/caqtdm/lib/qt4
%defattr(755,root,root)
/opt/caqtdm/lib/qt4/caqtdm_designer

%files qt4
/usr/local/bin
%defattr(755,root,root)
/usr/local/bin/caqtdm


%post qt4



ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt4/adl2ui
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt4/edl2ui
#ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt4/caQtDM
ln -sfv /opt/caqtdm/lib/qt4/caQtDM /usr/local/bin/caQtDM
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt4/caqtdm_designer
echo "check update"
if [ "$1" = "2" ] ; then # upgrade
        echo "running update"
        %{__rm} -rf /opt/caqtdm/bin/*
fi


%preun qt4

%endif

%files bin-qt5
/opt/caqtdm/lib/qt5

%files qt5
/usr/local/bin
%defattr(755,root,root)
/usr/local/bin/caqtdm
%defattr(755,root,root)
/opt/caqtdm/lib/qt5/caqtdm_designer

%post qt5
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt5/adl2ui
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt5/edl2ui
ln -t /usr/local/bin -sfv /opt/caqtdm/lib/qt5/caqtdm_designer
ln -sfv /opt/caqtdm/lib/qt5/caQtDM /usr/local/bin/caQtDM

%preun qt5

%postun doc
if [ "$1" = "0" ] ; then # last uninstall
        if [ -z "$(ls -A /opt/caqtdm)" ]; then
           %{__rm} -rf /opt/caqtdm
        fi
fi

%if 0%{?qt4}
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
%postun bin-qt4
if [ "$1" = "0" ] ; then # last uninstall
        if [ -z "$(ls -A /opt/caqtdm/lib)" ]; then
           %{__rm} -rf /opt/caqtdm/lib
        fi
        if [ -z "$(ls -A /opt/caqtdm)" ]; then
           %{__rm} -rf /opt/caqtdm
        fi
fi

%endif

%postun qt5
if [ "$1" = "0" ] ; then # last uninstall
        %{__rm} -rf /opt/caqtdm/bin

        %{__rm} -f  /usr/local/bin/adl2ui
        %{__rm} -f  /usr/local/bin/edl2ui
        %{__rm} -f  /usr/local/bin/caqtdm
        %{__rm} -f  /usr/local/bin/caQtDM
        %{__rm} -f  /usr/local/bin/caqtdm_designer

        if [ -z "$(ls -A /opt/caqtdm)" ]; then
           %{__rm} -rf /opt/caqtdm
        fi
fi
if [ "$1" = "1" ] ; then # last uninstall
        %{__rm} -rf /opt/caqtdm/bin
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

