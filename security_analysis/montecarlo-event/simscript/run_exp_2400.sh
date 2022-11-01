#!/bin/bash
mkdir -p ../results/trh_2400/

while read LINE; do
        ATTACK_ROUNDS=`echo ${LINE} | cut -d ' ' -f1`
        DETERMINISTIC_ACCESS_NUM=`echo ${LINE} | cut -d ' ' -f2`
        RANDOM_GUESS_NUM=`echo ${LINE} | cut -d ' ' -f3`
        RH_PROBABILITY=`echo ${LINE} | cut -d ' ' -f4`
        FILENAME="2400_${ATTACK_ROUNDS}.out"
        ./../bin/run 2400 6 $DETERMINISTIC_ACCESS_NUM $RANDOM_GUESS_NUM $RH_PROBABILITY 100 > ./../results/trh_2400/${FILENAME}
done < input/list_trh_2400.txt