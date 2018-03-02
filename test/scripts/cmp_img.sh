#!/bin/bash
for arg in "$@"
do
case $arg in
    -h|-?|-help|--help)
	echo "Usage: $0 <image> <image>"
	echo "return 0 if both images are the same, else 1."
	exit 64
    ;;
esac
done
if [[ ! "$#" -eq "2" ]]; then
    echo "Usage: $0 [--help] <image> <image>" 1>&2
    exit 64
fi
if [[ ! -f "$1" ]]; then
    echo "File not found: $1" 1>&2
    exit 2
fi
if [ ! -f "$2" ]; then
    echo "File not found: $2" 1>&2
    exit 2
fi

diff=`compare -metric AE "$1" "$2" -compose difference /dev/null 2>&1`
[ "$diff" = "0" ]
