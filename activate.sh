#!/bin/bash
# Zephyr 4.4.0 Development Environment Activation Script
# Usage: source activate.sh
#
# This script detects its own location so it works from any clone path.

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Activate conda environment (if conda is available)
if command -v conda &> /dev/null; then
    # Try to find the zephyr conda env
    CONDA_ENV=$(conda env list | awk '/^zephyr\s/{print $NF}')
    if [ -n "$CONDA_ENV" ]; then
        # shellcheck disable=SC1090
        source "$(conda info --base)/etc/profile.d/conda.sh" 2>/dev/null || true
        conda activate zephyr 2>/dev/null || true
    fi
fi

# Set Zephyr environment variables
export ZEPHYR_BASE="${SCRIPT_DIR}/zephyr"
export ZEPHYR_SDK_INSTALL_DIR="${ZEPHYR_SDK_INSTALL_DIR:-${HOME}/zephyr-sdk-1.0.1}"

# Add Zephyr scripts to PATH
export PATH="${ZEPHYR_BASE}/scripts:${PATH}"

echo "============================================"
echo " Zephyr Development Environment Activated"
echo "============================================"
echo " Zephyr Base:    ${ZEPHYR_BASE}"
if [ -f "${ZEPHYR_SDK_INSTALL_DIR}/sdk_version" ]; then
    echo " SDK Version:    $(cat "${ZEPHYR_SDK_INSTALL_DIR}/sdk_version")"
else
    echo " SDK:            ${ZEPHYR_SDK_INSTALL_DIR} (not found)"
fi
echo " Python:         $(python --version 2>&1)"
echo " West:           $(west --version 2>&1 2>/dev/null || echo 'not installed')"
echo "============================================"
echo ""
echo " Quick start:"
echo "   west build -b qemu_x86 samples/hello_world"
echo "   west build -t run"
echo ""
echo " To run the benchmark:"
echo "   cd zephyr-claude && python runner.py --check"
echo ""
