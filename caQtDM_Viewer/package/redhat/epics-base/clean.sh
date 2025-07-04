#!/bin/bash
echo "" 
echo "     caQtDM CleanScript"
echo ""

rm -rf base*

rm -rf ../../rpmbuild/SRPMS/epics*
rm -rf ../../rpmbuild/SOURCES/epics*
rm -rf ../../rpmbuild/RPMS/noarch/epics*
rm -rf ../../rpmbuild/RPMS/x86_64/epics*
rm -rf ../../rpmbuild/RPMS/aarch64/epics*
