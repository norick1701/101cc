#!/bin/bash
try() {
    expected="$1"
    input="$2"

    ./101cc "$input" > tmp.s
    ##cat tmp.s
    gcc -o tmp tmp.s
    ./tmp
    actual="$?"
    if [ "$actual" != "$expected" ]; then
        echo "$input expected, but got $actual"
        exit 1
    fi
}

try   0  0
try  42 42
try  21 ' 5  + 20 - 4'
try  41 ' 12 + 34 - 5 '
try   5 ' 1  + 2  + 3 + 4 - 5 '
try 255 ' 10 + 20 + 30 + 40 + 205 - 50 '
try  49 ' 7 + 6 * 7'
try  15 ' 5 * ( 9 - 6 ) '
try   4 ' ( 3 + 5 ) / 2 '
try  21 ' ( 1 + 2 ) *( 3 + 4 ) '
try  14 ' 2 * ( 3 + 4 ) '

echo OK

