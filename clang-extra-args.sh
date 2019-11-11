#! /usr/bin/env bash

# Extract the builtin clang header search path from the default
# clang compiler. The tool will have the defaults from the LLVM
# build tree, which will not be what we need to compile real code.
cxx::search::dirs() {
	$CXX -E -x c++ - -v < /dev/null 2>&1 | awk '
		$0 ~ /search starts here/ {
			m = 1;
		}

		$0 ~ /End of search/ {
			m = 0;
		}

		m == 1 && $1 ~ /^\//{
			print $1
		}
	'
}

readonly CXX=${CXX:-clang++}
readonly SDKROOT=${SDKROOT:-$(xcrun --show-sdk-path)}

declare -a BUILTINS=()

# Add the default C++ search directories with -isystem to suppress warnings.
for p in $(cxx::search::dirs) ; do
	BUILTINS+=("-extra-arg-before=-isystem" "-extra-arg-before=${p}")
done

readonly PROG="$1"

shift

exec $PROG \
	-extra-arg-before="-isysroot" \
	-extra-arg-before="${SDKROOT}" \
	${BUILTINS[*]} \
	"$@"
