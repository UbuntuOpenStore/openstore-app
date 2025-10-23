#!/bin/bash

set -e

# Get the project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Running clang-format on all C++ files in $PROJECT_ROOT"

find "$PROJECT_ROOT" -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "*/build/*" | while read -r file; do
    echo "Formatting $file"
    clang-format -i -style=file "$file"
done

echo "Formatting complete!"
