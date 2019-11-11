#! /usr/bin/env bash

readonly HERE=$(cd $(dirname $0); pwd)
readonly PROG=${PROG:-${HERE}/rename-fmt-format}

exec $HERE/clang-extra-args.sh $PROG "$@"
