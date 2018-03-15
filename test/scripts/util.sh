die() {
    printf '[%s] \e[1;31mError: \e[0;31m%s\e[0m\n' "$(basename "$0" .sh)" "$1"
    exit 1
}

testing() {
    printf '[%s] Testing %s... ' "$(basename "$0" .sh)" "$1"
}

fail() {
    printf '\e[1;31mFAIL \e[0;31m%s\e[0m\n' "$1"
    exit 1
}

pass() {
    printf '\e[1;32mPASS\e[0m\n'
}
