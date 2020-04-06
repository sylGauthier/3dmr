code=0

die() {
    printf '[%s] \e[1;31mError: \e[0;31m%s\e[0m\n' "$(basename "$0" .sh)" "$1"
    exit 1
}

testing() {
    if [ -n "$TEST_VERBOSE" ]; then
        printf '\e[1m[%s] \e[36mTesting %s...\e[0m\n' "$(basename "$0" .sh)" "$1"
    else
        printf '[%s] Testing %s... ' "$(basename "$0" .sh)" "$1"
    fi
}

fail() {
    if [ -n "$TEST_VERBOSE" ]; then
        printf '\e[1m[%s] \e[31mFAIL \e[0;31m%s\e[0m\n' "$(basename "$0" .sh)" "$1" >&3
    else
        printf '\e[1;31mFAIL \e[0;31m%s\e[0m\n' "$1" >&3
    fi
    exit 1
}

pass() {
    if [ -n "$TEST_VERBOSE" ]; then
        printf '\e[1m[%s] \e[32mPASS\e[0m\n' "$(basename "$0" .sh)" >&3
    else
        printf '\e[1;32mPASS\e[0m\n' >&3
    fi
    true
}

assets() {
    mkdir -p assets && make --no-print-directory -s -C assets -f ../assets.mk "$@"
}

do_test() {
    testing "$1"
    if [ -n "$TEST_VERBOSE" ]; then
        (. /dev/stdin) 3>&1 && pass 3>&1 || code=1
    else
        (. /dev/stdin) 3>&1 1>/dev/null 2>/dev/null && pass 3>&1 || code=1
    fi
}
