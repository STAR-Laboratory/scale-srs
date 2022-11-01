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
./runall_baseline.sh
cd ../ ;

## wait for baseline runs to finish
i=0
while [ `ps -aux | grep usimm.bin | grep -v "grep" | wc -l` -gt 0 ] ; do
    num_running=`ps -aux | grep usimm.bin | grep -v "grep" | wc -l`
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
./runall_srs1K.sh
cd ../ ;

## wait for baseline runs to finish
i=0
while [ `ps -aux | grep usimm.bin | grep -v "grep" | wc -l` -gt 0 ] ; do
    num_running=`ps -aux | grep usimm.bin | grep -v "grep" | wc -l`
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
./runall_srs1K.sh
cd ../ ;

## wait for RRS runs to finish
i=0
while [ `ps -aux | grep usimm.bin | grep -v "grep" | wc -l` -gt 0 ] ; do
    num_running=`ps -aux | grep usimm.bin | grep -v "grep" | wc -l`
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

# Normalized performance for workloads in the left half of Fig 14, i.e., workloads with at least one row having > 800 activations / 64ms 
./getdata.pl -s ADDED_IPC -w interest_name -n 0 -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/

# Normalized Perf. of RRS (Gmean) for SPEC-2006, SPEC-2017, GAP, PARSEC, BIOBENCH, COMM, MIX, ALL-78     
echo ""     
./getdata.pl -s ADDED_IPC -w spec2006_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ | sed 's/Gmean/SPEC2K6-29/'  | tail -n1 
./getdata.pl -s ADDED_IPC -w spec2017_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ | sed 's/Gmean/SPEC2K17-22/' | tail -n1 
./getdata.pl -s ADDED_IPC -w gap_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/      | sed 's/Gmean/GAP-6/'	      | tail -n1 
./getdata.pl -s ADDED_IPC -w comm_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/     | sed 's/Gmean/COMM-5/'      | tail -n1
./getdata.pl -s ADDED_IPC -w parsec_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/   | sed 's/Gmean/PARSEC-5/'    | tail -n1    
./getdata.pl -s ADDED_IPC -w biobench_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/ | sed 's/Gmean/BIOBENCH-2/'  | tail -n1     
./getdata.pl -s ADDED_IPC -w mix_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/      | sed 's/Gmean/MIX-6/'	      | tail -n1
./getdata.pl -s ADDED_IPC -w all78 -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/         | sed 's/Gmean/ALL-78/'      | tail -n1 
