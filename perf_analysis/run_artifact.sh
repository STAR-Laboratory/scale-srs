#!/bin/bash

##########
## SET FIRE-WIDTH FOR RUNNING WORKLOADS
## (number of processes to run at a time)
firewidth=80
##########

## 0. Get Traces Ready
echo "---------------------------"
echo ""
echo "#####################"
echo "0. Trace Inputs"
echo "#####################"
echo ""
echo "Note: This artifact requires memory-access traces to be already available."
echo "Prepare traces following the instruction" 

# 1. Compile the USIMM
echo "---------------------------"
echo ""
echo "#####################"
echo "1. Compiling USIMM"
echo "#####################"
echo ""
cd src
make clean; make;
cd .. ;

# 2. Running Baseline
echo "---------------------------"
echo ""
echo "#####################"
echo "2. Running Baseline"
echo "#####################"
echo ""
echo "--> Note this fires all baseline sims: ~78 of them --> takes 7-8 hours to complete."
echo ""
cd simscript
./runall_baseline.pl --w 8c_2ch_ALL_78_WL --i ALL_78_WL_name --f $firewidth --d "../output/8c_2ch_baseline" --o "2"
cd ../ ;

## wait for baseline runs to finish
i=0
while [ `ps -aux | grep sim_baseline.bin | grep -v "grep" | wc -l` -gt 0 ] ; do
    num_running=`ps -aux | grep sim_baseline.bin | grep -v "grep" | wc -l`
    mins=$(( 10*i ))
    echo "Time Elapsed: ${mins} minutes. Workloads Running: ${num_running}/78";
    echo "";
    sleep 600s;
    ((i=i+1));
done
echo "Baseline Runs Completed!"

# 3. Running Scale-SRS
echo "---------------------------"
echo ""
echo "#####################"
echo "3. Running Scale-SRS"
echo "#####################"
echo ""
echo "--> Note this fires all baseline sims: ~78 of them --> takes 7-8 hours to complete."
echo ""
cd simscript
./runall_srs1K.pl --w 8c_2ch_ALL_78_WL --i ALL_78_WL_name --f $firewidth --d "../output/8c_2ch_srs1K" --o "2"
cd ../ ;

## wait for baseline runs to finish
i=0
while [ `ps -aux | grep sim_srs1K.bin | grep -v "grep" | wc -l` -gt 0 ] ; do
    num_running=`ps -aux | grep sim_srs1K.bin | grep -v "grep" | wc -l`
    mins=$(( 10*i ))
    echo "Time Elapsed: ${mins} minutes. Workloads Running: ${num_running}/78";
    echo "";
    sleep 600s;
    ((i=i+1));
done
echo "Scale-SRS Runs Completed!"

## 4. Running RRS
echo "---------------------------"
echo ""
echo "###################"
echo "4. Running RRS"
echo "###################"
echo ""
"--> Note this fires all RRS sims: ~78 of them --> takes 7-8 hours to complete."
echo ""
cd simscript
./runall_rrs1K.pl --w 8c_2ch_ALL_78_WL --i ALL_78_WL_name --f $firewidth --d "../output/8c_2ch_rrs1K" --o "2"
cd ../ ;

## wait for RRS runs to finish
i=0
while [ `ps -aux | grep sim_rrs1K.bin | grep -v "grep" | wc -l` -gt 0 ] ; do
    num_running=`ps -aux | grep sim_rrs1K.bin | grep -v "grep" | wc -l`
    mins=$(( 10*i ))
    echo "Time Elapsed: ${mins} minutes. Workloads Running: ${num_running}/78";
    echo "";
    sleep 600s;
    ((i=i+1));
done
echo "RRS Runs Completed!"


## 5. Collate Results
echo "---------------------------"
echo ""
echo "###################"
echo "5. Collating Results"
echo "###################"
echo ""

cd simscript
./plot.sh
cd ../ ;
echo "Collating Results Completed!"

## 6. Generating Figure 14
echo "---------------------------"
echo ""
echo "###################"
echo "6. Generating Figure 14"
echo "###################"
echo ""
cd graph/
python3 figure14.py 
echo "Generating Figure 14 Completed!"