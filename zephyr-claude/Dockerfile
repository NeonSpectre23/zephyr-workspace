FROM ubuntu:24.04

# System dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    cmake \
    ninja-build \
    python3 \
    python3-pip \
    python3-venv \
    python3-dev \
    git \
    wget \
    curl \
    ca-certificates \
    && rm -rf /var/lib/apt/lists/*

# Python packages for Zephyr
RUN pip3 install --break-system-packages \
    west \
    pyelftools \
    PyYAML \
    pykwalify \
    jsonschema \
    packaging \
    requests \
    pyserial \
    intelhex \
    && rm -rf /root/.cache/pip

# Node.js 22.x (for Claude Code CLI)
RUN curl -fsSL https://deb.nodesource.com/setup_22.x | bash - \
    && apt-get install -y nodejs \
    && rm -rf /var/lib/apt/lists/*

# Claude Code CLI
RUN npm install -g @anthropic-ai/claude-code@2.1.146 \
    && rm -rf /root/.npm

# Non-root user
RUN useradd -m -s /bin/bash agent
WORKDIR /workspace

ENTRYPOINT ["bash"]
