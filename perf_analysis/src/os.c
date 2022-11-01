#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>

#include "params.h"
#include "os.h"


////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////


OS *os_new(uns num_pages, uns OS_PAGESIZE, uns OS_NUM_RND_TRIES, uns num_threads)
{
    OS *os = (OS *) calloc (1, sizeof (OS));

    os->num_pages      = num_pages;
    os->num_threads    = num_threads;
    os->num_rnd_tries  = OS_NUM_RND_TRIES;
    os->lines_in_page  = OS_PAGESIZE/CACHE_LINE_SIZE;
    os->page_alloc_isrand = 1;
    os->last_allotted_pagenum = 0;

    os->pt     = (PageTable *) calloc (1, sizeof (PageTable));
    os->pt->entries     = (Hash_Table *) calloc (1, sizeof(Hash_Table));
    init_hash_table(os->pt->entries, "PageTableEntries", num_pages, sizeof( PageTableEntry ));
    os->pt->max_entries = os->num_pages;
    for(int i = 0; i < num_threads; i++){
        os->pt->last_xlation[i].pfn = 0;
    }

    os->ipt     = (InvPageTable *) calloc (1, sizeof (InvPageTable));
    os->ipt->entries = (InvPageTableEntry *) calloc (os->num_pages, sizeof (InvPageTableEntry));
    os->ipt->num_entries = os->num_pages;

    assert(os->pt->entries);
    assert(os->ipt->entries);

    printf("Initialized OS for %u pages\n", num_pages);

    return os;
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns os_vpn_to_pfn(OS *os, uns64 vpn, uns tid, Flag *hit)
{
    Flag first_access;
    PageTable *pt = os->pt;
    InvPageTable *ipt = os->ipt;
    PageTableEntry *pte;
    InvPageTableEntry *ipte;
    *hit = TRUE;
    
    if(vpn>>60!=0)
    {
        printf("vpn:%llx %llx\n",vpn, vpn>>60);
    }
   // assert(vpn>>28 == 0);
   // vpn = (tid<<28)+vpn; // embed tid information in high bits
    assert(vpn>>60 == 0); //x86-64 system uses 48 bit virtual address space; 
    vpn = (((uns64)(tid))<<60)+vpn; // embed tid information in high bits
    
    if( pt->last_xlation[tid].vpn == vpn ){
	    return pt->last_xlation[tid].pfn;
    }
    
    pte = (PageTableEntry *) hash_table_access_create(pt->entries, vpn, &first_access);

    if(first_access){
	pte->pfn = os_get_victim_from_ipt(os);
	ipte = &ipt->entries[ pte->pfn ]; 
	ipte->valid = TRUE;
	ipte->dirty = FALSE;
	ipte->vpn   = vpn;
	assert( (uns)pt->entries->count <= pt->max_entries);
	pt->miss_count++;
	*hit=FALSE;
    }

    ipte = &ipt->entries[ pte->pfn ]; 
    ipte->ref = TRUE;
    
    pt->last_xlation[tid].vpn = vpn;
    pt->last_xlation[tid].pfn = pte->pfn;

    return pte->pfn;
}


////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

uns    os_get_victim_from_ipt(OS *os)
{
    PageTable *pt = os->pt;
    InvPageTable *ipt = os->ipt;
    uns ptr = ipt->refptr;
    uns max = ipt->num_entries;
    Flag found=FALSE;
    uns victim=0;
    uns random_invalid_tries=0;
    uns tries=0;
    //tries=tries; // To avoid warning
    random_invalid_tries = os->num_rnd_tries;
    // try random invalid first

    if(os->page_alloc_isrand == 0){ 
    //If first_touch policy for page mapping:
      //Check if page after last_allotted_page is invalid / available
       os->last_allotted_pagenum++;
       victim = ( (os->last_allotted_pagenum) %  ipt->num_entries ) ; 
       if(! ipt->entries[victim].valid ){
            found = TRUE;
       }   
    }
    else{
        while( tries < random_invalid_tries){
        victim = rand()%max;
            if(! ipt->entries[victim].valid ){
                found = TRUE;
                break;
            }
        tries++;
        }
    }
    // try finding a victim if no invalid victim
    while(!found){
	  if( ! ipt->entries[ ptr ].valid ){
	    found = TRUE;
	  }
	
	  if( ipt->entries[ ptr ].valid && ipt->entries[ ptr ].ref == FALSE){
	    found = hash_table_access_delete(pt->entries, ipt->entries[ptr].vpn);
	    assert(found);
	  }else{
	    ipt->entries[ptr].ref = FALSE;
	  }
	  victim = ptr;
	  ipt->refptr = (ptr+1)%max;
	  ptr = ipt->refptr;
    }
    // update page writeback information
    if( ipt->entries[victim].valid){
	pt->total_evicts++;
	if(ipt->entries[victim].dirty ){
	    pt->evicted_dirty_page++;
	}
    } 
    return victim; 
}
////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

void os_print_stats(OS *os, uns OS_PAGESIZE)
{
    char header[256];
    sprintf(header, "OS");
    
    printf("\n\n");
    printf("\n%s_PAGE_MISS       \t : %llu",  header, os->pt->miss_count);
    printf("\n%s_PAGE_EVICTS     \t : %llu",  header, os->pt->total_evicts);
    printf("\n%s_FOOTPRINT       \t : %llu",  header, (os->pt->miss_count*OS_PAGESIZE)/(1024*1024));
    printf("\n");
    free(os->ipt->entries);
    free(os->pt->entries->entries);
    free(os->ipt);
    free(os->pt);
    free(os);
}

////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////

Addr os_v2p_lineaddr(OS *os, Addr lineaddr, uns tid){
  uns64 vpn = lineaddr/os->lines_in_page;
  uns lineid = lineaddr%os->lines_in_page;
  Flag pagehit;
  uns pfn = os_vpn_to_pfn(os, vpn, tid, &pagehit);
  Addr retval = (pfn*os->lines_in_page)+lineid;
  return retval;
}
