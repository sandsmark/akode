#!/bin/sh

usage()
{
	echo "usage: $0 [OPTIONS]"
cat << EOH

options:
	[--libs]
	[--cflags]
	[--version]
	[--prefix]
EOH
	exit 1;
}

prefix=${CMAKE_INSTALL_PREFIX}
exec_prefix=${CMAKE_INSTALL_PREFIX}
libdir=${LIB_INSTALL_DIR}
includedir=${INCLUDE_INSTALL_DIR}

flags=""

if test $# -eq 0 ; then
  usage
fi

while test $# -gt 0
do
  case $1 in
    --libs)
	  flags="$flags -L$libdir -lakode"
	  ;;
    --cflags)
	  flags="$flags -I$includedir"
	  ;;
    --version)
	  echo 2.0.0
	  ;;
    --prefix)
	  echo $prefix
	  ;;
	*)
	  echo "$0: unknown option $1"
	  echo
	  usage
	  ;;
  esac
  shift
done

if test -n "$flags"
then
  echo $flags
fi
