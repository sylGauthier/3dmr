#!/bin/bash

usage() {
    printf 'Usage: %s [-h] [-p] [-t threshold] <image> <image>\n' "$0"
    printf 'return 0 if both images are the same, else 1.\n\n'
    printf ' -t threshold: set the error threshold under which\n    the images are considered to be the same\n'
    printf ' -p: print error\n'
}

threshold=0
while [ "$#" -gt 0 ]; do
    case "$1" in
        -h|-help|--help)
            usage; exit 0
            ;;
        -t|-threshold|--threshold)
            shift
            if [ "$#" -eq 0 ]; then
                usage >&2; exit 1
            fi
            threshold="$1"
            ;;
        -p|-print|--print)
            print="yes"
            ;;
        *)
            break
            ;;
    esac
    shift
done
if [ "$#" -ne "2" ]; then
    usage >&2; exit 1
fi
[ -f "$1" ] || notfound="$notfound $1"
[ -f "$2" ] || notfound="$notfound $2"
if [ -n "$notfound" ]; then
    printf 'Error: file not found:%s\n' "$notfound" >&2
    exit 2
fi

error="$(dc -e "20k $(compare -colorspace RGB -depth 8 -metric MAE "$1" "$2" null: 2>&1 | cut -d' ' -f1 | sed 's/e\([0-9-]*\)/ 10 \1 ^*/;s/-/_/g') 0k1/p")"
[ -n "$print" ] && printf 'Error: %s\n' "$error"
[ "$error" -le "$threshold" ]
