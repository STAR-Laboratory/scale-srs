
#ifndef __CONFIG_FILE_IN_H__
#define __CONFIG_FILE_IN_H__

#include "params.h"

#define 	EOL 	10
#define 	CR 	13
#define 	SPACE	32
#define		TAB	9

typedef enum {
    inst_million_token,
    processor_clk_multiplier_token,
    robsize_token,
    max_retire_token,
    max_fetch_token,
    pipelinedepth_token,
    
    num_channels_token,
    num_ranks_token,
    num_banks_token,
    num_rows_token,
    num_columns_token,
    cache_line_size_token,
    cachesize_token,
    address_bits_token,
    
    dram_clk_frequency_token,
    t_rcd_token,
    t_rp_token,
    t_cas_token,
    t_rc_token,
    t_ras_token,
    t_rrd_token,
    t_faw_token,
    t_wr_token,
    t_wtr_token,
    t_rtp_token,
    t_ccd_token,
    t_rfc_token,
    t_refi_token,
    t_cwd_token,
    t_rtrs_token,
    t_pd_min_token,
    t_xp_token,
    t_xp_dll_token,
    t_data_trans_token,
    t_swap_token,
    t_move_token,
    
    vdd_token,
    idd0_token,
    idd2p0_token,
    idd2p1_token,
    idd2n_token,
    idd3p_token,
    idd3n_token,
    idd4r_token,
    idd4w_token,
    idd5_token,
    
    wq_capacity_token,
    rq_capacity_token,
    bm_mode_token,
    address_mapping_token,
    wq_lookup_latency_token,
   
    os_pagesize_token,
    os_num_rnd_tries_token, 

    switch_rate_token,
    trh_token,
    mitigation_mode_token,

    os_pages_token,
    blacklist_token,
    rqenable_token,
    sparerows_token,

    cra_filter_entries_token,
    cra_filter_threshold_perc_token,
    cra_enable_rai_token,

    ccachesize_token,
    ccacheblock_token,
    ccacheassoc_token,
    ccacherepl_token,
    cclookuplatency_token,

    comment_token,
    unknown_token
}token_t;


token_t tokenize(char * input){
    size_t length;
    length = strlen(input);
    if(strncmp(input, "//",2) == 0) {
        return comment_token;
    } else if (strncmp(input, "INST_MILLION",length) == 0) {
        return inst_million_token;
    } else if (strncmp(input, "PROCESSOR_CLK_MULTIPLIER",length) == 0) {
        return processor_clk_multiplier_token;
    } else if (strncmp(input, "ROBSIZE",length) == 0) {
        return robsize_token;
    } else if (strncmp(input, "MAX_RETIRE",length) == 0) {
        return max_retire_token;
    } else if (strncmp(input, "MAX_FETCH",length) == 0) {
        return max_fetch_token;
    } else if (strncmp(input, "PIPELINEDEPTH",length) == 0) {
        return pipelinedepth_token;
    } else if (strncmp(input, "NUM_CHANNELS",length) == 0) {
        return num_channels_token;
    } else if (strncmp(input, "NUM_RANKS",length) == 0) {
        return num_ranks_token;
    } else if (strncmp(input, "NUM_BANKS",length) == 0) {
        return num_banks_token;
    } else if (strncmp(input, "NUM_ROWS",length) == 0) {
        return num_rows_token;
    } else if (strncmp(input, "NUM_COLUMNS",length) == 0) {
        return num_columns_token;
    } else if (strncmp(input, "CACHE_LINE_SIZE",length) == 0) {
        return cache_line_size_token;
    } else if (strncmp(input, "CACHE_SIZE",length) == 0) {
        return cachesize_token;
    } else if (strncmp(input, "ADDRESS_BITS",length) == 0) {
        return address_bits_token;
    } else if (strncmp(input, "DRAM_CLK_FREQUENCY",length) == 0) {
        return dram_clk_frequency_token;
    } else if (strncmp(input, "T_RC",length) == 0) {
        return t_rc_token;
    } else if (strncmp(input, "T_RP",length) == 0) {
        return t_rp_token;
    } else if (strncmp(input, "T_CAS",length) == 0) {
        return t_cas_token;
    } else if (strncmp(input, "T_RCD",length) == 0) {
        return t_rcd_token;
    } else if (strncmp(input, "T_RAS",length) == 0) {
        return t_ras_token;
    } else if (strncmp(input, "T_RRD",length) == 0) {
        return t_rrd_token;
    } else if (strncmp(input, "T_FAW",length) == 0) {
        return t_faw_token;
    } else if (strncmp(input, "T_WR",length) == 0) {
        return t_wr_token;
    } else if (strncmp(input, "T_WTR",length) == 0) {
        return t_wtr_token;
    } else if (strncmp(input, "T_RTP",length) == 0) {
        return t_rtp_token;
    } else if (strncmp(input, "T_CCD",length) == 0) {
        return t_ccd_token;
    } else if (strncmp(input, "T_RFC",length) == 0) {
        return t_rfc_token;
    } else if (strncmp(input, "T_SWAP",length) == 0) {
        return t_swap_token;
    } else if (strncmp(input, "T_MOVE",length) == 0) {
        return t_move_token;
    } else if (strncmp(input, "T_REFI",length) == 0) {
        return t_refi_token;
    } else if (strncmp(input, "T_CWD",length) == 0) {
        return t_cwd_token;
    } else if (strncmp(input, "T_RTRS",length) == 0) {
        return t_rtrs_token;
    } else if (strncmp(input, "T_PD_MIN",length) == 0) {
        return t_pd_min_token;
    } else if (strncmp(input, "T_XP",length) == 0) {
        return t_xp_token;
    } else if (strncmp(input, "T_XP_DLL",length) == 0) {
        return t_xp_dll_token;
    } else if (strncmp(input, "T_DATA_TRANS",length) == 0) {
        return t_data_trans_token;
    } else if (strncmp(input, "VDD",length) == 0) {
        return vdd_token;
    } else if (strncmp(input, "IDD0",length) == 0) {
        return idd0_token;
    } else if (strncmp(input, "IDD2P0",length) == 0) {
        return idd2p0_token;
    } else if (strncmp(input, "IDD2P1",length) == 0) {
        return idd2p1_token;
    } else if (strncmp(input, "IDD2N",length) == 0) {
        return idd2n_token;
    } else if (strncmp(input, "IDD3P",length) == 0) {
        return idd3p_token;
    } else if (strncmp(input, "IDD3N",length) == 0) {
        return idd3n_token;
    } else if (strncmp(input, "IDD4R",length) == 0) {
        return idd4r_token;
    } else if (strncmp(input, "IDD4W",length) == 0) {
        return idd4w_token;
    } else if (strncmp(input, "IDD5",length) == 0) {
        return idd5_token;
    } else if (strncmp(input, "WQ_CAPACITY",length) == 0) {
        return wq_capacity_token;
    } else if (strncmp(input, "RQ_CAPACITY",length) == 0) {
        return rq_capacity_token;
    } else if (strncmp(input, "ADDRESS_MAPPING",length) == 0) {
        return address_mapping_token;
    } else if (strncmp(input, "WQ_LOOKUP_LATENCY",length) == 0) {
        return wq_lookup_latency_token;
    } else if (strncmp(input, "OS_PAGESIZE",length) == 0) {
        return os_pagesize_token;
    } else if (strncmp(input, "BM_MODE",length) == 0) {
        return bm_mode_token;
    } else if (strncmp(input, "OS_NUM_RND_TRIES",length) == 0) {
        return os_num_rnd_tries_token;
    } else if (strncmp(input, "TRH",length) == 0) {
        return trh_token;
    } else if (strncmp(input, "SWITCH_RATE",length) == 0) {
        return switch_rate_token;
    } else if (strncmp(input, "MITIGATION_MODE",length) == 0) {
        return mitigation_mode_token;
    } else if (strncmp(input, "TOTALOSPAGES",length) == 0) {
        return os_pages_token;
    } else if (strncmp(input, "BLACKLIST",length) == 0) {
        return blacklist_token;
    } else if (strncmp(input, "RQENABLE",length) == 0) {
        return rqenable_token;
    } else if (strncmp(input, "SPAREROWS",length) == 0) {
        return sparerows_token;
    } else if (strncmp(input, "CRA_FILTER_ENTRIES",length) == 0) {
        return cra_filter_entries_token;
    } else if (strncmp(input, "CRA_FILTER_THRESHOLD_PERC",length) == 0) {
        return cra_filter_threshold_perc_token;
    } else if (strncmp(input, "CRA_ENABLE_RAI",length) == 0) {
        return cra_enable_rai_token;
    } else if (strncmp(input, "CCSIZE",length) == 0) {
        return ccachesize_token;
    } else if (strncmp(input, "CCBLOCK",length) == 0) {
        return ccacheblock_token;
    } else if (strncmp(input, "CCASSOC",length) == 0) {
        return ccacheassoc_token;
    } else if (strncmp(input, "CCREPL",length) == 0) {
        return ccacherepl_token;
    } else if (strncmp(input, "CCLOOKUPLATENCY",length) == 0) {
        return cclookuplatency_token;
    }
    else {
        printf("PANIC :Unknown token %s\n",input);
        return unknown_token;
    }
}


void read_config_file(FILE * fin)
{
    char 	c;
    char 	input_string[256];
    int	input_int;
    long long int input_long;
    float   input_float;
    
    while ((c = fgetc(fin)) != EOF){
        if((c != EOL) && (c != CR) && (c != SPACE) && (c != TAB)){
           if(fscanf(fin,"%s",&input_string[1])==1){
            input_string[0] = c;}
        } else { 
           if(fscanf(fin,"%s",&input_string[0])==1){
           }
        }
        token_t input_field = tokenize(&input_string[0]);
        switch(input_field)
        {
            case comment_token:
                while (((c = fgetc(fin)) != EOL) && (c != EOF)){
                    /*comment, to be ignored */
                }
                break;
            
	    case inst_million_token:
               if(fscanf(fin,"%lld",&input_long)==1){
                INST_MILLION =  input_long;}
                break;
    
            case processor_clk_multiplier_token:
               if(fscanf(fin,"%d",&input_int)==1){
                PROCESSOR_CLK_MULTIPLIER =  input_int;}
                break;
                
            case robsize_token:
               if(fscanf(fin,"%d",&input_int)==1){
                ROBSIZE =  input_int;}
                break;
                
            case max_retire_token:
               if(fscanf(fin,"%d",&input_int)==1){
                MAX_RETIRE =  input_int;}
                break;
                
            case max_fetch_token:
                if(fscanf(fin,"%d",&input_int)==1){
                MAX_FETCH =  input_int;}
                break;
                
            case pipelinedepth_token:
                if(fscanf(fin,"%d",&input_int)==1){
                PIPELINEDEPTH =  input_int;}
                break;
                
                
            case num_channels_token:
                if(fscanf(fin,"%d",&input_int)==1){
                NUM_CHANNELS =  input_int;}
                break;
                
                
            case num_ranks_token:
                if(fscanf(fin,"%d",&input_int)==1){
                NUM_RANKS =  input_int;}
                break;
                
            case num_banks_token:
                if(fscanf(fin,"%d",&input_int)==1){
                NUM_BANKS =  input_int;}
                break;
                
            case num_rows_token:
                if(fscanf(fin,"%d",&input_int)==1){
                NUM_ROWS =  input_int;}
                break;
                
            case num_columns_token:
                if(fscanf(fin,"%d",&input_int)==1){
                NUM_COLUMNS =  input_int;}
                break;
                
            case cache_line_size_token:
                if(fscanf(fin,"%d",&input_int)==1){
                CACHE_LINE_SIZE =  input_int;}
                break;
                
            case address_bits_token:
                if(fscanf(fin,"%d",&input_int)==1){
                ADDRESS_BITS =  input_int;}
                break;
                
            case dram_clk_frequency_token:
                if(fscanf(fin,"%d",&input_int)==1){
                DRAM_CLK_FREQUENCY =  input_int;}
                break;
                
            case t_rcd_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_RCD = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_rp_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_RP = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_cas_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_CAS = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_rc_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_RC = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_ras_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_RAS = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_rrd_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_RRD = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_faw_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_FAW = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_wr_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_WR = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_wtr_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_WTR = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_rtp_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_RTP = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_ccd_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_CCD = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_rfc_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_RFC = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
            
            case t_swap_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_SWAP = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
 
            case t_move_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_MOVE = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                               
            case t_refi_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_REFI = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_cwd_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_CWD = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_rtrs_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_RTRS = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_pd_min_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_PD_MIN = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_xp_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_XP = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_xp_dll_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_XP_DLL = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case t_data_trans_token:
                if(fscanf(fin,"%d",&input_int)==1){
                T_DATA_TRANS = input_int*PROCESSOR_CLK_MULTIPLIER;}
                break;
                
            case vdd_token:
               if(fscanf(fin,"%f",&input_float)==1){
                VDD = input_float;}
                break;
                
            case idd0_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD0 = input_float;}
                break;
                
            case idd2p0_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD2P0 = input_float;}
                break;
                
            case idd2p1_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD2P1 = input_float;}
                break;
                
            case idd2n_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD2N = input_float;}
                break;
                
            case idd3p_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD3P = input_float;}
                break;
                
            case idd3n_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD3N = input_float;}
                break;
                
            case idd4r_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD4R = input_float;}
                break;
                
            case idd4w_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD4W = input_float;}
                break;
                
            case idd5_token:
               if(fscanf(fin,"%f",&input_float)==1){
                IDD5 = input_float;}
                break;
                
            case wq_capacity_token:
               if(fscanf(fin,"%d",&input_int)==1){
                WQ_CAPACITY = input_int;}
                break;

            case rq_capacity_token:
               if(fscanf(fin,"%d",&input_int)==1){
                RQ_CAPACITY = input_int;}
                break;
 
            case bm_mode_token:
               if(fscanf(fin,"%d",&input_int)==1){
                BM_MODE = input_int;}
                break;
               
            case address_mapping_token:
               if(fscanf(fin,"%d",&input_int)==1){
                ADDRESS_MAPPING= input_int;}
                break;
                
            case wq_lookup_latency_token:
               if(fscanf(fin,"%d",&input_int)==1){
                WQ_LOOKUP_LATENCY = input_int;}
                break;
                
            case cachesize_token:
               if(fscanf(fin,"%d",&input_int)==1){
                CACHE_SIZE = input_int;}
                break;

            case os_pagesize_token:
               if(fscanf(fin,"%d",&input_int)==1){
                OS_PAGESIZE = input_int;}
                break;

            case os_num_rnd_tries_token:
               if(fscanf(fin,"%d",&input_int)==1){
                OS_NUM_RND_TRIES = input_int;}
                break;

            case trh_token:
               if(fscanf(fin,"%d",&input_int)==1){
                TRH = input_int;}
                break;

            case switch_rate_token:
               if(fscanf(fin,"%d",&input_int)==1){
                SWITCH_RATE = input_int;}
                break;

            case mitigation_mode_token:
               if(fscanf(fin,"%d",&input_int)==1){
                MITIGATION_MODE = input_int;}
                break;               

            case os_pages_token:
               if(fscanf(fin,"%lld",&input_long)==1){
                TOTALOSPAGES = input_long;}
                break;

            case blacklist_token:
               if(fscanf(fin,"%d",&input_int)==1){
                BLACKLIST = input_int;}
                break;               
 
            case rqenable_token:
               if(fscanf(fin,"%d",&input_int)==1){
                RQENABLE = input_int;}
                break;               
 
            case sparerows_token:
               if(fscanf(fin,"%lld",&input_long)==1){
                SPAREROWS = input_long;}
                break;
           
            case cra_filter_entries_token:
               if(fscanf(fin,"%lld",&input_long)==1){
                CRA_FILTER_ENTRIES = input_long;}
                break;

            case cra_filter_threshold_perc_token:
               if(fscanf(fin,"%d",&input_int)==1){
                CRA_FILTER_THRESHOLD_PERC = input_int;}
                break;

            case cra_enable_rai_token:
               if(fscanf(fin,"%d",&input_int)==1){
                CRA_ENABLE_RAI = input_int;}
                break;               

            case ccachesize_token:
               if(fscanf(fin,"%d",&input_int)==1){
                CCSIZE = input_int;}
                break;

            case ccacheassoc_token:
               if(fscanf(fin,"%d",&input_int)==1){
                CCASSOC = input_int;}
                break;

            case ccacheblock_token:
               if(fscanf(fin,"%d",&input_int)==1){
                CCBLOCK = input_int;}
                break;

            case ccacherepl_token:
               if(fscanf(fin,"%d",&input_int)==1){
                CCREPL = input_int;}
                break;

            case cclookuplatency_token:
               if(fscanf(fin,"%d",&input_int)==1){
                CCLOOKUPLATENCY = input_int;}
                break;                         

            case unknown_token:
            default:
                printf("PANIC: bad token in cfg file\n");
                break;
        }
    }
}


void print_params()
{
    printf("----------------------------------------------------------------------------------------\n");
    printf("------------------------\n");
    printf("- SIMULATOR PARAMETERS -\n");
    printf("------------------------\n");
    printf("\n-------------\n");
    printf("-  INST/CORE  -\n");
    printf("-------------\n");
    printf("INSTRUCTIONS_PER_CORE :   	%6lld Million\n", INST_MILLION);
 
    printf("\n-------------\n");
    printf("- PROCESSOR -\n");
    printf("-------------\n");
    printf("PROCESSOR_CLK_MULTIPLIER :   %6d\n", PROCESSOR_CLK_MULTIPLIER);
    printf("ROBSIZE :                    %6d\n", ROBSIZE);
    printf("MAX_FETCH :                  %6d\n", MAX_FETCH);
    printf("MAX_RETIRE :                 %6d\n", MAX_RETIRE);
    printf("PIPELINEDEPTH :              %6d\n", PIPELINEDEPTH);
    
    printf("\n---------------\n");
    printf("- DRAM_Config -\n");
    printf("---------------\n");
    printf("NUM_CHANNELS :               %6d\n", NUM_CHANNELS);
    printf("NUM_RANKS :                  %6d\n", NUM_RANKS);
    printf("NUM_BANKS :                  %6d\n", NUM_BANKS);
    printf("NUM_ROWS :                   %6d\n", NUM_ROWS);
    printf("NUM_COLUMNS :                %6d\n", NUM_COLUMNS);
    
    printf("\n---------------\n");
    printf("- DRAM_Timing -\n");
    printf("---------------\n");
    printf("T_RCD :                      %6d\n", T_RCD);
    printf("T_RP :                       %6d\n", T_RP);
    printf("T_CAS :                      %6d\n", T_CAS);
    printf("T_RC :                       %6d\n", T_RC);
    printf("T_RAS :                      %6d\n", T_RAS);
    printf("T_RRD :                      %6d\n", T_RRD);
    printf("T_FAW :                      %6d\n", T_FAW);
    printf("T_WR :                       %6d\n", T_WR);
    printf("T_WTR :                      %6d\n", T_WTR);
    printf("T_RTP :                      %6d\n", T_RTP);
    printf("T_CCD :                      %6d\n", T_CCD);
    printf("T_RFC :                      %6d\n", T_RFC);
    printf("T_REFI :                     %6d\n", T_REFI);
    printf("T_CWD :                      %6d\n", T_CWD);
    printf("T_RTRS :                     %6d\n", T_RTRS);
    printf("T_PD_MIN :                   %6d\n", T_PD_MIN);
    printf("T_XP :                       %6d\n", T_XP);
    printf("T_XP_DLL :                   %6d\n", T_XP_DLL);
    printf("T_DATA_TRANS :               %6d\n", T_DATA_TRANS);
    
    printf("\n---------------------------\n");
    printf("- DRAM_VI_Specifications -\n");
    printf("---------------------------\n");
    
    printf("VDD :                        %05.2f\n", VDD);
    printf("IDD0 :                       %05.2f\n", IDD0);
    printf("IDD2P0 :                     %05.2f\n", IDD2P0);
    printf("IDD2P1 :                     %05.2f\n", IDD2P1);
    printf("IDD2N :                      %05.2f\n", IDD2N);
    printf("IDD3P :                      %05.2f\n", IDD3P);
    printf("IDD3N :                      %05.2f\n", IDD3N);
    printf("IDD4R :                      %05.2f\n", IDD4R);
    printf("IDD4W :                      %05.2f\n", IDD4W);
    printf("IDD5 :                       %05.2f\n", IDD5);
    
    printf("\n-------------------\n");
    printf("- DRAM Controller -\n");
    printf("-------------------\n");
    printf("WQ_CAPACITY :                %6d\n", WQ_CAPACITY);
    printf("RQ_CAPACITY :                %6d\n", RQ_CAPACITY);
    printf("ADDRESS_MAPPING :            %6d\n", ADDRESS_MAPPING);
    printf("WQ_LOOKUP_LATENCY :          %6d\n", WQ_LOOKUP_LATENCY);
    
    printf("\n-------------------\n");
    printf("- Cache_Mode -\n");
    printf("-------------------\n");
    printf("CACHE_SIZE :                 %6d\n", CACHE_SIZE);

    printf("\n-------------------\n");
    printf("- OS -\n");
    printf("-------------------\n");
    printf("PAGE_SIZE :                  %llu\n", OS_PAGESIZE);
    printf("RAND_TRIES :                 %llu\n", OS_NUM_RND_TRIES);

    printf("\n-------------------\n");
    printf("- ADDITIONAL_CONFIGS -\n");
    printf("-------------------\n");
    printf("BENCHMARKING_MODE :          %d\n", BM_MODE);
    printf("T_RH :                       %d\n", TRH);
    printf("SWITCH_RATE :                %d\n", SWITCH_RATE);
    printf("T_SWAP :                     %d\n", T_SWAP);
    printf("T_MOVE :                     %d\n", T_MOVE);
    printf("MITIGATION_MODE :            %d\n", MITIGATION_MODE);
    printf("TOTALOSPAGES :               %lld\n", TOTALOSPAGES);
    printf("BLACKLIST :                  %d\n", BLACKLIST);
    printf("RQENABLE :                   %d\n", RQENABLE);
    printf("SPAREROWS :                  %lld\n", SPAREROWS);
    printf("CRA_FILTER_ENTRIES :         %lld\n", CRA_FILTER_ENTRIES);
    printf("CRA_FILTER_THRESHOLD_PERC :  %d\n", CRA_FILTER_THRESHOLD_PERC);
    printf("CRA_ENABLE_RAI :             %d\n", CRA_ENABLE_RAI);
    printf("CCSIZE :                     %d\n", CCSIZE);
    printf("CCBLOCK :                    %d\n", CCBLOCK);
    printf("CCASSOC :                    %d\n", CCASSOC);
    printf("CCREPL :                     %d\n", CCREPL);
    printf("CCLOOKUPLATENCY :            %d\n", CCLOOKUPLATENCY);

    printf("\n----------------------------------------------------------------------------------------\n");
}


#endif // __CONFIG_FILE_IN_H__
