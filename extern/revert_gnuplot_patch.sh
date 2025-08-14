#!/bin/bash

# Script to revert the gnuplot error suppression patch from matplotplusplus
# This restores the original gnuplot command without stderr redirection

SCRIPT_DIR="$(pwd)"
PATCH_FILE="$SCRIPT_DIR/suppress_gnuplot_errors.patch"
TARGET_FILE="$SCRIPT_DIR/matplotplusplus/source/matplot/backend/gnuplot.cpp"

echo "Reverting gnuplot error suppression patch..."
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

# Check if patch is currently applied
if ! grep -q "gnuplot 2>/dev/null" "$TARGET_FILE"; then
    echo "Patch does not appear to be applied (no '2>/dev/null' found in target file)"
    echo "Nothing to revert"
    exit 0
fi

# Revert the patch
if patch -R -p1 < "$PATCH_FILE"; then
    echo "Patch reverted successfully!"
    echo "Gnuplot error messages will now be visible again"
else
    echo "Error: Failed to revert patch"
    echo "You may need to manually edit the file or restore from backup"
    exit 1
fi

echo "Done!"
