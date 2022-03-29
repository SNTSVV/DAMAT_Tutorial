#!/bin/bash
#
# Copyright (c) University of Luxembourg 2020.
# Created by Oscar Eduardo CORNEJO OLIVARES, oscar.cornejo@uni.lu, SnT, 2020.
# Modified by Enrico VIGANO, enrico.vigano@uni.lu, SnT, 2021.
#
deco="$(tput setaf 4)*******************************************************************$(tput sgr0)"

echo "$(tput setaf 4)************************* COMPILE *********************************$(tput sgr0)"

mutant_id=$1
singleton=$2

###############################################################################
#enabling extended pattern matching features:
shopt -s
#options for enabling aliases:
shopt -s expand_aliases
###############################################################################


echo "$deco"
echo "Mutant opt: "$mutant_id
echo "$deco"

###############################################################################

# exporting the operation counter
export MUTATIONOPT=$mutant_id

if [ $singleton == "TRUE" ]; then
  export _FAQAS_SINGLETON_FM=$singleton
  EXTRA_FLAGS_SINGL="--singleton TRUE"
fi

# here the user must invoke the compilation of the SUT, we provided a simple example.

compilation_folder="/home/vagrant/libcsp_workspace/libcsp"

pushd $compilation_folder

echo "$deco"
echo "compiling test"
echo "$deco"

./waf distclean configure build --mutation-opt $mutant_id $EXTRA_FLAGS_SINGL --with-os=posix --enable-rdp --enable-promisc --enable-hmac --enable-dedup --enable-can-socketcan --with-driver-usart=linux --enable-if-zmqhub #--enable-examples

echo "$deco"
if [ $? -eq 0 ]; then
    echo $x "$(tput setaf 2) compilation OK $(tput sgr0)"
else
    echo $x "$(tput setaf 1) compilation FAILED $(tput sgr0)"
fi
echo "$deco"
popd
