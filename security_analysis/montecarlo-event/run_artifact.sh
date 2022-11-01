#!/bin/bash
## 0. Get Traces Ready
echo "---------------------------"
echo ""
echo "#####################"
echo "1. Compiling Event-driven Monte Carlo Code"
echo "#####################"
echo ""
make clean; make;

# 2. Running Simulations for Row Hammer Threshold of 4800
echo "---------------------------"
echo ""
echo "#####################"
echo "2. Running RH Threshold of 4800"
echo "#####################"
echo ""
cd simscript
./run_exp_4800.sh
echo "RH Threshold of 4800 Simulations Completed!"

# 3. Running Simulations for Row Hammer Threshold of 2400
echo "---------------------------"
echo ""
echo "#####################"
echo "3. Running RH Threshold of 2400"
echo "#####################"
echo ""
./run_exp_2400.sh
echo "RH Threshold of 2400 Simulations Completed!"

# 4. Running Simulations for Row Hammer Threshold of 1200
echo "---------------------------"
echo ""
echo "#####################"
echo "4. Running RH Threshold of 1200"
echo "#####################"
echo ""
./run_exp_1200.sh
echo "RH Threshold of 1200 Simulations Completed!"

## 5. Collate Results
echo "---------------------------"
echo ""
echo "###################"
echo "5. Collating Results"
echo "###################"
echo ""
python3 get_results_4800.py
python3 get_results_2400.py
python3 get_results_1200.py
cd ../ ;
echo "Collating Results Completed!"

## 6. Generating Figure 6
echo "---------------------------"
echo ""
echo "###################"
echo "6. Generating Figure 6"
echo "###################"
echo ""
cd graph/
python3 figure6.py 
echo "Generating Figure 6 Completed!"