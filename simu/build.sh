#!/bin/bash
set -eo pipefail
LOCK_FILES=
trap 'rm -f $LOCK_FILES' EXIT

CLANG="/home/doki/my/wasi-sdk-19.0/bin/clang --sysroot /home/doki/my/wasi-sdk-19.0/share/wasi-sysroot"
OPT="/home/doki/my/binaryen-version_112/bin/wasm-opt"

CLANG_FLAGS="-O3 -g -mexec-model=reactor"
OPT_FLAGS="-O4"
CLANG_FLAGS="-O0 -g -gdwarf-5 -gpubnames -mexec-model=reactor"
OPT_FLAGS=""

function lock-file {
    TMP_NAME=`mktemp -u`
    TMP_DIR=`dirname $TMP_NAME`
    LOCK_NAME="$TMP_DIR/$1"
    if [ -e $LOCK_NAME ]; then
        return 1
    fi
    echo $$ > $TMP_NAME
    mv -n $TMP_NAME $LOCK_NAME
    PID=`cat $LOCK_NAME`
    if [ "$PID" != "$$" ]; then
        return 1
    fi
    LOCK_FILES="$LOCK_FILES $LOCK_NAME"
    return 0
}

function unlock-file {
    TMP_NAME=`mktemp -u`
    TMP_DIR=`dirname $TMP_NAME`
    LOCK_NAME="$TMP_DIR/$1"
    rm -f $LOCK_NAME
    LOCK_FILES="${LOCK_FILES//$LOCK_NAME/}"
}

if ! lock-file simu-BuiLdInG.lock; then
    if ! lock-file simu-WaitInG.lock; then
        # echo Already running and waiting
        exit 0
    fi
    # echo -n Waiting
    while ! lock-file simu-BuiLdInG.lock; do
        # echo -n .
        sleep 0.5
    done
    # echo
    unlock-file simu-WaitInG.lock
fi

reset

echo Building C/C++ sources

$CLANG $CLANG_FLAGS main.cpp -o model.wasm
#$OPT $OPT_FLAGS model-unopt.wasm -o model.wasm
#cp model-unopt.wasm model.wasm

echo Done
