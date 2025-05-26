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

# Get the source
wget https://epics-controls.org/download/base/base-7.0.9.tar.gz
# Get signature
wget https://epics-controls.org/download/base/base-7.0.9.tar.gz.asc

# Check if the GPG key is already imported (4B688BF5CAF9452CBD5BE86FD306120EEACB4576)
if gpg --list-keys 4B688BF5CAF9452CBD5BE86FD306120EEACB4576 > /dev/null 2>&1; then
    echo "GPG key is already imported."
else
    echo "Importing GPG key..."
    gpg --recv-keys 4B688BF5CAF9452CBD5BE86FD306120EEACB4576
    if [ $? -ne 0 ]; then
        echo "Failed to import GPG key!"
        exit 1
    fi
fi
# Verify the signature
gpg --verify base-7.0.9.tar.gz.asc base-7.0.9.tar.gz
if [ $? -ne 0 ]; then
    echo "Signature verification failed!"
    exit 1
fi

# Remove any existing RPM build environment
rm -rf ~/rpmbuild || true

# Create the RPM build environment
rpmdev-setuptree

# Copy the source tarball and signature to the SOURCES directory
cp base-7.0.9* ~/rpmbuild/SOURCES/

cp *.patch ~/rpmbuild/SOURCES/

# Copy the spec file to the SPECS directory
cp epics-base.spec ~/rpmbuild/SPECS/
# Change to the SPECS directory
cd ~/rpmbuild/SPECS

# Build the RPM package
rpmbuild -ba epics-base.spec

# Check if the build was successful
if [ $? -ne 0 ]; then
    echo "RPM build failed!"
    exit 1
fi
# Copy the generated RPMs to the current directory
# Erstellt: /home/user/rpmbuild/SRPMS/epics-base-7.0.9-1.0.0-1.fc42.src.rpm
# Erstellt: /home/user/rpmbuild/RPMS/x86_64/epics-base-7.0.9-debugsource-1.0.0-1.fc42.x86_64.rpm
# Erstellt: /home/user/rpmbuild/RPMS/x86_64/epics-base-7.0.9-1.0.0-1.fc42.x86_64.rpm
# Erstellt: /home/user/rpmbuild/RPMS/x86_64/epics-base-7.0.9-debuginfo-1.0.0-1.fc42.x86_64.rpm


# Remove any existing epics-base-7.0.9 rpm files
rm -f $CURRENT_DIR/epics-base-7.0.9-*.rpm || true

cp ~/rpmbuild/RPMS/x86_64/epics-base-7.0.9-*.rpm $CURRENT_DIR/
cp ~/rpmbuild/SRPMS/epics-base-7.0.9-*.rpm $CURRENT_DIR/


echo "RPM build completed successfully!"
echo "RPMs are located in $CURRENT_DIR"
echo "Source RPMs are located in $CURRENT_DIR"
echo "The following RPMs were created:"
ls -l $CURRENT_DIR/epics-base-7.0.9-*.rpm
echo "Do you want to install the RPMs?"
read -p "y/n: " answer
if [[ $answer == "y" ]]; then
    sudo rpm -Uvh --replacepkgs $CURRENT_DIR/epics-base-7.0.9-*.rpm
    if [ $? -ne 0 ]; then
        echo "RPM installation failed!"
        exit 1
    fi
    echo "RPMs installed successfully!"
else
    echo "Skipping RPM installation."
fi

echo "Should the build directory be deleted?"
read -p "y/n: " answer
if [[ $answer == "y" ]]; then
    rm -rf ~/rpmbuild
    echo "Build directory deleted."
else
    echo "Build directory not deleted."
fi
echo "Script completed successfully!"