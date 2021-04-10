#!/bin/bash

. "$(dirname "$0")/util.sh"

cd "$(dirname "$0")/.." || die "wrong directory"
[ -f jpeg_r ] || die "wrong directory"
mkdir -p out

assets tux.jpeg gray_tux.jpeg cmyk_tux.jpeg || die "assets"

for jpeg in assets/*tux.jpeg; do
    do_test "$(basename "$jpeg")" <<END
        ./jpeg_r "$jpeg" || fail "jpeg_r"
END
done

exit "$code"
