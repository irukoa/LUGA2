#!/bin/bash
File="$1"
Dir="$2"
shift 2
gcc -x c -MM -MP -I. $@ $File |     # Get dependencies,
sed "1s|^|$Dir/|" |                 # append Dir at the beginning, and
sed '1s|^\([^:]*\)\.o:|\1.d \1.o:|' # add a .d entry.
