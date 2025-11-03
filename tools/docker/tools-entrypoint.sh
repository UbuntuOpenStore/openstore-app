#!/bin/bash

set -e

COMMAND=$1

case "$COMMAND" in
    format-cpp)
        echo "Formatting C++ files..."
        find /code -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
            clang-format -i -style=file "$file"
        done
        ;;

    check-cpp)
        echo "Checking C++ format..."
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
        ;;

    format-qml)
        echo "Formatting QML files..."
        find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
            qmlformat -i "$file"
        done
        ;;

    check-qml)
        echo "Checking QML format..."
        FAILED=0
        find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
            temp_file=$(mktemp)
            qmlformat "$file" > "$temp_file"

            if ! diff -q "$file" "$temp_file" &>/dev/null; then
                echo "❌ $file is not properly formatted"
                diff -u --color=always "$file" "$temp_file" || true
                FAILED=1
            else
                echo "✅ $file is properly formatted"
            fi

            rm "$temp_file"
        done
        exit $FAILED
        ;;

    lint-qml)
        echo "Linting QML files..."
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
        ;;

    format-all)
        echo "Formatting all files..."
        # Format C++ files
        find /code -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
            clang-format -i -style=file "$file"
        done

        # Format QML files
        find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
            qmlformat -i "$file"
        done
        ;;

    check-all)
        echo "Checking all file formats..."
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
        echo -e "\nChecking QML files..."
        find /code -type f -name "*.qml" -not -path "*/build/*" -not -path "*/\.*/*" | while read -r file; do
            temp_file=$(mktemp)
            qmlformat "$file" > "$temp_file"

            if ! diff -q "$file" "$temp_file" &>/dev/null; then
                echo "❌ $file is not properly formatted"
                diff -u --color=always "$file" "$temp_file" || true
                FAILED=1
            else
                echo "✅ $file is properly formatted"
            fi

            rm "$temp_file"
        done

        exit $FAILED
        ;;

    *)
        echo "Unknown command: $COMMAND"
        echo "Available commands: format-cpp, check-cpp, format-qml, check-qml, lint-qml, format-all, check-all"
        exit 1
        ;;
esac
