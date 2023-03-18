#!/bin/bash
trap 'kill -INT $TSC_PID ; kill -INT $PHP_PID ; sleep 2' EXIT

npx tsc --watch --project tsconfig.json &
TSC_PID=$!

php -S 127.0.0.1:9592 &
PHP_PID=$!

xdg-open http://127.0.0.1:9592/vis.html

./build.sh

inotifywait --include "(\.cpp|\.hpp|\.c|\.h|\.inc|Makefile|build.sh)\$" -q -m -r -e modify,delete,create . | while read DIRECTORY EVENT FILE; do
    echo Change detected, rebuilding
    sleep 0.5
    ./build.sh &
done
