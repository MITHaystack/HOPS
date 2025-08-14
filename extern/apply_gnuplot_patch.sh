#!/bin/bash

# Script to apply gnuplot error suppression patch to matplotplusplus
# This suppresses gnuplot stderr messages that appear during plotting

SCRIPT_DIR="$(pwd)"
PATCH_FILE="$SCRIPT_DIR/suppress_gnuplot_errors.patch"
TARGET_FILE="$SCRIPT_DIR/matplotplusplus/source/matplot/backend/gnuplot.cpp"

echo "Applying gnuplot error suppression patch..."
echo "Patch file: $PATCH_FILE"
echo "Target file: $TARGET_FILE"

# Check if target file exists
if [ ! -f "$TARGET_FILE" ]; then
    echo "Error: Target file not found: $TARGET_FILE"
    echo "Make sure you're running this script from the project root directory"
    exit 1
fi

# Check if patch file exists
if [ ! -f "$PATCH_FILE" ]; then
    echo "Error: Patch file not found: $PATCH_FILE"
    exit 1
fi

# Check if patch is already applied
if grep -q "gnuplot 2>/dev/null" "$TARGET_FILE"; then
    echo "Patch appears to already be applied (found '2>/dev/null' in target file)"
    echo "Skipping patch application"
    exit 0
fi

# Apply the patch
if patch -p1 < "$PATCH_FILE"; then
    echo "Patch applied successfully!"
    echo "Gnuplot error messages should now be suppressed"
else
    echo "Error: Failed to apply patch"
    echo "This might happen if:"
    echo "  1. The patch has already been applied"
    echo "  2. The matplotplusplus source code has changed"
    echo "  3. The file has been modified in an incompatible way"
    exit 1
fi

echo "Done!"
