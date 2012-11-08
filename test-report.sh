#!/bin/sh
find tests -type f -and -name "*.diff" -or -name "*.out" | sort | while read file; do
    echo "FILE: ${file}"
    cat "$file"
    echo "\n"
done
