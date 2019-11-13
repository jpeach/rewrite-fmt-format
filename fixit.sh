#! /usr/bin/env bash

set -o nounset
set -o errexit
set -o pipefail

readonly HERE="$(cd $(dirname $0) && pwd)"
readonly FIXES="$HERE/fixes"

mkdir -p $FIXES
rm -f $FIXES/*

has::format() {
    grep -q fmt::format "$1"
}

clang::fixit() {
    local f="$1"
    local out="$(mktemp $FIXES/$(basename $f).XXXX)"

    mv $out "$out.yaml"
    out="$out.yaml"

    # NOTE(jpeach): we hack around the following error emitted by a
    # custom clang build in the preprocessor:
    # 
    # /.../clang/11.0.0/include/xmmintrin.h:1951:3: error: use of undeclared identifier '__builtin_ia32_storelps' [clang-diagnostic-error]
    #  __builtin_ia32_storelps((__v2si *)__p, (__v4sf)__a);

    $HERE/clang-extra-args.sh \
        $HERE/clang-tidy \
        -extra-arg-before="-D__builtin_ia32_storehps(a,b)" \
        -extra-arg-before="-D__builtin_ia32_storelps(a,b)" \
        --export-fixes=$out \
        --checks=-*,abseil-fmt-format-conversion \
        $f

    sed -i '' '-e/^$/d' $out
}

for target in "$@" ; do
    if ! has::format $target ; then
        echo skipping $target
    else
        echo fixing $target
        git checkout $target
        clang::fixit $target
    fi
done

$HERE/clang-apply-replacements $FIXES
