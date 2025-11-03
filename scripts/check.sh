#!/bin/bash

set -e

# Get the project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DOCKER_RUNNER="$PROJECT_ROOT/tools/docker/run-docker-tools.sh"

if ! "$DOCKER_RUNNER" check-all; then
    echo -e "\n❌ Some files are not properly formatted. Please run ./scripts/format.sh to format them."
    exit 1
else
    echo -e "\n✅ All files are properly formatted!"
    exit 0
fi
