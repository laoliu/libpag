#!/bin/bash
# One-click installer for PAG Exporter plugin
# This script compiles, fixes dependencies, and installs the plugin to After Effects

set -e

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
PROJECT_ROOT="$(cd "$SCRIPT_DIR/../../.." && pwd)"
BUILD_DIR="$PROJECT_ROOT/exporter/build"
PLUGIN_NAME="PAGExporter.plugin"
AE_PLUGIN_DIR="/Applications/Adobe After Effects 2024/Plug-ins/Keyframe"

echo "================================================"
echo "PAG Exporter - One-Click Installer for AE 2024"
echo "================================================"
echo ""

# Step 1: Build the plugin
echo "[1/5] Building plugin..."
cd "$BUILD_DIR"
ninja

# Step 2: Run macdeployqt
echo ""
echo "[2/5] Deploying Qt frameworks..."
macdeployqt "$PLUGIN_NAME" -always-overwrite

# Step 3: Add missing libraries
echo ""
echo "[3/5] Adding missing libraries..."
cp "$PROJECT_ROOT/exporter/vendor/ffaudio/mac/x64/libffaudio.dylib" "$PLUGIN_NAME/Contents/Frameworks/"
cp /usr/local/Cellar/brotli/*/lib/libbrotlicommon.1*.dylib "$PLUGIN_NAME/Contents/Frameworks/libbrotlicommon.1.dylib" 2>/dev/null || true

# Step 4: Fix ALL dependencies recursively
echo ""
echo "[4/5] Fixing all dependency paths..."
python3 - << 'PYTHON_SCRIPT'
import subprocess
import os
import re

PLUGIN = os.getenv('PLUGIN_PATH')
FRAMEWORKS = f"{PLUGIN}/Contents/Frameworks"
BINARY = f"{PLUGIN}/Contents/MacOS/PAGExporter"

def run_cmd(cmd):
    return subprocess.run(cmd, shell=True, capture_output=True, text=True)

def fix_file(filepath, base_path="@loader_path"):
    if not os.path.isfile(filepath):
        return
    
    result = run_cmd(f"file '{filepath}'")
    if "Mach-O" not in result.stdout:
        return
    
    print(f"  Fixing: {os.path.basename(filepath)}")
    
    result = run_cmd(f"otool -L '{filepath}'")
    for line in result.stdout.split('\n')[1:]:
        match = re.search(r'^\s*(@executable_path|@rpath)(\/[^\s]+)', line)
        if match:
            old_path = match.group(0).strip()
            if '@executable_path/../Frameworks' in old_path:
                new_path = old_path.replace('@executable_path', base_path)
            elif '@rpath/' in old_path:
                lib_name = os.path.basename(old_path)
                if base_path == "@loader_path":
                    new_path = f"@loader_path/{lib_name}"
                else:
                    new_path = f"{base_path}/../Frameworks/{lib_name}"
            elif '@executable_path' in old_path:
                new_path = old_path.replace('@executable_path', base_path)
            else:
                continue
            
            run_cmd(f"install_name_tool -change '{old_path}' '{new_path}' '{filepath}' 2>/dev/null")

# Fix main binary
print("Main binary:")
fix_file(BINARY, "@loader_path/../Frameworks")

# Fix frameworks
print("\nFrameworks:")
for root, dirs, files in os.walk(FRAMEWORKS):
    for file in files:
        filepath = os.path.join(root, file)
        if '.framework' in filepath:
            # Framework binaries need to go up to Frameworks dir
            if 'Versions/A/' in filepath and not file.endswith('.dylib'):
                fix_file(filepath, "@loader_path/../../..")
        elif filepath.endswith('.dylib'):
            fix_file(filepath, "@loader_path")
            # Fix dylib ID
            dylib_name = os.path.basename(filepath)
            run_cmd(f"install_name_tool -id '@loader_path/{dylib_name}' '{filepath}' 2>/dev/null")

print("\n✅ All paths fixed!")
PYTHON_SCRIPT

export PLUGIN_PATH="$BUILD_DIR/$PLUGIN_NAME"
python3 << 'PYTHON_SCRIPT'
import subprocess
import os
import re

PLUGIN = os.getenv('PLUGIN_PATH')
FRAMEWORKS = f"{PLUGIN}/Contents/Frameworks"
BINARY = f"{PLUGIN}/Contents/MacOS/PAGExporter"

def run_cmd(cmd):
    return subprocess.run(cmd, shell=True, capture_output=True, text=True)

def fix_file(filepath, base_path="@loader_path"):
    if not os.path.isfile(filepath):
        return
    
    result = run_cmd(f"file '{filepath}'")
    if "Mach-O" not in result.stdout:
        return
    
    print(f"  {os.path.basename(filepath)}")
    
    result = run_cmd(f"otool -L '{filepath}'")
    for line in result.stdout.split('\n')[1:]:
        match = re.search(r'^\s*(@executable_path|@rpath)(\/[^\s]+)', line)
        if match:
            old_path = match.group(0).strip()
            if '@executable_path/../Frameworks' in old_path:
                new_path = old_path.replace('@executable_path', base_path)
            elif '@rpath/' in old_path:
                lib_name = os.path.basename(old_path)
                if base_path == "@loader_path":
                    new_path = f"@loader_path/{lib_name}"
                else:
                    new_path = f"{base_path}/../Frameworks/{lib_name}"
            elif '@executable_path' in old_path:
                new_path = old_path.replace('@executable_path', base_path)
            else:
                continue
            
            run_cmd(f"install_name_tool -change '{old_path}' '{new_path}' '{filepath}' 2>/dev/null")

fix_file(BINARY, "@loader_path/../Frameworks")

for root, dirs, files in os.walk(FRAMEWORKS):
    for file in files:
        filepath = os.path.join(root, file)
        if '.framework' in filepath:
            if 'Versions/A/' in filepath and not file.endswith('.dylib'):
                fix_file(filepath, "@loader_path/../../..")
        elif filepath.endswith('.dylib'):
            fix_file(filepath, "@loader_path")
            dylib_name = os.path.basename(filepath)
            run_cmd(f"install_name_tool -id '@loader_path/{dylib_name}' '{filepath}' 2>/dev/null")
PYTHON_SCRIPT

# Step 5: Install to After Effects
echo ""
echo "[5/5] Installing to After Effects..."
sudo rm -rf "$AE_PLUGIN_DIR/$PLUGIN_NAME"
sudo cp -R "$PLUGIN_NAME" "$AE_PLUGIN_DIR/"

echo ""
echo "================================================"
echo "✅ Installation Complete!"
echo "================================================"
echo ""
echo "Plugin installed to:"
echo "  $AE_PLUGIN_DIR/$PLUGIN_NAME"
echo ""
echo "Next steps:"
echo "  1. Quit After Effects completely"
echo "  2. Restart After Effects"
echo "  3. Check File → Export menu for PAG options"
echo ""
