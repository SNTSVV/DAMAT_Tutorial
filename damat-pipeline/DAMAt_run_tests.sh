#!/bin/bash
#
# Copyright (c) University of Luxembourg 2020.
# Created by Oscar Eduardo CORNEJO OLIVARES, oscar.cornejo@uni.lu, SnT, 2020.
# Modified by Enrico VIGANO, enrico.vigano@uni.lu, SnT, 2021.
#
deco="$(tput setaf 4)*******************************************************************$(tput sgr0)"

echo "$(tput setaf 4)************************ RUN TESTS ********************************$(tput sgr0)"

mutant_id=$1
tests_list=$2
DAMAT_FOLDER=$3
results_dir=$DAMAT_FOLDER/results

mutant_dir=$results_dir/run_"$mutant_id"
execution_log=$mutant_dir/"$mutant_id"_execution.out
coverage_file=$mutant_dir/"$mutant_id"_coverage.csv
results_file=$mutant_dir/main.csv

###############################################################################
#date in milliseconds
start_time=$(($(date +%s%N)/1000000))
mkdir -p $results_dir
mkdir -p $mutant_dir
touch $execution_log
touch $coverage_file
touch $results_file
export FAQAS_COVERAGE_FILE=$coverage_file
export _FAQAS_SINGLETON_FM
###############################################################################

while IFS="," read -r p || [ -n "$p" ];do

    mutant_start_time=$(($(date +%s%N)/1000000))

    # obtaining test number to be executed
    read tst <<< $(awk -F',' '{print $1}' <<< "$p")

    # obtaining corresponding timeout for the test case
    TIMEOUT=$(echo "$p" | awk -F',' '{$2=($2*4)/1000; printf("%.0f\n", $2);}')

    echo "$deco"
    echo "$(tput setaf 3) Running mutant $mutant_id against test case $tst $(tput sgr0)"
    echo "$deco"
    echo -n "${mutant_id};COMPILED;${tst};" >> $results_file


###############################################################################
# here the user shall call the execution of the current test case,

  pushd /home/vagrant/libcsp_workspace/test_suite

  echo "$deco" 2>&1 | tee -a $execution_log
  echo "$(tput setaf 1) RUNNING THE TEST NOW! $(tput sgr0)" 2>&1 | tee -a $execution_log
  echo "$deco" 2>&1 | tee -a $execution_log

  # timeout 30 ./build/examples/csp_server_client -t
  make clean
  make $tst

  EXEC_RET_CODE=$?

  echo "$deco" 2>&1 | tee -a $execution_log
  echo "$(tput setaf 2) FINITO! $(tput sgr0)" 2>&1 | tee -a $execution_log
  echo "$deco" 2>&1 | tee -a $execution_log

  popd

###############################################################################
#the exec return code should be 0 if the test case passes, 1 if the test case fails and 124 in case of a timeout


  mutant_end_time=$(($(date +%s%N)/1000000))
  mutant_elapsed="$(($mutant_end_time-$mutant_start_time))"



  if [ $EXEC_RET_CODE -ge 124 ]; then
    echo "$deco" 2>&1 | tee -a $execution_log
    echo "Test return code: [$EXEC_RET_CODE]" 2>&1 | tee -a $execution_log
    echo "$(tput setaf 1) Mutant timeout by $tst $(tput sgr0)" 2>&1 | tee -a $execution_log
    echo -ne "TIMEOUT;KILLED_${EXEC_RET_CODE};${mutant_elapsed}\n" >> $results_file
    echo "$deco"
  else
      if [ $EXEC_RET_CODE -eq 0 ]; then
        echo "$deco" 2>&1 | tee -a $execution_log
        echo "Test return code: [$EXEC_RET_CODE]" 2>&1 | tee -a $execution_log
        echo "$(tput setaf 2) live mutant $(tput sgr0)" 2>&1 | tee -a $execution_log
        echo -ne "PASSED;LIVE;${mutant_elapsed}\n" >> $results_file
        echo "$deco"

      else
        echo "$deco"
        echo "Test return code: [$EXEC_RET_CODE]" 2>&1 | tee -a $execution_log
        echo "$(tput setaf 1) Mutant killed by $tst $(tput sgr0)" 2>&1 | tee -a $execution_log
        echo -ne "FAILED;KILLED;${mutant_elapsed}\n" >> $results_file
        echo "$deco"

      fi
  fi


###############################################################################

	#create a different coverage file for every test

	NEW_COVERAGE_FILE=$mutant_dir/coverage_"$tst".csv
	cp $coverage_file $NEW_COVERAGE_FILE
	>$FAQAS_COVERAGE_FILE


# # break in case the mutant is killed
#   if [ $EXEC_RET_CODE -ge 124 ]; then
#     echo "$deco"
#   else
#       if [ $EXEC_RET_CODE -eq 0 ]; then
#         echo "$deco"
#       else
#         echo "$deco"
#         break
#       fi
#   fi

done < $tests_list

rm -rf ~/Obsw/Test/System/testresults/*

end_time=$(($(date +%s%N)/1000000))
elapsed="$(($end_time-$start_time))"

echo "elapsed time $elapsed [ms]"
