#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/../.." || die "wrong directory"
[ -f test/ubo ] || die "wrong directory"

testing "ubo offsets"; (
    ./test/ubo >/dev/null 2>&1 || fail
) && pass || set_failed
