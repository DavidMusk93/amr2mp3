#!/bin/bash

IsZero() { return $1; }

OnInit()
{
  ROOT_PATH=$PWD
}

GetProp()
{
  local REGEX='[[:blank:]]*:?=[[:blank:]]*(.*)$'
  sed -n -E "s/$1$REGEX/\1/p" ./Makefile
}

FormatCode()
{
  local f
  for f in `find ${1-$ROOT_PATH} -type f | egrep '\.h$|\.c$|\.cc$|\.cpp$'`; do
    sed -i -e 's/$//g' -e 's/\t/  /g' -e 's/\s\+$//g' $f
  done
}

OnFinish()
{
  local mode from to
  TARGETS=(~/AndroidStudioProjects/XposedTest/app/src/main/jniLibs/arm64-v8a /data/local/tmp)
  mode=`GetProp TEST_MODE`
  from=./libs/arm64-v8a/*
  to=${TARGETS[$mode]}
  case $mode in
    0)
      cp -f $from $to;;
    1)
      adb push $from $to;;
  esac
}

main()
{
  OnInit
  case $1 in
    0|format)
      FormatCode $2;;
    1|finish)
      OnFinish;;
  esac
}
# set -x
main $*
