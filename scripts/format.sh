#!/bin/bash

set -e

# Get the project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
DOCKER_RUNNER="$PROJECT_ROOT/tools/docker/run-docker-tools.sh"

# Run the formatter in Docker container
echo "Running code formatters using Docker container..."
"$DOCKER_RUNNER" format-all

echo "Formatting complete!"
