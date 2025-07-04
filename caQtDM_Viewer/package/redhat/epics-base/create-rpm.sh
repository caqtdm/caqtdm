#!/bin/bash
# This script creates an RPM package for epics-base

# Check if dependencies are installed and install them if not
# BuildRequires:  gcc
# BuildRequires:  make
# BuildRequires:  patch
# BuildRequires:  perl-core
# BuildRequires:  readline-devel
# BuildRequires:  ncurses-devel

# Requires:       readline
# Requires:       ncurses
# Requires:       perl-core

for pkg in gcc make patch perl readline-devel ncurses-devel wget2 rpmdevtools gnupg2; do
    if rpm -q $pkg > /dev/null 2>&1; then
        echo "$pkg is installed."
    else
        echo "Installing $pkg..."
        sudo dnf install -y $pkg
    fi
done

CURRENT_DIR=$(pwd)

# Remove any existing source tarball and signature
rm -f base-7.0.9* || true

# Create the RPM build environment
rpmdev-setuptree

# Get the source
wget https://epics-controls.org/download/base/base-7.0.9.tar.gz

# Copy the source tarball and signature to the SOURCES directory
cp base-7.0.9* ~/rpmbuild/SOURCES/

cp *.patch ~/rpmbuild/SOURCES/

# Copy the spec file to the SPECS directory
cp epics-base.spec ~/rpmbuild/SPECS/
# Change to the SPECS directory
cd ~/rpmbuild/SPECS

# Build the RPM package
rpmbuild -ba epics-base.spec

echo "Script completed successfully!"
