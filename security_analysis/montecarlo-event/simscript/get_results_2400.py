#!/bin/python

import re
import os
import sys
import subprocess
import glob



rh_threshold='2400'

output_file='./../results/aggregate_trh_'+rh_threshold+'.csv'

global num_instructions

workload_list = open("input/rh_2400_list.txt","rt")
result= open(output_file,'w')

result.write("Workloads, Iterations Number, Attack Time (Days), Attack Time (Seconds)\n")

while True:
    workload = workload_list.readline()
    if not workload : break
    workload_temp = workload.split()
    workload_name= rh_threshold + '_' + workload_temp[0]+'.out'
    result.write(workload_name+', ')
    temp = open('./../results/trh_2400/'+workload_name,"r")
    if temp:
        while True:
            line = temp.readline()
            if not line :
                result.write('\n')
                break
            # Find Simulation Iterations here
            iter_find = re.findall(r'Simulation iterations',line)
            if iter_find:
                iter_temp = line.split()
                # print(iter_temp)
                sim_iter = iter_temp[2]
                result.write(sim_iter+', ')
            # Find average attack time here
            average_attack_time_find = re.findall(r'Final Average Attack', line)
            if average_attack_time_find:
                average_attack_time_list = line.split()
                # print(average_attack_time_list)
                average_attack_time_temp = average_attack_time_list[4]
                average_attack_time = average_attack_time_temp.split('s')
                result.write(average_attack_time[0]+', ')
                average_attack_time_day = float(average_attack_time[0])/3600/24
                result.write(str(average_attack_time_day))
    temp.close()
workload_list.close()
result.close()


