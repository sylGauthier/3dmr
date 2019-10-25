#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/../.." || die "wrong directory"
[ -f test/ubo ] || die "wrong directory"

do_test "ubo offsets" <<END
    ./test/ubo >/dev/null 2>&1 || fail
END
