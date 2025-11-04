#!/bin/bash

set -e

# Get the project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/../.." && pwd)"
DOCKER_DIR="$PROJECT_ROOT/tools/docker"
IMAGE_NAME="bhdouglass/openstore-app-tools"

# Build the formatter Docker image if it doesn't exist
build_image_if_needed() {
    if ! docker image inspect "$IMAGE_NAME" &> /dev/null; then
        echo "Building the OpenStore formatter Docker image..."
        docker build -t "$IMAGE_NAME" "$DOCKER_DIR"
    fi
}

# Run a command in the Docker container
run_docker() {
    docker run --rm -v "$PROJECT_ROOT:/code" "$IMAGE_NAME" /usr/local/bin/docker-runner.sh "$1"
}

build_image_if_needed

COMMAND=$1

# Pass the command directly to the docker-runner.sh script inside the container
run_docker "$COMMAND"

