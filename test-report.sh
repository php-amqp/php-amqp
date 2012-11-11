#!/bin/sh
result=`find tests -type f -and -name "*.diff" -or -name "*.out" | sort | while read file; do
    echo "FILE: ${file}"
    cat "$file"
    echo "\n"
done`

if [ -n "$result" ]; then
    echo "$result"
    exit 1
fi
