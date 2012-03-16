#!/bin/sh
# For every quoted line that doesn't have a newline, add one

sed '/[^n]"$/s@"$@\\n"@' -i $@
