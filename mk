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

mk_name=mk

# $1 represents what exit code the program should exit with.
# The remaining arguments passed represents the exit message.
die() {
    exit_code="$1"
    shift

    printf '\33[1;91mfatal!\33[m %s\n' "$@"
    exit "$exit_code"
}

# logging format
# [mk/FATAL]: Fuck you.

log_head() {
    output="$(printf '\33[1;97m[\33[93m%s' "$mk_name")"

    while [ -n "$1" ]; do
        case "$1" in
            w) code='\33[1;97m';;
            y) code='\33[1;93m';;
            g) code='\33[1;92m';;
            r) code='\33[1;91m';;
            b) code='\33[1;96m';;
            n) code='\33[m';;
            *)
                unset output
                return 1
            ;;
        esac

        shift
        if [ -z "$1" ]; then
            unset code
            unset output
            return 2
        fi

        output="$output""$(printf '\33[1;97m/'"$code"'%s' "$1")"
        unset code

        shift
    done

    printf '%s\33[1;97m]\33[m ' "$output" 1>&2
    unset output
}

# $@ is the log message
log() {
    printf '%s\n' "`echo "$@" | tr '\n' ' '`" 1>&2
}

info() {
    log_head n info && log "$@"
}

clean() {
    rm -r "$bin_name" "${bin_name}.o" "$dev_bin_name" "${dev_bin_name}.o" "$obj_dir" \
        "$dev_obj_dir" "$unit_test_bin_dir" "$dev_unit_test_bin_dir" 2>/dev/null; true
}

# $1 is the object file to re-build
# remaining parameters are dependencies of $1
# 0 is returned if rebuild needed, else 1
# 2 is returned if called incorrectly
needs_rebuild() {
    [ -z "$1" ] && return 2
    [ -f "$1" ] || return 0

    dest_date="`date -r "$1" +%s`"
    shift

    while [ -n "${1+deez}" ]; do
        if [ -f "$1" ]; then
            shift
            continue
        fi

        if [ "`date -r "$1" +%s`" -gt "$dest_date" ]; then
            unset dest_date
            return 0
        fi

        shift
    done

    unset dest_date
    return 1
}

# $1 is any extra flags to pass to cc
# $2 is the directory to use for objects
# $3 is the name of the final executeable
build() {
    # check to see if main.c needs rebuilding
    ## this means we need to start with each of the files in src, and check if those need
    ## to be recompiled...
    ### How do we handle header files? (how about, for now, we don't?)

    # create object directory if it doesn't exist
    (mkdir "$2" 2>/dev/null) && log_head b build && log Created directory '"'"$2"'"'.; true

    ls -A1 "$source_dir"/*.c | while read -r file; do
        base_file="`basename "$file" | sed 's/..$//'`"

        if needs_rebuild "$2"/"$base_file".o "$file"; then
            "$c_compiler" $c_flags $1 -c "$file" -o "$2"/"$base_file".o # buggy?
        fi
    done

    # create final executeable object file
    if needs_rebuild "$3".o main.c; then
        "$c_compiler" $c_flags $1 -c main.c -o "$3".o
    fi

    # create final binary
    if needs_rebuild "$3" "$3".o $(ls -A1 "$2"/*.o); then
        "$c_compiler" $c_flags $1 "$3".o $(ls -A1 "$2"/*.o) -o "$3"
    fi
}

main() {
    case "$1" in
        build)
            build "$release_flags" "$obj_dir" "$bin_name"
        ;;
        dev)
            build "$dev_flags" "$dev_obj_dir" "$dev_bin_name"
        ;;
        clean)
            clean
        ;;
        *)
            build "$release_flags" "$obj_dir" "$bin_name"
        ;;
    esac
}

wd="`dirname "$0"`"
cd "$wd" || die 1 Cannot cd into right diectory! I give up!
unset wd

main "$@"

