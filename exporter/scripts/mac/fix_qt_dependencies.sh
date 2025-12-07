#!/bin/bash
# Fix all Qt framework dependencies for AE plugin
# This script converts @executable_path to @loader_path to work correctly when loaded by After Effects

set -e

PLUGIN_BUNDLE="$1"

if [ -z "$PLUGIN_BUNDLE" ]; then
    echo "Usage: $0 <path-to-plugin.bundle>"
    exit 1
fi

if [ ! -d "$PLUGIN_BUNDLE" ]; then
    echo "Error: Plugin bundle not found: $PLUGIN_BUNDLE"
    exit 1
fi

FRAMEWORKS_DIR="$PLUGIN_BUNDLE/Contents/Frameworks"
BINARY="$PLUGIN_BUNDLE/Contents/MacOS/PAGExporter"

echo "================================================"
echo "Fixing Qt dependencies for AE plugin"
echo "Plugin: $PLUGIN_BUNDLE"
echo "================================================"

# Ensure Frameworks directory exists
mkdir -p "$FRAMEWORKS_DIR"

# Copy libffaudio.dylib if not already present
SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXPORTER_DIR="$(cd "$SCRIPT_DIR/../.." && pwd)"
ARCH="x64"  # Assuming x64, adjust if needed
FFAUDIO_SOURCE="$EXPORTER_DIR/vendor/ffaudio/mac/$ARCH/libffaudio.dylib"
FFAUDIO_TARGET="$FRAMEWORKS_DIR/libffaudio.dylib"

if [ -f "$FFAUDIO_SOURCE" ]; then
    echo ""
    echo "=== Step 0: Copying libffaudio.dylib ==="
    cp "$FFAUDIO_SOURCE" "$FFAUDIO_TARGET"
    chmod +x "$FFAUDIO_TARGET"
    # Fix the install name
    install_name_tool -id "@loader_path/libffaudio.dylib" "$FFAUDIO_TARGET" 2>/dev/null || true
    echo "  Copied: libffaudio.dylib"
fi

# Function to fix a Mach-O binary's dependencies
fix_binary() {
    local file="$1"
    local base_path="$2"  # Relative path from binary to Frameworks dir
    
    if [ ! -f "$file" ]; then
        return
    fi
    
    # Check if it's a Mach-O binary
    if ! file "$file" | grep -q "Mach-O"; then
        return
    fi
    
    echo "Processing: $(basename "$file")"
    
    # Get all dependencies
    otool -L "$file" | tail -n +2 | awk '{print $1}' | grep -E "@executable_path|@rpath|@loader_path" | while read dep; do
        # Determine the new path
        if [[ "$dep" == @executable_path/* ]]; then
            # Replace @executable_path with @loader_path and adjust the relative path
            new_dep=$(echo "$dep" | sed "s|@executable_path|$base_path|")
        elif [[ "$dep" == @rpath/* ]]; then
            # Extract the framework/lib name from @rpath dependency
            dep_name=$(echo "$dep" | sed 's|@rpath/||')
            # Check if it's a framework (contains .framework)
            if [[ "$dep_name" == *.framework/* ]]; then
                # It's a framework - preserve the full path structure
                new_dep="$base_path/$dep_name"
            else
                # It's a dylib - just use the base name
                lib_name=$(basename "$dep")
                new_dep="$base_path/$lib_name"
            fi
        elif [[ "$dep" == @loader_path/* ]]; then
            # Check if this path structure is incorrect (e.g., missing .framework)
            dep_tail=$(echo "$dep" | sed 's|.*Frameworks/||')
            if [[ "$dep_tail" != "" ]] && [[ "$dep_tail" != *".framework"* ]] && [[ "$dep_tail" =~ ^Qt ]]; then
                # This is a Qt framework without proper .framework path - fix it
                fw_name="$dep_tail"
                new_dep="$base_path/$fw_name.framework/Versions/A/$fw_name"
                echo "  $dep (INCORRECT PATH)"
                echo "    -> $new_dep"
                install_name_tool -change "$dep" "$new_dep" "$file" 2>/dev/null || true
                continue
            else
                # Path looks correct, skip
                continue
            fi
        else
            continue
        fi
        
        echo "  $dep"
        echo "    -> $new_dep"
        install_name_tool -change "$dep" "$new_dep" "$file" 2>/dev/null || true
    done
}

check_and_copy_framework() {
    local fw_name="$1"
    local target="$FRAMEWORKS_DIR/$fw_name.framework"
    
    # Ensure Frameworks directory exists
    mkdir -p "$FRAMEWORKS_DIR"
    
    # Remove if it's a broken symlink or symlink
    if [ -L "$target" ]; then
        echo "  Removing symlink: $fw_name"
        rm -f "$target"
    fi
    
    if [ ! -d "$target" ]; then
        # Find the real framework location (resolve symlinks)
        local real_fw=$(find /usr/local/Cellar -name "$fw_name.framework" -type d 2>/dev/null | head -1)
        
        if [ -z "$real_fw" ] || [ ! -d "$real_fw/Versions/A" ]; then
            echo "  WARNING: Framework $fw_name not found"
            return
        fi
        
        echo "  Copying missing framework: $fw_name"
        # Create framework directory and copy Versions
        mkdir -p "$target/Versions"
        cp -R "$real_fw/Versions/A" "$target/Versions/"
        
        # Create proper symlinks
        cd "$target/Versions"
        ln -s A Current
        cd ..
        ln -s Versions/Current/$fw_name $fw_name
        ln -s Versions/Current/Headers Headers 2>/dev/null || true
        ln -s Versions/Current/Resources Resources 2>/dev/null || true
        cd - > /dev/null
        
        # Fix it immediately
        fw_binary="$target/Versions/A/$fw_name"
        if [ -f "$fw_binary" ]; then
            fix_binary "$fw_binary" "@loader_path/../../.."
            install_name_tool -id "@loader_path/../../../$fw_name.framework/Versions/A/$fw_name" "$fw_binary" 2>/dev/null || true
        fi
    fi
}

check_and_copy_dylib() {
    local lib_name="$1"
    local target="$FRAMEWORKS_DIR/$lib_name"
    
    if [ ! -f "$target" ] && [ -f "$QT_LIB_DIR/$lib_name" ]; then
        echo "  Copying missing dylib: $lib_name"
        cp "$QT_LIB_DIR/$lib_name" "$FRAMEWORKS_DIR/"
        fix_binary "$target" "@loader_path"
        install_name_tool -id "@loader_path/$lib_name" "$target" 2>/dev/null || true
    fi
}

# Step 1: Fix main binary
echo ""
echo "=== Step 1: Fixing main binary ==="
fix_binary "$BINARY" "@loader_path/../Frameworks"

# Step 2: Fix all frameworks (they need to reference each other and dylibs)
echo ""
echo "=== Step 2: Fixing Qt frameworks ==="
find "$FRAMEWORKS_DIR" -name "*.framework" -type d | while read fw_dir; do
    # Skip symlinks
    if [ -L "$fw_dir" ]; then
        continue
    fi
    
    # Find the actual binary inside framework (usually Versions/A/FrameworkName)
    fw_binary=$(find "$fw_dir" -type f -perm +111 | grep -E "Versions/A/Qt" | head -1)
    if [ -n "$fw_binary" ] && [ -f "$fw_binary" ]; then
        # From a framework binary, path to Frameworks is: ../../..
        # e.g., QtCore.framework/Versions/A/QtCore -> ../../../ gets to Frameworks/
        fix_binary "$fw_binary" "@loader_path/../../.."
        
        # Also fix the framework's install name
        fw_name=$(basename "$fw_dir" .framework)
        install_name_tool -id "@loader_path/../../../$fw_name.framework/Versions/A/$fw_name" "$fw_binary" 2>/dev/null || true
        
        # Fix incomplete Qt framework paths (missing .framework/Versions/A/)
        otool -L "$fw_binary" | grep "@loader_path/\.\./\.\./\.\./" | grep -v "\.framework" | awk '{print $1}' | while read dep; do
            lib_name=$(basename "$dep")
            if [[ "$lib_name" =~ ^Qt ]]; then
                new_path="@loader_path/../../../$lib_name.framework/Versions/A/$lib_name"
                echo "  Fixing incomplete path: $dep -> $new_path"
                install_name_tool -change "$dep" "$new_path" "$fw_binary" 2>/dev/null || true
            fi
        done
    fi
done

# Step 3: Fix all dylibs
echo ""
echo "=== Step 3: Fixing dylibs ==="
find "$FRAMEWORKS_DIR" -name "*.dylib" -type f | while read dylib; do
    fix_binary "$dylib" "@loader_path"
    
    # Fix the dylib's install name
    dylib_name=$(basename "$dylib")
    install_name_tool -id "@loader_path/$dylib_name" "$dylib" 2>/dev/null || true
done

# Step 4: Check for missing dependencies and copy them
echo ""
echo "=== Step 4: Checking for missing dependencies ==="

# Try to detect Qt library path from existing frameworks
if [ -d "$FRAMEWORKS_DIR/QtCore.framework" ]; then
    # Read the path from an existing framework to find Qt installation
    QT_LIB_DIR=$(otool -L "$FRAMEWORKS_DIR/QtCore.framework/Versions/A/QtCore" | grep -o "/.*Qt.*\.framework" | head -1 | sed 's|/[^/]*\.framework||')
fi

# Fallback to common locations if not found
if [ -z "$QT_LIB_DIR" ] || [ ! -d "$QT_LIB_DIR" ]; then
    for possible_path in "/usr/local/opt/qt/lib" "/opt/homebrew/opt/qt/lib" "$HOME/Qt/"*/macos/lib; do
        if [ -d "$possible_path" ]; then
            QT_LIB_DIR="$possible_path"
            break
        fi
    done
fi

echo "  Using Qt library path: $QT_LIB_DIR"

if [ -z "$QT_LIB_DIR" ] || [ ! -d "$QT_LIB_DIR" ]; then
    echo "  WARNING: Qt library directory not found, skipping dependency check"
else
    # Common missing frameworks
    for fw in QtDBus QtQuickTemplates2; do
        check_and_copy_framework "$fw"
    done

    # Common missing dylibs (check what's referenced but not present)
    for dylib in libbrotlicommon.1.dylib; do
        check_and_copy_dylib "$dylib"
    done
fi

# Step 5: Re-run fixes to catch newly added dependencies
echo ""
echo "=== Step 5: Final pass to fix all dependencies ==="
fix_binary "$BINARY" "@loader_path/../Frameworks"

find "$FRAMEWORKS_DIR" -name "*.framework" -type d | while read fw_dir; do
    # Skip symlinks
    if [ -L "$fw_dir" ]; then
        continue
    fi
    
    fw_binary=$(find "$fw_dir" -type f -perm +111 | grep -E "Versions/A/Qt" | head -1)
    if [ -n "$fw_binary" ] && [ -f "$fw_binary" ]; then
        fix_binary "$fw_binary" "@loader_path/../../.."
        
        # Fix incomplete Qt framework paths again
        otool -L "$fw_binary" | grep "@loader_path/\.\./\.\./\.\./" | grep -v "\.framework" | awk '{print $1}' | while read dep; do
            lib_name=$(basename "$dep")
            if [[ "$lib_name" =~ ^Qt ]]; then
                new_path="@loader_path/../../../$lib_name.framework/Versions/A/$lib_name"
                install_name_tool -change "$dep" "$new_path" "$fw_binary" 2>/dev/null || true
            fi
        done
    fi
done

find "$FRAMEWORKS_DIR" -name "*.dylib" -type f | while read dylib; do
    fix_binary "$dylib" "@loader_path"
done

# Step 6: Create PkgInfo if missing
echo ""
echo "=== Step 6: Creating PkgInfo ==="
PKGINFO="$PLUGIN_BUNDLE/Contents/PkgInfo"
if [ ! -f "$PKGINFO" ]; then
    printf "AEgxFXTC" > "$PKGINFO"
    echo "  Created PkgInfo"
fi

echo ""
echo "================================================"
echo "✅ All dependencies fixed successfully!"
echo "================================================"
echo ""
echo "Plugin is ready to install to:"
echo "  /Applications/Adobe After Effects 2024/Plug-ins/Keyframe/"
echo ""
