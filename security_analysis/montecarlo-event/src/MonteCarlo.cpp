#include <iostream>
#include <vector>
#include <algorithm>

#include <chrono>
#include <ctime>
#include <cmath>

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <errno.h>   // for errno
#include <limits.h>  // for INT_MAX, INT_MIN
#include <time.h>
#include <stdint.h>

using namespace std;

//*****************************************
// COMMAND-LINE ARGUMENTS
//*****************************************
// argv[1] : ROW HAMMER THRESHOLD
uint64_t RH_THRESHOLD = 4800;

// argv[2] : SWAP RATE (k)
uint64_t SWAP_RATE = 6;

// argv[3] : # of Deterministic ACTS
uint64_t NUM_DETERMIN_ACTS = 3200;

// TODO: Change this to be calculated with argv 1 & 2 & 3 later
// argv[4] : # of Random Guess ACTS 
uint64_t NUM_RAND_GUESS = 437;

// argv[5] : Probability 
long double RH_PROBABILITY_PER_REFRESH_INTERVAL = 0.0;

// argv[6] : Simulation Iteration
uint64_t SIMULATION_ITERATION = 1;


// To calculate large factorial
#define MAX 10000

// # of Rows == # of Buckets
#define NUM_ROWS (128*1024)

// Experiment Size
#define THOUSAND_TRIES (1000)

// RAND NUM GEN Params
#define NN 312
#define MM 156
#define MATRIX_A 0xB5026F5AA96619E9ULL
#define UM 0xFFFFFFFF80000000ULL /* Most significant 33 bits */
#define LM 0x7FFFFFFFULL /* Least significant 31 bits */

// --- FUNCTIONS FOR PRN (Pseudo Random Number) GENERATION --- //
/* The array for the state vector */
static unsigned long long mt[NN]; 
/* mti==NN+1 means mt[NN] is not initialized */
static int mti=NN+1; 

/* initializes mt[NN] with a seed */
void init_genrand64(unsigned long long seed)
{
    mt[0] = seed;
    for (mti=1; mti<NN; mti++)
        mt[mti] =  (6364136223846793005ULL * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
}

/* initialize by an array with array-length */
/* init_key is the array for initializing keys */
/* key_length is its length */
void init_by_array64(unsigned long long init_key[],unsigned long long key_length)
{
    unsigned long long i, j, k;
    init_genrand64(19650218ULL);
    i=1; j=0;
    k = (NN>key_length ? NN : key_length);
    for (; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 3935559000370003845ULL))
        + init_key[j] + j; /* non linear */
        i++; j++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
        if (j>=key_length) j=0;
    }
    for (k=NN-1; k; k--) {
        mt[i] = (mt[i] ^ ((mt[i-1] ^ (mt[i-1] >> 62)) * 2862933555777941757ULL))
        - i; /* non linear */
        i++;
        if (i>=NN) { mt[0] = mt[NN-1]; i=1; }
    }
    
    mt[0] = 1ULL << 63; /* MSB is 1; assuring non-zero initial array */
}

/* generates a random number on [0, 2^64-1]-interval */
unsigned long long genrand64_int64(void)
{
    int i;
    unsigned long long x;
    static unsigned long long mag01[2]={0ULL, MATRIX_A};
    
    if (mti >= NN) { /* generate NN words at one time */

        /* if init_genrand64() has not been called, */
        /* a default initial seed is used     */
    if (mti == NN+1)
        init_genrand64(5489ULL);

    for (i=0;i<NN-MM;i++) {
        x = (mt[i]&UM)|(mt[i+1]&LM);
        mt[i] = mt[i+MM] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
    }
    for (;i<NN-1;i++) {
        x = (mt[i]&UM)|(mt[i+1]&LM);
        mt[i] = mt[i+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&1ULL)];
    }
    x = (mt[NN-1]&UM)|(mt[0]&LM);
    mt[NN-1] = mt[MM-1] ^ (x>>1) ^ mag01[(int)(x&1ULL)];

    mti = 0;
}

x = mt[mti++];

x ^= (x >> 29) & 0x5555555555555555ULL;
x ^= (x << 17) & 0x71D67FFFEDA60000ULL;
x ^= (x << 37) & 0xFFF7EEE000000000ULL;
x ^= (x >> 43);

return x;
}

/* generates a random number on [0, 2^63-1]-interval */
long long genrand64_int63(void)
{
    return (long long)(genrand64_int64() >> 1);
}

/* generates a random number on [0,1]-real-interval */
double genrand64_real1(void)
{
    return (genrand64_int64() >> 11) * (1.0/9007199254740991.0);
}

/* generates a random number on [0,1)-real-interval */
double genrand64_real2(void)
{
    return (genrand64_int64() >> 11) * (1.0/9007199254740992.0);
}

/* generates a random number on (0,1)-real-interval */
double genrand64_real3(void)
{
    return ((genrand64_int64() >> 12) + 0.5) * (1.0/4503599627370496.0);
}

// ----------------- END OF PRN FUNCTIONS ----------------- //

void PrintStat(vector<uint64_t> v){
    cout << "====Aggregated # of Hammered Rows per 64ms===="<<endl;
    for(vector<uint64_t>::iterator iter = v.begin(); iter != v.end(); iter++){
        cout << *iter <<endl;
    }
}

// ------------------------------

int main(int argc, char* argv[]){

      // ------- Initialize the random number generator  -------
    struct timespec clk;
    if( clock_gettime( CLOCK_REALTIME, &clk) == -1 ) {
        perror( "clock gettime" );
        exit( EXIT_FAILURE );
    }
    uint64_t timens = (uint64_t)(clk.tv_nsec);
    //PRNG initialization
    uint64_t param1 = (uint64_t)(0x12547ULL ^ timens);
    uint64_t param2 = (uint64_t)(0x21325ULL ^ timens);
    uint64_t param3 = (uint64_t)(0x31204ULL ^ timens);
    uint64_t param4 = (uint64_t)(0x45678ULL ^ timens);
    unsigned long long init[4]={param1, param2, param3, param4}, length=4;
    init_by_array64(init, length);
    // -------------------------------------------------------

    // Get arguments and Calculate relevant parameters
    assert((argc == 7) && "Need 6 arguments: (ROW_HAMMER_THRESHOLD:[4.8K, 2.4K, 1.2K] SWAP_RATE:[6-10] NUM_DETERMIN_ACTS: , NUM_RANDOM_ACTS[0-5], RH_PROBABILITY_PER_REFRESH_INTERVAL, SIMULATION_ITERATION(1K per ITER)[1-10]");
    RH_THRESHOLD = atoi(argv[1]);
    SWAP_RATE = atoi(argv[2]);
    NUM_DETERMIN_ACTS = atoi(argv[3]);
    NUM_RAND_GUESS = atoi(argv[4]);
    RH_PROBABILITY_PER_REFRESH_INTERVAL = (long double)atof(argv[5]);
    SIMULATION_ITERATION = atoi(argv[6])*THOUSAND_TRIES;

    int SWAP_THRESHOLD (RH_THRESHOLD/SWAP_RATE);
    
    // For Results
    long double total_attack_iterations=0;

    // Print used variables before starting simulation   
    cout<<"=====Used variables in the simulation===="<<endl;
    cout<<"# of Rows (# of Buckets): "<<NUM_ROWS<<endl;
    cout<<"Row Hammer Threshold: "<<RH_THRESHOLD<<endl;
    cout<<"Swap Rate: "<<SWAP_RATE<<endl;
    cout<<"Swap Threshold: "<<SWAP_THRESHOLD<<endl;
    cout<<"# of Deterministic Access per 64ms: "<<NUM_DETERMIN_ACTS<<endl;
    cout<<"# of Random Guess per 64ms: "<<NUM_RAND_GUESS<<endl;
    cout<<"RH Probability per 64ms: "<<RH_PROBABILITY_PER_REFRESH_INTERVAL<<endl;
    cout<<"Simulation iterations: "<<SIMULATION_ITERATION<<endl;

    // Start Simulation Here
    for (size_t i = 1; i <= SIMULATION_ITERATION; i++){
        long double attack_iteration = 0.0;
        attack_iteration = (-1)* (log(1-((long double)(genrand64_int64()%1000000)/1000000))) /RH_PROBABILITY_PER_REFRESH_INTERVAL;
        total_attack_iterations += attack_iteration;

        if(i%(1000)==0){
            cout<<"====Iteration: "<<i/(1000)<<" K===="<<endl;
            cout<<"Current average attack time: "<<total_attack_iterations*0.064/i<<"secs"<<endl;
        }
    }
    cout<<"Final Average Attack Time: "<<total_attack_iterations*0.064/SIMULATION_ITERATION<<"secs"<<endl;
}