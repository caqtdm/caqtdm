Name:	caqtdm_archiver	
Version:	1.0
Release:	1%{?dist}
Summary:	caQtDM archiver support files

Group: caqtdm_archiver		
License: GPL	
URL:	https://git.psi.ch/gfa_apps/caqtdm_archiver.git	
Source0: caqtdm_archiver-1.0.tar.gz	


%description


%prep
%setup -q


%build
pushd caQtDM_Logging_Library
make %{?_smp_mflags}
popd
pushd caQtDM_ArchiveCA_Library
# build all necessary libraries for the channel access archive plugin
pushd xerces-c-3.1.4
./build.sh
popd

pushd Storage
make %{?_smp_mflags}
popd

pushd Tools
make %{?_smp_mflags}
popd




%install
mkdir -p %{buildroot}/opt/caqtdm-archiver/lib
mkdir -p %{buildroot}/opt/caqtdm-archiver/lib/Tools
mkdir -p %{buildroot}/opt/caqtdm-archiver/lib/Storage
mkdir -p %{buildroot}/opt/caqtdm-archiver/lib/xerces-c-3.1.4


mkdir -p %{buildroot}/usr/local/include

pushd caQtDM_Logging_Library
cp libNewLogRPC_64.a %{buildroot}/opt/caqtdm-archiver/lib
cp logtimes.h %{buildroot}/usr/local/include
popd
pushd caQtDM_ArchiveCA_Library
pushd xerces-c-3.1.4
cp ./libxerces-c_64.a %{buildroot}/opt/caqtdm-archiver/lib/xerces-c-3.1.4

popd

pushd Storage
cp ./libStorage_64.a %{buildroot}/opt/caqtdm-archiver/lib/Storage
cp *.h %{buildroot}/usr/local/include

popd

pushd Tools
cp ./libTools_64.a %{buildroot}/opt/caqtdm-archiver/lib/Tools
cp *.h %{buildroot}/usr/local/include

popd
popd

%files
%doc
/opt/caqtdm-archiver/lib
/usr/local/include

%changelog

