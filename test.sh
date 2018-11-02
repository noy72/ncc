#!/bin/bash
try() {
  expected="$1"
  input="$2"

  gcc -o ncc ncc.c
  ./ncc "$input" > tmp.s
  gcc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" != "$expected" ]; then
    echo "$input expected, but got $actual"
    exit 1
  fi
}

try 0 0
try 42 42
try 21 '5+20-4'
try 41 ' 12 + 34 - 5 '
try 4 ' (1 + 3  )'
try 20 ' (1 + 3  ) * 5'
try 40 ' (1 + 3  ) * 5 * ( 10 / 5) '
try 20 '4 * 10 / 2'
try 0 ' 20 * 2 / (30 + 10) '
# 20 * (2 / (30 + 10)) -> 20 * 0 -> 0
# 優先度が等しい場合は左から処理されている？

echo OK
