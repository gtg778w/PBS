
struct mostat_s
{
    u64 last_transition_time;
    int last_mo;
    int mo_count;
    /*The following field is a variable length array*/
    u64 stat[1];
};

/*A separate mo_stat_s structure is defined per cpu*/
struct mostat_s *mostat_p;

void LAMbS_mostat_transition(int cpu, int old_mo, int new_mo)
{
    mo_stat_s *mostat;
}

int LAMbS_mostat_read(int cpu, u64* mo_stat_read)
{
    
}

int LAMbS_mostat_init()
{
    int ret = 0;
    int mo;
    
    /*Based on the number of modes of operation, determine the size of the mostat_s
    structure*/
    int mostat_size = sizeof(mo_stat_s) + sizeof(u64)*mocount;
    
    /*Allocate the structure*/
    mostat_p = (struct mostat_s*)kmalloc(mostat_size, GFP_KERNEL);                                         
    if(NULL == percpu_mostat_p)
    {
        ret = -1;
        goto exit0
    }
    
    /*initialize the mo_stat structure*/
    mostat_p->mocount = 0;
    /*zero out the time spent in each mo*/
    for(mo = 0; mo < mocount; mo++)
    {
        mostat_p->stat[mo] = 0;
    }
    
    
    
exit0:
    return ret;
}

void LAMbS_mostat_free()
{
    
}

