#!/bin/bash

set -e

# Get the project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DOCKER_DIR="$PROJECT_ROOT/tools/docker"

# Build the formatter Docker image if it doesn't exist
build_image_if_needed() {
    if ! docker image inspect bhdouglass/openstore-app-tools &> /dev/null; then
        echo "Building the OpenStore formatter Docker image..."
        docker build -t bhdouglass/openstore-app-tools "$DOCKER_DIR"
    fi
}

build_image_if_needed

docker run --rm -v "$PROJECT_ROOT:/code" bhdouglass/openstore-app-tools "$@"
