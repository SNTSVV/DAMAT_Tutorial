#!/bin/bash

#
# Copyright (c) University of Luxembourg 2020.
# Created by Enrico VIGANO, enrico.vigano@uni.lu, SnT, 2021.
#

echo "************************* PROBE GENERATION ************************"


DAMAt_FOLDER=$(pwd)

. $DAMAt_FOLDER/DAMAt_configure.sh

# _FAQAS_SINGLETON_FM="TRUE" can be exported to load the fault model in a singleton variable to save memory
if [ $singleton == "TRUE" ]; then
export _FAQAS_SINGLETON_FM=$singleton
echo "$(tput setaf 2)************************* SINGLETON MODE **************************$(tput sgr0)"
fi

export _FAQAS_INITIAL_PADDING=$padding
###############################################################################

# STEP 2 MUTATION API GENERATION
pushd $DAMAt_FOLDER

python3 generateDataMutator.py "$buffer_type" "$fault_model"
popd

###############################################################################
