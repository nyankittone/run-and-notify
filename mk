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

# This function needs refactoring so that it will not run the thing itself, but instead take in the
# exit code of a thing.
maybe_die() {
    eval "$@"
    exit_code=$?
    if [ $exit_code != 0 ]; then
        die 1 Compiler process failed with code $exit_code
    fi

    unset exit_code
}

# $1 represents what exit code the program should exit with.
# The remaining arguments passed represents the exit message.
die() {
    exit_code="$1"
    shift

    log_head r FATAL && log "$@"
    exit "$exit_code"
}

# logging format
# [mk/FATAL]: Fuck you.

log_head() {
    output="$(printf '\33[1;97m[\33[93m%s' "$mk_name")"

    while [ -n "${1+deez}" ]; do
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
        if [ -z "${1+deez}" ]; then
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
    for thing_to_delete in "$bin_name" "${bin_name}.o" "$dev_bin_name" "${dev_bin_name}.o" \
        "$obj_dir" "$dev_obj_dir" "$unit_test_bin_dir" "$dev_unit_test_bin_dir"
    do
        if rm -r "$thing_to_delete" 2>/dev/null; then
            log_head b clean && log Removed "$thing_to_delete"
        fi
    done

    log_head b clean && log All cleaned up! :3
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

show_and_run() {
    log "$@"
    eval "$@"
}

# $1 is any extra flags to pass to cc
# $2 is the directory to use for objects
# $3 is the name of the final executeable
build() {
    # create object directory if it doesn't exist
    (mkdir "$2" 2>/dev/null) && log_head b build && log Created directory '"'"$2"'"'; true
    log_head b build && log Compiling app...

    ls -A1 "$source_dir"/*.c | while read -r file; do
        base_file="`basename "$file" | sed 's/..$//'`"

        if needs_rebuild "$2"/"$base_file".o "$file"; then
            maybe_die log_head b build b cmd && show_and_run "$c_compiler" $c_flags $1 -c "$file" -o "$2"/"$base_file".o # buggy?
            rebuilt=
        fi
    done

    # create final executeable object file
    if needs_rebuild "$3".o main.c; then
        maybe_die log_head b build b cmd && show_and_run "$c_compiler" $c_flags $1 -c main.c -o "$3".o
        rebuilt=
    fi

    # create final binary
    # BUG: This will break if our object files contain spaces in them. This should be fixed.
    if needs_rebuild "$3" "$3".o $(ls -A1 "$2"/*.o); then
        maybe_die log_head b build b cmd && show_and_run "$c_compiler" $c_flags $1 "$3".o $(ls -A1 "$2"/*.o) -o "$3"
        rebuilt=
    fi

    [ -z "${rebuilt+deez}" ] && log_head b build && log Nothing to build.; true
    unset rebuilt
}

# $1 is any extra flags to pass to cc
# $2 is the name of the source object files
# $3 is the name of the destination binaries
build_tests() {
    # create unit test dir
    (mkdir "$3" 2>/dev/null) && log_head b buildtests && log Created directory '"'"$3"'"'; true
    log_head b buildtests && log Compiling unit tests...

    ls -A "$unit_test_dir"/*.c | while read -r file; do
        things=$(sed '/^\s*$/d;s/^/'"$2"'\//;s/$/.o/' "`printf %s "$file" | sed 's/.$/txt/'`")

        # BUG: More space-sensitive code here... blehhhghh....,
        # BUG: Why is this re-compiling the thing every time???? ANNOYING!
        if needs_rebuild "$3".c $things; then
            maybe_die "log_head b buildtests b cmd && show_and_run "$c_compiler" $c_flags $1 $things "$file" -o "$3"/"`basename "$file" | sed 's/..$//'`""
        fi

        unset things
    done
}

run() {
    log_head b run && log Running program...
    ./"$dev_bin_name" "$@"
}

test() {
    log_head b test && log Running tests...
    ./run_tests.py "$@"
}

main() {
    case "$1" in
        build)
            build "$release_flags" "$obj_dir" "$bin_name"
            build_tests "$dev_flags" "$obj_dir" "$unit_test_bin_dir"
            test "$unit_test_bin_dir" "bin_name"
        ;;
        dev)
            build "$dev_flags" "$dev_obj_dir" "$dev_bin_name"
        ;;
        clean)
            clean
        ;;
        run)
            shift
            main dev
            run "$@"
        ;;
        buildtests)
            main dev
            build_tests "$dev_flags" "$dev_obj_dir" "$dev_unit_test_bin_dir"
        ;;
        test)
            main buildtests
            test "$dev_unit_test_bin_dir" "$dev_bin_name"
        ;;
        '')
            if [ -z "${1+deez}" ]; then
                main build
            else
                die 5 Invalid command \""$1"\".
            fi
        ;;
        *)
            die 5 Invalid command \""$1"\".
        ;;
    esac
}

wd="`dirname "$0"`"
cd "$wd" || die 1 Cannot cd into right diectory! I give up!
unset wd

main "$@"

