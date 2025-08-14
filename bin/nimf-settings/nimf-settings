#!/bin/bash
# Nimf Settings Universal Wrapper Script
# Handles compatibility issues across different desktop environments

# Function to detect virtualization environment
detect_virtual_env() {
    if command -v lspci >/dev/null 2>&1; then
        if lspci 2>/dev/null | grep -iq 'vmware\|virtualbox\|qemu'; then
            return 0  # Running in virtual environment
        fi
    fi
    
    # Additional checks for virtual environments
    if [ -d "/proc/vz" ] || [ -f "/.dockerenv" ]; then
        return 0  # Container or OpenVZ
    fi
    
    # Check for hypervisor
    if [ -r "/sys/hypervisor/type" ]; then
        return 0  # Xen or other hypervisor
    fi
    
    return 1  # Running on physical hardware
}

# Function to detect GTK version
detect_gtk_version() {
    if command -v ldd >/dev/null 2>&1; then
        if ldd /usr/bin/nimf-settings-bin 2>/dev/null | grep -q "libgtk-4"; then
            echo "gtk4"
            return
        elif ldd /usr/bin/nimf-settings-bin 2>/dev/null | grep -q "libgtk-3"; then
            echo "gtk3"
            return
        fi
    fi
    echo "unknown"
}

# Main execution logic
main() {
    local gtk_version=$(detect_gtk_version)
    
    # Try different approaches based on environment
    if detect_virtual_env; then
        # For GTK4 in virtual environments
        if [ "$gtk_version" = "gtk4" ]; then
            GSK_RENDERER=cairo nimf-settings-bin "$@" 2>/dev/null && exit 0
        fi
        
        # For GTK3 or general virtual env issues
        # Try with X11 backend (prevents Wayland issues in VMs)
        GDK_BACKEND=x11 nimf-settings-bin "$@" 2>/dev/null && exit 0
        
        # Try with default theme (prevents theme loading issues)
        GTK_THEME=Adwaita nimf-settings-bin "$@" 2>/dev/null && exit 0
        
        # Try disabling animations (reduces GPU load)
        GTK_ENABLE_ANIMATIONS=0 nimf-settings-bin "$@" 2>/dev/null && exit 0
        
        # Try with software OpenGL
        LIBGL_ALWAYS_SOFTWARE=1 nimf-settings-bin "$@" 2>/dev/null && exit 0
    fi
    
    # Default execution for physical hardware or when all else fails
    exec nimf-settings-bin "$@"
}

# Execute main function
main "$@"