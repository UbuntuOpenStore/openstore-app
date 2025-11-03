#!/bin/bash

set -e

# Get the project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DOCKER_RUNNER="$PROJECT_ROOT/tools/docker/run-docker-tools.sh"

# Run the QML linter in Docker container
echo "Linting QML files using Docker container..."
if ! "$DOCKER_RUNNER" lint-qml; then
    echo -e "\n❌ Some QML files have linting errors. Please fix them."
    exit 1
else
    echo -e "\n✅ All QML files passed linting!"
    exit 0
fi
