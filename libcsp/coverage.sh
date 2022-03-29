#!/bin/sh
BUILD_DIRECTORY="/home/vagrant/libcsp_workspace/libcsp/build"
EXAMPLES_DIRECTORY="$BUILD_DIRECTORY/examples"
SOURCE_GCOV="$BUILD_DIRECTORY/src"
REAL_SOURCE="/home/vagrant/libcsp_workspace/libcsp/src"

./waf distclean configure build --with-os=posix --enable-rdp --enable-promisc --enable-hmac --enable-dedup --enable-can-socketcan --with-driver-usart=linux --enable-if-zmqhub --enable-examples

./build/examples/csp_server_client

cp $REAL_SOURCE/* $SOURCE_GCOV/

pushd $SOURCE_GCOV
  for i in *.c; do
    [ -f "$i" ] || break
    NAME=$(echo "$i" | awk '{ print substr( $0, 1, length($0)-2 ) }')
    echo $NAME
    OLD_O="$NAME".c.2.o
    OLD_GCNO="$NAME".c.2.gcno
    OLD_GCDA="$NAME".c.2.gcda
    NEW_O="$NAME".o
    NEW_GCNO="$NAME".gcno
    NEW_GCDA="$NAME".gcda
    mv $OLD_O $NEW_O
    mv $OLD_GCNO $NEW_GCNO
    mv $OLD_GCDA $NEW_GCDA
    gcov $i
  done
  for i in *.h; do
    [ -f "$i" ] || break
    echo $i
    gcov $i
  done
popd
