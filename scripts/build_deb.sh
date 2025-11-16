#!/bin/bash

set -e

echo "╔════════════════════════════════════════════════════╗"
echo "║   LinuxDroid DEB Package Builder                  ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get script directory
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"
PROJECT_DIR="$(dirname "$SCRIPT_DIR")"

echo "Project directory: $PROJECT_DIR"
echo ""

# Check for required tools
echo "Checking dependencies..."
MISSING_DEPS=()

command -v cmake >/dev/null 2>&1 || MISSING_DEPS+=("cmake")
command -v make >/dev/null 2>&1 || MISSING_DEPS+=("make")
command -v g++ >/dev/null 2>&1 || MISSING_DEPS+=("g++")
command -v dpkg-deb >/dev/null 2>&1 || MISSING_DEPS+=("dpkg-deb")

if [ ${#MISSING_DEPS[@]} -ne 0 ]; then
    echo -e "${RED}Error: Missing required dependencies: ${MISSING_DEPS[*]}${NC}"
    echo "Install them with: sudo apt install ${MISSING_DEPS[*]}"
    exit 1
fi

# Check for Qt6
if ! pkg-config --exists Qt6Core; then
    echo -e "${YELLOW}Warning: Qt6 not found via pkg-config${NC}"
    echo "Install Qt6 with: sudo apt install qt6-base-dev"
    echo "Continuing anyway (cmake will fail if Qt6 is really missing)..."
fi

echo -e "${GREEN}✓ All dependencies found${NC}"
echo ""

# Clean previous build
if [ -d "$PROJECT_DIR/build" ]; then
    echo "Cleaning previous build..."
    rm -rf "$PROJECT_DIR/build"
fi

# Create build directory
echo "Creating build directory..."
mkdir -p "$PROJECT_DIR/build"
cd "$PROJECT_DIR/build"

# Run CMake
echo ""
echo "Running CMake..."
cmake -DCMAKE_BUILD_TYPE=Release \
      -DCMAKE_INSTALL_PREFIX=/usr \
      ..

if [ $? -ne 0 ]; then
    echo -e "${RED}CMake configuration failed!${NC}"
    exit 1
fi

# Build the project
echo ""
echo "Building LinuxDroid..."
make -j$(nproc)

if [ $? -ne 0 ]; then
    echo -e "${RED}Build failed!${NC}"
    exit 1
fi

echo -e "${GREEN}✓ Build successful${NC}"
echo ""

# Create package structure
echo "Creating Debian package structure..."
PACKAGE_NAME="linuxdroid_1.0.0_amd64"
PACKAGE_DIR="$PROJECT_DIR/build/$PACKAGE_NAME"

rm -rf "$PACKAGE_DIR"
mkdir -p "$PACKAGE_DIR/DEBIAN"
mkdir -p "$PACKAGE_DIR/usr/bin"
mkdir -p "$PACKAGE_DIR/usr/share/applications"
mkdir -p "$PACKAGE_DIR/usr/share/icons/hicolor/512x512/apps"
mkdir -p "$PACKAGE_DIR/usr/share/doc/linuxdroid"
mkdir -p "$PACKAGE_DIR/lib/systemd/system"
mkdir -p "$PACKAGE_DIR/opt/linuxdroid"

# Copy binaries
echo "Copying binaries..."
cp linuxdroid "$PACKAGE_DIR/usr/bin/"
cp linuxdroid-daemon "$PACKAGE_DIR/usr/bin/"

# Copy desktop file
echo "Copying desktop entry..."
cp "$PROJECT_DIR/debian/linuxdroid.desktop" "$PACKAGE_DIR/usr/share/applications/"

# Copy icon
echo "Copying icon..."
cp "$PROJECT_DIR/resources/icons/linuxdroid.png" "$PACKAGE_DIR/usr/share/icons/hicolor/512x512/apps/"

# Copy systemd service
echo "Copying systemd service..."
cp "$PROJECT_DIR/debian/linuxdroid-download.service" "$PACKAGE_DIR/lib/systemd/system/"

# Copy resources
echo "Copying resources..."
cp "$PROJECT_DIR/resources/android_images.json" "$PACKAGE_DIR/opt/linuxdroid/"

# Copy documentation
echo "Copying documentation..."
cp "$PROJECT_DIR/README.md" "$PACKAGE_DIR/usr/share/doc/linuxdroid/" 2>/dev/null || echo "README.md not found, skipping..."
cp "$PROJECT_DIR/LICENSE" "$PACKAGE_DIR/usr/share/doc/linuxdroid/" 2>/dev/null || echo "LICENSE not found, skipping..."
cp "$PROJECT_DIR/debian/copyright" "$PACKAGE_DIR/usr/share/doc/linuxdroid/"
cp "$PROJECT_DIR/debian/changelog" "$PACKAGE_DIR/usr/share/doc/linuxdroid/"

# Copy Debian control files
echo "Copying Debian control files..."
cp "$PROJECT_DIR/debian/control" "$PACKAGE_DIR/DEBIAN/"
cp "$PROJECT_DIR/debian/postinst" "$PACKAGE_DIR/DEBIAN/"
cp "$PROJECT_DIR/debian/postrm" "$PACKAGE_DIR/DEBIAN/"

# Set permissions
echo "Setting permissions..."
chmod 755 "$PACKAGE_DIR/DEBIAN/postinst"
chmod 755 "$PACKAGE_DIR/DEBIAN/postrm"
chmod 755 "$PACKAGE_DIR/usr/bin/linuxdroid"
chmod 755 "$PACKAGE_DIR/usr/bin/linuxdroid-daemon"
chmod 644 "$PACKAGE_DIR/usr/share/applications/linuxdroid.desktop"
chmod 644 "$PACKAGE_DIR/lib/systemd/system/linuxdroid-download.service"

# Build DEB package
echo ""
echo "Building .deb package..."
dpkg-deb --build "$PACKAGE_DIR"

if [ $? -ne 0 ]; then
    echo -e "${RED}Package creation failed!${NC}"
    exit 1
fi

# Move package to project root
mv "${PACKAGE_DIR}.deb" "$PROJECT_DIR/"

echo ""
echo "╔════════════════════════════════════════════════════╗"
echo "║   Package created successfully!                    ║"
echo "╚════════════════════════════════════════════════════╝"
echo ""
echo -e "${GREEN}Package: $PROJECT_DIR/${PACKAGE_NAME}.deb${NC}"
echo ""
echo "Package information:"
dpkg-deb --info "$PROJECT_DIR/${PACKAGE_NAME}.deb"
echo ""
echo "Package contents:"
dpkg-deb --contents "$PROJECT_DIR/${PACKAGE_NAME}.deb"
echo ""
echo "To install the package, run:"
echo "  sudo dpkg -i $PROJECT_DIR/${PACKAGE_NAME}.deb"
echo "  sudo apt-get install -f  # Install any missing dependencies"
echo ""
echo "Or simply:"
echo "  sudo apt install ./${PACKAGE_NAME}.deb"
echo ""
