#define __STDC_FORMAT_MACROS

#define TYPES_BOTTOM 0x00
#define INSN   0x01


unsigned int lm_types (unsigned int *mvec, unsigned int *rvec)
{
	unsigned int allowed = 0;

	if(mvec[M_OP] == other)
	{
		if(mvec[M_CI] == INSN)
		{
			rvec[R_RES] = TYPES_BOTTOM;
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == arith2s1d)
	{
		if((mvec[M_CI] == INSN) && ((mvec[M_OP1] == TYPES_BOTTOM) && (mvec[M_OP1] == mvec[M_OP2])))
		{		
			rvec[R_RES] = TYPES_BOTTOM;
			allowed = 1;
		}

	}
	else if(mvec[M_OP] == arith1s1d)
	{
		if((mvec[M_CI] == INSN) && (mvec[M_OP1] == TYPES_BOTTOM))
		{		
			rvec[R_RES] = TYPES_BOTTOM;
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == arith1d)
	{
		if(mvec[M_CI] == INSN)
		{
			rvec[R_RES] = TYPES_BOTTOM;
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == cbranch)
	{
		if((mvec[M_CI] == INSN) && (mvec[M_OP1] == TYPES_BOTTOM) && (mvec[M_OP2] == TYPES_BOTTOM))
		{
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == ijump)
	{
		if((mvec[M_CI] == INSN))
		{
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == dcall)
	{
		if((mvec[M_CI] == INSN))
		{
			//rvec[R_RES] = RETADDR;
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == wload)
	{
		if((mvec[M_CI] == INSN) && (mvec[M_OP1] == TYPES_BOTTOM) && (mvec[M_MR] == TYPES_BOTTOM))
		{
			rvec[R_RES] = mvec[M_MR];
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == bload)
	{
		if((mvec[M_CI] == INSN) && (mvec[M_OP1] == TYPES_BOTTOM) && (mvec[M_MR] == TYPES_BOTTOM))
		{
			rvec[R_RES] = TYPES_BOTTOM;
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == wstore)
	{
		if((mvec[M_CI] == INSN) && (mvec[M_OP2] == TYPES_BOTTOM) && (mvec[M_OP1] == TYPES_BOTTOM))
		{
			rvec[R_RES] = mvec[M_OP1];
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == bstore)
	{
		if((mvec[M_CI] == INSN) && (mvec[M_OP2] == TYPES_BOTTOM) && (mvec[M_OP1] == TYPES_BOTTOM))
		{
			rvec[R_RES] = TYPES_BOTTOM;
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == move)
	{
		if((mvec[M_CI] == INSN) && (mvec[M_OP1] == TYPES_BOTTOM))
		{
			rvec[R_RES] = mvec[M_OP1];
			allowed = 1;
		}
	}
	else if(mvec[M_OP] == ret)
	{
		if(mvec[M_CI] == INSN)
		{
			allowed = 1;
		}
	}

	return allowed;
}

