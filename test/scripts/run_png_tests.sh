#!/bin/bash
if [[ "$_" != "$0" ]]
then
    echo "FATAL: do not source this file !"
else
    [[ "${PWD##*/}" == "scripts" ]] && cd ..
    if [[ "${PWD##*/}" != "test" ]]; then
	if [[ -d "test" ]]; then
	    cd test
	else
	    echo "Error: This must be run from the 'test/' directory" 1>&2
	    exit 2
	fi
    fi
    export PATH=./scripts/:./main:$PATH

    mkdir -p out/
    error=0
    for png in $(find "assets/" -type f -name "*.png")
    do
	echo -n "testing $png ... "
	$(exec png_rw "$png" "out/${png##*/}")
	if [ $? -ne 0 ]; then
	    echo "png_rw exited with error"
	else
	    if cmp_img.sh "$png" "out/${png##*/}"; then
            echo "PASSED"
        else
            echo "FAIL"
            error=1
        fi
	fi
    done
fi
exit $error
