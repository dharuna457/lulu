#!/bin/bash

echo "LinuxDroid System Verification"
echo "=============================="
echo ""

# Colors
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m'

ERRORS=0
WARNINGS=0

# Function to check a requirement
check_requirement() {
    local name="$1"
    local command="$2"
    local type="${3:-error}"  # error or warning

    echo -n "Checking $name... "
    if eval "$command" &>/dev/null; then
        echo -e "${GREEN}✓${NC}"
        return 0
    else
        if [ "$type" == "error" ]; then
            echo -e "${RED}✗${NC}"
            ERRORS=$((ERRORS + 1))
        else
            echo -e "${YELLOW}⚠${NC}"
            WARNINGS=$((WARNINGS + 1))
        fi
        return 1
    fi
}

# Check CPU virtualization
echo "=== CPU Virtualization ==="
check_requirement "VT-x/AMD-V support" "grep -E '(vmx|svm)' /proc/cpuinfo" "warning"

# Check KVM
echo ""
echo "=== KVM Support ==="
check_requirement "KVM module loaded" "lsmod | grep -q kvm"
check_requirement "/dev/kvm exists" "test -e /dev/kvm"
check_requirement "/dev/kvm accessible" "test -r /dev/kvm && test -w /dev/kvm" "warning"

# Check required packages
echo ""
echo "=== Required Software ==="
check_requirement "QEMU" "command -v qemu-system-x86_64"
check_requirement "libvirt" "dpkg -l | grep -q libvirt0"
check_requirement "Qt6 Core" "pkg-config --exists Qt6Core" "warning"

# Check system resources
echo ""
echo "=== System Resources ==="

# RAM
TOTAL_RAM=$(grep MemTotal /proc/meminfo | awk '{print $2}')
TOTAL_RAM_GB=$((TOTAL_RAM / 1024 / 1024))
echo -n "RAM (minimum 8GB)... "
if [ "$TOTAL_RAM_GB" -ge 8 ]; then
    echo -e "${GREEN}✓ ${TOTAL_RAM_GB}GB${NC}"
else
    echo -e "${YELLOW}⚠ ${TOTAL_RAM_GB}GB (recommended: 8GB)${NC}"
    WARNINGS=$((WARNINGS + 1))
fi

# CPU cores
CPU_CORES=$(nproc)
echo -n "CPU cores (minimum 2)... "
if [ "$CPU_CORES" -ge 2 ]; then
    echo -e "${GREEN}✓ ${CPU_CORES} cores${NC}"
else
    echo -e "${RED}✗ ${CPU_CORES} cores${NC}"
    ERRORS=$((ERRORS + 1))
fi

# Disk space
DISK_SPACE=$({ df /opt 2>/dev/null || df /; } | tail -1 | awk '{print $4}')
DISK_SPACE_GB=$((DISK_SPACE / 1024 / 1024))
echo -n "Disk space (minimum 20GB)... "
if [ "$DISK_SPACE_GB" -ge 20 ]; then
    echo -e "${GREEN}✓ ${DISK_SPACE_GB}GB available${NC}"
else
    echo -e "${YELLOW}⚠ ${DISK_SPACE_GB}GB available (recommended: 20GB)${NC}"
    WARNINGS=$((WARNINGS + 1))
fi

# Check groups
echo ""
echo "=== User Permissions ==="
check_requirement "User in kvm group" "groups | grep -q kvm" "warning"
check_requirement "User in libvirt group" "groups | grep -q libvirt" "warning"

# Summary
echo ""
echo "=============================="
if [ "$ERRORS" -eq 0 ] && [ "$WARNINGS" -eq 0 ]; then
    echo -e "${GREEN}✓ System meets all requirements!${NC}"
    exit 0
elif [ "$ERRORS" -eq 0 ]; then
    echo -e "${YELLOW}⚠ System meets minimum requirements with $WARNINGS warnings${NC}"
    echo ""
    echo "Recommendations:"
    echo "- Add your user to kvm and libvirt groups:"
    echo "    sudo usermod -aG kvm,libvirt \$USER"
    echo "- Log out and log back in for group changes to take effect"
    exit 0
else
    echo -e "${RED}✗ System does not meet requirements ($ERRORS errors, $WARNINGS warnings)${NC}"
    echo ""
    echo "To fix:"
    echo "1. Enable virtualization in BIOS/UEFI"
    echo "2. Install QEMU: sudo apt install qemu-system-x86"
    echo "3. Install libvirt: sudo apt install libvirt0"
    echo "4. Add user to groups: sudo usermod -aG kvm,libvirt \$USER"
    exit 1
fi
