#!/bin/sh
printf '\33[1;92mCompiling...\33[m\n'

if make devbuild; then
    printf '\33[1;92mRunning program...\33[m\n'
    ./devbuild "$@"
else
    printf '\33[1;91mCompilation failed! Too bad!!!\33[m\n'
fi

