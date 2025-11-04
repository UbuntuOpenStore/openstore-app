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
    docker run --rm -v "$PROJECT_ROOT:/code" "$IMAGE_NAME" bash -c "$1"
}

build_image_if_needed

COMMAND=$1

case "$COMMAND" in
    format-cpp)
        echo "Formatting C++ files..."
        run_docker 'find /code -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "*/build/*" -not -path "*/\.*/*" -exec clang-format -i -style=file {} \;'
        ;;

    check-cpp)
        echo "Checking C++ format..."
        run_docker '
            FAILED=0
            find /code -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
                if ! clang-format -style=file --dry-run -Werror "$file" &>/dev/null; then
                    echo "❌ $file is not properly formatted"
                    clang-format -style=file "$file" | diff -u --color=always "$file" - || true
                    FAILED=1
                else
                    echo "✅ $file is properly formatted"
                fi
            done
            exit $FAILED
        '
        ;;

    # format-qml)
    #     echo "Formatting QML files..."
    #     run_docker 'find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" -exec qmlformat -i {} \;'
    #     ;;

    # check-qml)
    #     echo "Checking QML format..."
    #     run_docker '
    #         FAILED=0
    #         find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
    #             temp_file=$(mktemp)
    #             qmlformat "$file" > "$temp_file"

    #             if ! diff -q "$file" "$temp_file" &>/dev/null; then
    #                 echo "❌ $file is not properly formatted"
    #                 diff -u --color=always "$file" "$temp_file" || true
    #                 FAILED=1
    #             else
    #                 echo "✅ $file is properly formatted"
    #             fi

    #             rm "$temp_file"
    #         done
    #         exit $FAILED
    #     '
    #     ;;

    lint-qml)
        echo "Linting QML files..."
        run_docker '
            FAILED=0
            find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
                if ! qmllint "$file" &>/dev/null; then
                    echo "❌ $file has QML lint errors:"
                    qmllint "$file"
                    FAILED=1
                else
                    echo "✅ $file passed QML lint"
                fi
            done
            exit $FAILED
        '
        ;;

    format-all)
        echo "Formatting all files..."
        run_docker '
            # Format C++ files
            find /code -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "*/build/*" -not -path "*/\.*/*" -exec clang-format -i -style=file {} \;

            # Format QML files
            # find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" -exec qmlformat -i {} \;
        '
        ;;

    check-all)
        echo "Checking all file formats..."
        run_docker '
            FAILED=0

            # Check C++ files
            echo "Checking C++ files..."
            find /code -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
                if ! clang-format -style=file --dry-run -Werror "$file" &>/dev/null; then
                    echo "❌ $file is not properly formatted"
                    clang-format -style=file "$file" | diff -u --color=always "$file" - || true
                    FAILED=1
                else
                    echo "✅ $file is properly formatted"
                fi
            done

            # Check QML files
            # echo "Checking QML files..."
            # find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
            #     temp_file=$(mktemp)
            #     qmlformat "$file" > "$temp_file"

            #     if ! diff -q "$file" "$temp_file" &>/dev/null; then
            #         echo "❌ $file is not properly formatted"
            #         diff -u --color=always "$file" "$temp_file" || true
            #         FAILED=1
            #     else
            #         echo "✅ $file is properly formatted"
            #     fi

            #     rm "$temp_file"
            # done

            exit $FAILED
        '
        ;;

    *)
        echo "Unknown command: $COMMAND"
        echo "Available commands: format-cpp, check-cpp, format-qml, check-qml, lint-qml, format-all, check-all"
        exit 1
        ;;
esac
