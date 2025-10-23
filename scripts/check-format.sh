#!/bin/bash

set -e

# Get the project root directory
PROJECT_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"

echo "Checking code formatting with clang-format..."

# Find all C++ source files
FAILED=0
find "$PROJECT_ROOT" -type f \( -name "*.cpp" -o -name "*.h" \) \
    -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do

    if ! clang-format -style=file --dry-run -Werror "$file" &>/dev/null; then
        echo "❌ $file is not properly formatted"
        # Show the formatting diff
        clang-format -style=file "$file" | diff -u --color=always "$file" - || true
        FAILED=1
    else
        echo "✅ $file is properly formatted"
    fi
done

if [ $FAILED -eq 1 ]; then
    echo -e "\n❌ Some files are not properly formatted. Please run ./scripts/format-code.sh to format them."
    exit 1
else
    echo -e "\n✅ All files are properly formatted!"
    exit 0
fi
