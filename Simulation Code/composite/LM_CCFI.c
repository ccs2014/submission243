#define __STDC_FORMAT_MACROS
#include "cfg_hashtable.c"
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define CCFI_BOTTOM 0x00

#define CFG_DEPTH 11 //log2-scale

cfg_hashtable *cfg;


unsigned int lm_ccfi (unsigned int *mvec, unsigned int *rvec) //mvec = {pc, ci, op, op1, op2, mr}
{
	unsigned int allowed = 0;

	if((mvec[M_OP] == ijump) || (mvec[M_OP] == ret) || (mvec[M_OP] == icall))
	{
		if(mvec[M_PC] == CCFI_BOTTOM)
		{
			allowed = 1;
			rvec[R_PC] = mvec[M_CI];
		}
		else
		{
			if(cfg_search(cfg, mvec[M_PC], mvec[M_CI]) != -1)
			{
				allowed = 1;
				rvec[R_PC] = mvec[M_CI];
			}
		}
	}
	else 
	{
		if(mvec[M_PC] == CCFI_BOTTOM)
		{
			allowed = 1;
			rvec[R_PC] = CCFI_BOTTOM;
		}
		else
		{
			if(cfg_search(cfg, mvec[M_PC], mvec[M_CI]) != -1)
			{
				allowed = 1;
				rvec[R_PC] = CCFI_BOTTOM;
			}
		}
	}

	return allowed;
}

unsigned int initialize_cfg(unsigned int depth)
{
	cfg = cfg_create(1 << depth);

	return 0;
}

unsigned int add_edge_to_cfg (unsigned int a, unsigned int b)
{
	cfg_insert(cfg, a, b);

	return 0;
}

void destroy_cfg()
{
	cfg_destroy(cfg);
}
