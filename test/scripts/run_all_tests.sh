#!/bin/bash
[[ "${PWD##*/}" == "scripts" ]] && cd ..
if [[ "${PWD##*/}" != "test" ]]; then
    if [[ -d "test" ]]; then
	cd test
    else
	echo "Error: This must be run from the 'test/' directory" 1>&2
	exit 2
    fi
fi
error=0
./scripts/run_png_tests.sh || error=1
exit $error
