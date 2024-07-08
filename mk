#!/bin/sh

# This script automatically builds, runs, and runst tests for this program.
# You moght be like, "why not use make?". Good question. Put simply, makefiles scare me, and I
# don't want to deal with them to be frank. So here I am, re-inventing make for my specific needs.

bin_name=run-and-notify
dev_bin_name=devbuild
source_dir=src
obj_dir=obj
dev_obj_dir=dev-obj
unit_test_dir=unit-tests
unit_test_bin_dir=unit-test-bin
dev_unit_test_bin_dir=dev-unit-test-bin

include_dirs='-Iinclude -I/usr/include/gdk-pixbuf-2.0 -I/usr/include/glib-2.0 -I/usr/lib/glib-2.0/include -I/usr/include/libpng16 -I/usr/include/libmount -I/usr/include/blkid -I/usr/include/sysprof-6'
libs='-lnotify -lgdk_pixbuf-2.0 -lgio-2.0 -lgobject-2.0 -lglib-2.0'

c_compiler=cc
c_flags='-std=c99 -lc -pthread '"$libs"' -pedantic-errors -Wall '"$include_dirs"
release_flags='-O3'
dev_flags='-Og -g'
workers=1

# $1 represents what exit code the program should exit with.
# The remaining arguments passed represents the exit message.
die() {
    exit_code="$1"
    shift

    printf '\33[1;91mfatal!\33[m %s\n' "$@"
    exit "$exit_code"
}

info() {
    printf '\33[1;93m%s\33[m\n' "$@"
}

success() {
    printf '\33[1;92m%s\33[m\n' "`echo "$@" | tr '\n' ' '`"
}

clean() {
    rm -r "$bin_name" "${bin_name}.o" "$dev_bin_name" "${dev_bin_name}.o" "$obj_dir" \
        "$dev_obj_dir" "$unit_test_bin_dir" "$dev_unit_test_bin_dir" 2>/dev/null
    success Cleaned up!
}

main() {
    case "$1" in
        build)
            shift
            build
        ;;
        dev)
            shift
            build
        ;;
        clean)
            shift
            clean
        ;;
        *)
            shift
            build
        ;;
    esac
}

wd="`dirname "$0"`"
cd "$wd" || die 1 Cannot cd into right diectory! I give up!
unset wd

main "$@"

