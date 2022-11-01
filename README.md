#  Scalable and Secure Row-Swap: Efficient and Safe Row Hammer Mitigation in Memory Systems (HPCA 2023)
**Authors**: Jeonghyun Woo (UBC), Gururaj Saileshwar (Georgia Tech), and Prashant Nair (UBC)

To appear in [HPCA 2023](https://hpca-conf.org/2023/)

## Introduction
This artifact covers two aspects of the results from the paper: 

* Security analysis of our Juggernaut attack against Randomized Row-Swap (RRS): 

* Performance analysis of our Scalable and Secure Row-Swap (Scale-SRS) and RRS: We provide the C code for the implementation of Scale-SRS and RRS which is encapsulated within the USIMM memory system simulator. The Scale-SRS and RRS structures and operations are implemented within the memory controller module in our artifact. We provide scripts to compile our simulator and run the baseline, Scale-SRS, and RRS for all the workloads we studied in this paper. We also provide scripts to parse the results and collate the performance results shown in Figure 14.

## Requirements for Performance Evaluations in USIMM Memory System Simulator:
* **Software Dependencies**: Perl (for scripts to run experiments and collate results) and gcc (tested to compile successfully with the version: 11.3.0).
* **Hardware Dependencies**: For running all the benchmarks, a CPU with lots of memory (128GB+) and cores (64+).
* **Trace Dependencies**: Our simulator requires traces of memory-accesses for benchmarks (filtered through a L1 and L2 cache). We generate these traces using an Intel Pintool (version 2.12), similar to this pin-tool. However, traces extracted in the format described at the end of the README by any methodology (e.g., any Pin version) would be supported.
## Compiling and Executing BASELINE, Scale-SRS, and RRS

### Clone the artifact and run the code.

* **Fetch the code**: `https://github.com/STAR-Laboratory/scale-srs`
* **Prepare Traces**:  
* **Run the artifact**: `cd rrs; ./run_artifact.sh`. This command runs all the following steps (compile, execute, collate results). You may also follow these steps manually.
* **Note:** Our artifact requires  memory-access traces as input and assumes that they are available in `/input` folder. We provide information on how traces can be generated at the end of the README.md.
 
### Compile

1. Compile USIMM with the following steps from the src folder
         
     	    $ cd scale-srs/perf_analysis/src/
     	    $ make clean
     	    $ make


### Execute

2. Run baseline with the following command from the simscript folder
         
     	    $ cd scale-srs/perf_analysis/simscript/
     	    $ ./runall_baseline.sh
     	    --> Note this command assumes traces files are present for all 78 benchmarks and fires baseline sims for all of them in parallel: --> takes 7-8 hours to complete.


3. Run Scale-SRS with the following command from the simscript folder         

     	    $ cd scale-srs/perf_analysis/simscript/
     	    $ ./runall_srs1K.sh
     	    --> Note this command assumes traces files are present for all 78 benchmarks and fires Scale-SRS sims for all of them in parallel: --> takes 7-8 hours to complete.

4. Run RRS with the following command from the simscript folder         

     	    $ cd scale-srs/perf_analysis/simscript/
     	    $ ./runall_rrs1K.sh
     	    --> Note this command assumes traces files are present for all 78 benchmarks and fires RRS sims for all of them in parallel: --> takes 7-8 hours to complete.


### Collate Results

`ONLY AFTER ALL SIMULATIONS COMPLETE --> typically 15-16 hours later, you may try to collate the results`  

5. Check the performance of Scale-SRS and RRS normalized to Baseline using the following command (Figure 14).  
  
	    --> Script to collate results is in simscript. Individual results for all workloads and collated results are stored in scale-srs/perf_analysis/output/    
     	    $ cd scale-srs/perf_analysis/simscript/

	    --> Normalized performance for workloads in the left half of Fig 14, i.e., workloads with at least one row having > 800 activations / 64ms            
            $ ./getdata.pl -s ADDED_IPC -w interest_name -n 0 -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/
	    
	    --> Normalized performance for workload suites in the right half of Fig 14, i.e. Averages.           
	    --> Gmean value ONLY for SPEC 2006
            $ ./getdata.pl -s ADDED_IPC -w spec2006_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/
            
	    --> Gmean value ONLY for SPEC 2017            
            $ ./getdata.pl -s ADDED_IPC -w spec2017_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/
            
	    --> Gmean value ONLY for GAP            
            $ ./getdata.pl -s ADDED_IPC -w gap_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/

		--> Gmean value ONLY for COMM                              
            $ ./getdata.pl -s ADDED_IPC -w comm_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/
	    
		--> Gmean value ONLY for PARSEC                     
            $ ./getdata.pl -s ADDED_IPC -w parsec_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/
            
	    --> Gmean value ONLY for BIOBENCH                                 
            $ ./getdata.pl -s ADDED_IPC -w biobench_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/

	    --> Gmean value ONLY for MIX                              
            $ ./getdata.pl -s ADDED_IPC -w mix_name -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/

	    --> Gmean value for ALL benchmarks                              
            $ ./getdata.pl -s ADDED_IPC -w all78 -n 0 -gmean -d ../output/8c_2ch_baseline/ ../output/8c_2ch_rrs1K/ ../output/8c_2ch_srs1K/

	    -- These numbers should be reflective of Figure 14 -- Performance Numbers.
## Input Trace Files
Our simulator requires that memory-access traces are generated and available in `/input` folder.  
* You can generate the memory access traces (in the trace format described at the end of the README) for any program with Intel Pintool (v2.12) and use it with our artifact.  
* We have created the artifact assuming program traces from the benchmark suites of BIOBENCH, COMM, GAP, PARSEC, SPEC2K17, SPEC2K6 are used.  
* Each benchmark-suite folder (`/input/{SUITE-NAME}`) has a `{SUITE-NAME}.workloads` file, that lists the trace-file names ({trace-file}.gz) which are expected within each bechmark suite folder.  
* You can edit the suites and trace-file names as per your use-case, but you need to update the `simscript/bench_common.pl` with the suite and trace-file names to ensure the runscript (`simscript/runall.pl`) is aware of these updates.  

### Trace Format
Our simulator uses traces of L2-Cache Misses (memory accesses filtered through the L1 and L2 cache). 
The trace file is a `.gz` file generated using `gzwrite()` to a `gzFile*` and read using `gzgets()`.

Each entry in our trace has the following format and has information regarding one L2-Cache Miss:    
`< num_nonmem_ops, R/W, Address, DontCare1-4byte, DontCare2-4byte>`. We describe these fields below:  

   - **num_nonmem_ops**: This is a 4-byte int storing the number of instructions between the current and previous L2-miss. This is useful in IPC calculation.  
   - **R/W**: This is a 1-byte char that encodes whether the L2-miss is a read request ('R') to L3, or a write-back request to L3 ('W').  
   - **Address:** This is am 8-byte long long int, that stores the 64-byte cacheline's address accessed (virtual address).  
   - **DontCare1-4byte**, **DontCare2-4byte**: These fields are ignored by the simulator (can be 0s in the trace).  

### Trace Generation
We use Intel Pintool to instrument execution of a program and get its memory accesses (similar to the intel starter [pintool](https://github.com/jingpu/pintools/blob/master/source/tools/SimpleExamples/pinatrace.cpp), here is a useful [guide](https://mahmoudhatem.wordpress.com/2016/11/07/tracing-memory-access-of-an-oracle-process-intel-pintools/) to understand this). We obtain the memory accesses for a representative section of the program and filter the memory accesses through a two level non-inclusive cache hierarchy implemented within the pintool, to obtain the L2-Miss Trace. We produce the trace file by writing each line of the trace to a compressed file stream. We generated the traces for SPEC 2k6, 2k17 and GAP using this methodology and reformatted the traces for PARSEC and COMM provided the USIMM distribution ([link](http://utaharch.blogspot.com/2012/02/usimm.html)). Our traces we used for this project are available at: https://www.dropbox.com/s/a6cdraqac79fg53/rrs_benchmarks.tar?dl=0.