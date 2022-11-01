#  Scalable and Secure Row-Swap: Efficient and Safe Row Hammer Mitigation in Memory Systems (HPCA 2023)

To appear in [HPCA 2023](https://hpca-conf.org/2023/)

## Introduction
This artifact covers two aspects of the results from the paper: 

* **Security analysis of our Juggernaut attack against Randomized Row-Swap (RRS)**: A [Bins and Buckets](https://en.wikipedia.org/wiki/Balls_into_bins_problem) model of the Juggernaut attack is provided in a C++ program to validate our statistical security analysis. Our program is based on the event-driven Monte Carlo simulations for faster simulations since naive Monte Carlo Simulations could spend a lot of time, like a few days or a week, for the simulations. We provide scripts to compile our simulators and to recreate the results shown in **Figure 6**.

* **Performance analysis of our Scalable and Secure Row-Swap (Scale-SRS) and RRS**: We provide the C code for the implementation of Scale-SRS and RRS which is encapsulated within the USIMM memory system simulator. The Scale-SRS and RRS structures and operations are implemented within the memory controller module in our artifact. We provide scripts to compile our simulator and run the baseline, Scale-SRS, and RRS for all the workloads we studied in this paper. We also provide scripts to parse the results and collate the performance results shown in **Figure 14**.

## Requirements for Security Evaluations for the Juggernaut Attack:
* **Software Dependencies**: C++, Python3, g++ (tested to compile successfully with the version: 9.4.0 and 11.3.0), and Python3 Packages (pandas and matplotlib).
* **Hardware Dependencies**: A single core CPU desktop/laptop will allow a 100,000 iterations Monte Carlo simulations in 1-3 minutes.
* **Data Dependencies**: Several input values, such as the number of attack rounds and the success probability of attack in a single refresh interval ($p_{k, T_S}$) in Equation 6 in the paper, are required to run the simulation. We generated these values following the equations in Section III-B in the paper and included the values in `scale-srs/security_analysis/montecarlo-event/simscript/input`.

## Steps for Security Evaluations

### Clone the artifact and run the code.

<!-- TODO: Change the url after creating random GitHub repo -->
* **Fetch the code**: `git clone https://github.com/STAR-Laboratory/scale-srs`
* **Run the artifact**: 

			$ cd scale-srs/security_analysis/montecarlo-event/
			$ ./run_artifact.sh
These commands run all the following steps (compile, execute, collate results, and generate the pdf file of Figure 6). You may also follow these steps manually.
### Compile

1. Compile the Event-driven Monte Carlo Code with the following steps
         
     	    $ cd scale-srs/security_analysis/montecarlo-event/
     	    $ make clean
     	    $ make
### Execute
2. Run Row Hammer threshold of 4800 with the following command from the simscript folder
         
     	    $ cd scale-srs/security_analysis/montecarlo-event/simscript/
     	    $ ./run_exp_4800.sh

3. Run Row Hammer threshold of 2400 with the following command from the simscript folder
         
     	    $ cd scale-srs/security_analysis/montecarlo-event/simscript/
     	    $ ./run_exp_2400.sh

4. Run Row Hammer threshold of 1200 with the following command from the simscript folder
         
     	    $ cd scale-srs/security_analysis/montecarlo-event/simscript/
     	    $ ./run_exp_1200.sh
### Collate Results  

5. Check the successful attack time of Juggernaut against RRS using the following command. This will generate csv files (`aggregate_trh_1200/2400/4800.csv`) in the `scale-srs/security_analysis/montecarlo-event/results` folder. 
		
		--> Collating results for the RH threshold of 4800
     	    $ cd scale-srs/security_analysis/montecarlo-event/simscript/
     	    $ python3 get_results_4800.py

		--> Collating results for the RH threshold of 2400
     	    $ cd scale-srs/security_analysis/montecarlo-event/simscript/
     	    $ python3 get_results_2400.py

		--> Collating results for the RH threshold of 1200
     	    $ cd scale-srs/security_analysis/montecarlo-event/simscript/
     	    $ python3 get_results_1200.py
### Generate Figure 6

6. Generate the pdf file of Figure 6 using the following commands.

			$ cd scale-srs/security_analysis/montecarlo-event/graph
			$ python3 figure6.py
## Requirements for Performance Evaluations in USIMM Memory System Simulator:
* **Software Dependencies**: Perl (for scripts to run experiments and collate results) and gcc (tested to compile successfully with the version: 11.3.0).
* **Hardware Dependencies**: For running all the benchmarks, a CPU with lots of memory (128GB+) and cores (64+).
* **Trace Dependencies**: Our simulator requires traces of memory-accesses for benchmarks (filtered through a L1 and L2 cache). We generate these traces using an Intel Pintool (version 2.12), similar to this pin-tool. However, traces extracted in the format described at the end of the README by any methodology (e.g., any Pin version) would be supported.
## Steps for Performance Evaluations

### Prepare Traces and Run the artifact.

* **Prepare Traces**: Download traces from this link `https://drive.google.com/file/d/1scEhit3nKWwnZwHiWLMBZ_lyNZzXoyzX/view?usp=sharing` and move them into the `scale-srs/perf_analysis/input/` folder. Downloaded file (`scale-srs-traces.tar.gz`) size should be ~8GB in size. Then change your difectory into the `scale-srs/perf_analysis/input/` folder, then type `tar -zxvf scale-srs-traces.tar.gz`. After finishing these steps, you should see `BIOBENCH, COMM, GAP, PARSEC, SPEC2K17, and SPEC2K6` folders inside the `scale-srs/perf_analysis/input/` folder. We also provide information on how traces can be generated at the end of the README.md.
* **Run the artifact**: 

			$ cd scale-srs/perf_analysis
			$ ./run_artifact.sh
These commands run all the following steps (compile, execute, collate results, and generate the pdf file of Figure 14). You may also follow these steps manually.
 
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

5. Check the performance of Scale-SRS and RRS normalized to Baseline using the following command. This will generate the `data.csv` file in the `scale-srs/perf_analysis/simscript` folder.  
  
	    	$ cd scale-srs/perf_analysis/simscript/
			$ ./plot.sh
	    -- These numbers should be reflective of Figure 14 -- Performance Numbers.

### Generate Figure 14

6. Generate the pdf file of Figure 14 using the following commands.

			$ cd scale-srs/perf_analysis/graph/
			$ python3 figure14.py
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
We use Intel Pintool to instrument execution of a program and get its memory accesses (similar to the intel starter [pintool](https://github.com/jingpu/pintools/blob/master/source/tools/SimpleExamples/pinatrace.cpp), here is a useful [guide](https://mahmoudhatem.wordpress.com/2016/11/07/tracing-memory-access-of-an-oracle-process-intel-pintools/) to understand this). We obtain the memory accesses for a representative section of the program and filter the memory accesses through a two level non-inclusive cache hierarchy implemented within the pintool, to obtain the L2-Miss Trace. We produce the trace file by writing each line of the trace to a compressed file stream. We generated the traces for SPEC 2k6, 2k17 and GAP using this methodology and reformatted the traces for PARSEC and COMM provided the USIMM distribution ([link](http://utaharch.blogspot.com/2012/02/usimm.html)). Our traces we used for this project are available at: https://drive.google.com/file/d/1scEhit3nKWwnZwHiWLMBZ_lyNZzXoyzX/view?usp=sharing.