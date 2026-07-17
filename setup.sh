#!/bin/bash
# ===============================================================
#  Zephyr Workspace - One-Click Environment Setup
# ===============================================================
# Usage: bash setup.sh [--no-sdk]
#
# This script will:
#   1. Fetch Zephyr source + modules via west (west init + west update)
#   2. Create conda environment from environment.yml
#   3. Download & install Zephyr SDK (unless --no-sdk)
#   4. Build Docker sandbox image
#   5. Print instructions for API key configuration
# ===============================================================

set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "$0")" && pwd)"
SDK_DIR="${HOME}/zephyr-sdk-1.0.1"

echo "=============================================="
echo " Zephyr Workspace Setup"
echo "=============================================="
echo ""

# -------------------------------------------------------
# Step 1: West init + update (fetch Zephyr source + modules)
# -------------------------------------------------------
echo "[1/5] Fetching Zephyr source code and modules via west..."

if [ -f "${SCRIPT_DIR}/zephyr/CMakeLists.txt" ]; then
    echo "  → zephyr/ already exists, skipping west init."
    echo "  → To refresh: rm -rf .west && west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.0 && west update"
elif command -v west &> /dev/null; then
    echo "  → Initializing west with Zephyr v4.4.0 manifest..."
    cd "${SCRIPT_DIR}"
    rm -rf .west
    west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.0
    echo "  → Fetching projects (zephyr/, modules/, bootloader/, tools/)..."
    west update
    echo "  ✓ Zephyr source and modules fetched."
else
    echo "  ⚠ 'west' command not found. Install it first:"
    echo "    pip install west"
    echo "  Then re-run this script."
    echo "  To fetch manually:"
    echo "    cd ${SCRIPT_DIR}"
    echo "    rm -rf .west"
    echo "    west init -m https://github.com/zephyrproject-rtos/zephyr --mr v4.4.0"
    echo "    west update"
fi
echo ""

# -------------------------------------------------------
# Step 2: Conda environment
# -------------------------------------------------------
echo "[2/5] Creating conda environment 'zephyr'..."
if command -v conda &> /dev/null; then
    if conda env list | grep -q "^zephyr "; then
        echo "  → Environment 'zephyr' already exists, skipping."
        echo "  → To recreate: conda env remove -n zephyr && conda env create -f environment.yml"
    else
        conda env create -f "${SCRIPT_DIR}/environment.yml"
        echo "  ✓ Conda environment created."
    fi
else
    echo "  ⚠ conda not found. Install Miniconda first:"
    echo "    https://docs.anaconda.com/miniconda/"
    echo "  Then re-run this script."
    echo "  Alternatively, install dependencies manually:"
    echo "    pip install west pyelftools PyYAML pykwalify jsonschema packaging pyserial intelhex"
    echo "    pip install tree-sitter tree-sitter-c"
fi
echo ""

# -------------------------------------------------------
# Step 3: Zephyr SDK
# -------------------------------------------------------
if [ "${1:-}" = "--no-sdk" ]; then
    echo "[3/5] Skipping Zephyr SDK installation (--no-sdk)."
else
    echo "[3/5] Installing Zephyr SDK ${SDK_VERSION:-1.0.1}..."
    SDK_VERSION="1.0.1"
    if [ -d "${SDK_DIR}" ]; then
        echo "  → Zephyr SDK ${SDK_VERSION} already installed at ${SDK_DIR}."
    else
        echo "[2/4] Downloading Zephyr SDK ${SDK_VERSION}..."
        SDK_TARBALL="zephyr-sdk-${SDK_VERSION}_linux-x86_64.tar.xz"
        SDK_URL="https://github.com/zephyrproject-rtos/sdk-ng/releases/download/v${SDK_VERSION}/${SDK_TARBALL}"

        if command -v wget &> /dev/null; then
            wget -q --show-progress "${SDK_URL}" -O "/tmp/${SDK_TARBALL}"
        elif command -v curl &> /dev/null; then
            curl -L -o "/tmp/${SDK_TARBALL}" "${SDK_URL}"
        else
            echo "  ✗ Neither wget nor curl found. Install one and re-run."
            exit 1
        fi

        echo "  → Extracting to ${SDK_DIR}..."
        tar xf "/tmp/${SDK_TARBALL}" -C "$(dirname "${SDK_DIR}")"
        rm "/tmp/${SDK_TARBALL}"

        echo "  → Running SDK setup..."
        cd "${SDK_DIR}"
        ./setup.sh
        echo "  ✓ Zephyr SDK ${SDK_VERSION} installed."
    fi
fi
echo ""

# -------------------------------------------------------
# Step 4: Docker sandbox
# -------------------------------------------------------
echo "[4/5] Building Docker sandbox image 'zephyr-sandbox:latest'..."
if command -v docker &> /dev/null; then
    docker build -t zephyr-sandbox:latest "${SCRIPT_DIR}/zephyr-claude"
    echo "  ✓ Docker image built."
else
    echo "  ⚠ Docker not found. Install Docker Desktop and re-run this step:"
    echo "    docker build -t zephyr-sandbox:latest ${SCRIPT_DIR}/zephyr-claude"
fi
echo ""

# -------------------------------------------------------
# Step 5: API key configuration
# -------------------------------------------------------
echo "[5/5] API key configuration..."
CLAUDE_SETTINGS="${HOME}/.claude/settings.json"
if [ -f "${CLAUDE_SETTINGS}" ]; then
    echo "  → ~/.claude/settings.json already exists."
    echo "  → Verify your API key is set correctly (see claude_settings.template)"
else
    mkdir -p "${HOME}/.claude"
    cp "${SCRIPT_DIR}/claude_settings.template" "${CLAUDE_SETTINGS}"
    echo "  → Created ~/.claude/settings.json from template."
    echo ""
    echo "  ⚠ IMPORTANT: Edit ~/.claude/settings.json and replace:"
    echo "     \"ANTHROPIC_AUTH_TOKEN\": \"sk-xxx\"  →  your real API key"
    echo "     \"ANTHROPIC_MODEL\": \"<model-name>\"  →  e.g. deepseek-v4-flash"
fi
echo ""

# -------------------------------------------------------
# Done
# -------------------------------------------------------
echo "=============================================="
echo " Setup complete!"
echo "=============================================="
echo ""
echo "To activate the environment:"
echo "  source activate.sh"
echo ""
echo "To verify all dependencies:"
echo "  cd zephyr-claude"
echo "  python runner.py --check"
echo ""
echo "To run the benchmark:"
echo "  python runner.py --batch 10"
echo ""
