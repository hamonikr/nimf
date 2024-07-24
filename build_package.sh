#!/bin/bash
# 
# Create package of various environments from the source
# Wed, 24 Jul 2024 10:59:35 +0900
# Kevin Kim (chaeya@gmail.com)

# Exit immediately if a command exits with a non-zero status
set -e

# Define project root directory
PROJECT_ROOT=$(pwd)

# Define build directories
BUILD_DIR="${PROJECT_ROOT}/build"
RPMS_DIR="${BUILD_DIR}/RPMS"
DEBS_DIR="${BUILD_DIR}/DEBS"
ARCH_DIR="${BUILD_DIR}/ARCH"

# Function to build RPM package
build_rpm() {
    SPEC_FILE="${PROJECT_ROOT}/nimf.spec"
    SOURCES_DIR="${BUILD_DIR}/SOURCES"
    SPECS_DIR="${BUILD_DIR}/SPECS"

    # Create required directories
    mkdir -p "${BUILD_DIR}" "${RPMS_DIR}" "${SOURCES_DIR}" "${SPECS_DIR}"

    # Install required packages
    sudo yum install -y rpm-build rpmdevtools

    # Setup rpmbuild environment
    rpmdev-setuptree

    # Copy spec file to SPECS directory
    cp "${SPEC_FILE}" "${SPECS_DIR}"

    # Clone and prepare sources
    cd "${SOURCES_DIR}"
    git clone https://github.com/hamonikr/nimf.git
    cd nimf
    git archive --format=tar.gz --prefix=nimf/ HEAD > ../nimf.tar.gz
    cd ..

    # Return to the project root directory
    cd "${PROJECT_ROOT}"

    # Build the RPM
    rpmbuild --define "_topdir ${BUILD_DIR}" -ba "${SPECS_DIR}/nimf.spec"

    # Output the location of the built RPMs
    echo "RPMs have been built and are located in: ${RPMS_DIR}"
}

# Function to build DEB package
build_deb() {
    DEBIAN_DIR="${PROJECT_ROOT}/debian"
    
    # Create required directories
    mkdir -p "${BUILD_DIR}" "${DEBS_DIR}"

    # Install required packages
    sudo apt-get update
    sudo apt-get install -y build-essential devscripts debhelper

    # Clone and prepare sources
    cd "${BUILD_DIR}"
    git clone https://github.com/hamonikr/nimf.git
    cd nimf

    # Build the DEB package
    debuild -us -uc

    # Move DEB files to DEBS_DIR
    mv ../*.deb "${DEBS_DIR}"

    # Return to the project root directory
    cd "${PROJECT_ROOT}"

    # Output the location of the built DEBs
    echo "DEB packages have been built and are located in: ${DEBS_DIR}"
}

# Function to build Arch Linux package
build_arch() {
    PKGBUILD_FILE="${PROJECT_ROOT}/PKGBUILD"

    # Create required directories
    mkdir -p "${BUILD_DIR}" "${ARCH_DIR}"

    # Install required packages
    sudo pacman -Sy --needed base-devel

    # Copy PKGBUILD to build directory
    cp "${PKGBUILD_FILE}" "${BUILD_DIR}"

    # Build the Arch package
    cd "${BUILD_DIR}"
    makepkg -si

    # Move package files to ARCH_DIR
    mv *.pkg.tar.zst "${ARCH_DIR}"

    # Return to the project root directory
    cd "${PROJECT_ROOT}"

    # Output the location of the built Arch packages
    echo "Arch packages have been built and are located in: ${ARCH_DIR}"
}

# Check for command line arguments and call respective functions
if [ "$#" -eq 0 ]; then
    echo "No package type specified. Please use 'rpm', 'deb', or 'arch' as arguments."
    exit 1
fi

for arg in "$@"; do
    case $arg in
        rpm)
            build_rpm
            ;;
        deb)
            build_deb
            ;;
        arch)
            build_arch
            ;;
        *)
            echo "Invalid argument: $arg. Please use 'rpm', 'deb', or 'arch'."
            exit 1
            ;;
    esac
done
