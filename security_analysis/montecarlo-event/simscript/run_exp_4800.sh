#!/bin/bash
mkdir -p ../results/trh_4800/

while read LINE; do
        ATTACK_ROUNDS=`echo ${LINE} | cut -d ' ' -f1`
        DETERMINISTIC_ACCESS_NUM=`echo ${LINE} | cut -d ' ' -f2`
        RANDOM_GUESS_NUM=`echo ${LINE} | cut -d ' ' -f3`
        RH_PROBABILITY=`echo ${LINE} | cut -d ' ' -f4`
        FILENAME="4800_${ATTACK_ROUNDS}.out"
        ./../bin/run 4800 6 $DETERMINISTIC_ACCESS_NUM $RANDOM_GUESS_NUM $RH_PROBABILITY 100 > ../results/trh_4800/${FILENAME}
done < input/list_trh_4800.txt