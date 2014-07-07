#define __STDC_FORMAT_MACROS
#include <time.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

//LM servers for the component policies
#include "LM_CCFI.c"
#include "LM_MemSafety.c"
#include "LM_Types.c"
#include "LM_TaintTracking.c"


struct CompositeTag
{
	unsigned int tag_value;
	unsigned int tagset[4];
};

struct CompositeTag *composite_tags;
unsigned int num_composite_tags;
unsigned int next_composite_tag_value;

//create a prefix search tree for composite tags...
//for N policies, it is N+1 deep (level 0 is the root, which is NULL)...
struct TagTreeNode
{
	unsigned int component_tag_value;
	struct TagTreeNode **subtree_root;
	unsigned int num_subtrees;

	unsigned int composite_tag_value;
};

struct TagTreeNode tag_prefix_tree_root;
struct TagTreeNode types_bottom, types_insn, memsafety_bottom, ccfi_bottom, taint_bottom;


unsigned int create_composite_tag(unsigned int policy, struct TagTreeNode *root, unsigned int *tagset)
{
	unsigned int i;
	unsigned int tag_value;

	if (num_composite_tags%1024 == 0)
	{
		composite_tags = (struct CompositeTag *) realloc(composite_tags, (num_composite_tags+1024) * sizeof(struct CompositeTag));
	}

	//damn you symbols, this should be a loop!
	composite_tags[num_composite_tags].tagset[TYPES] = tagset[TYPES];
	composite_tags[num_composite_tags].tagset[MEMSAFETY] = tagset[MEMSAFETY];
	composite_tags[num_composite_tags].tagset[CCFI] = tagset[CCFI];
	composite_tags[num_composite_tags].tagset[TAINT] = tagset[TAINT];

	composite_tags[num_composite_tags].tag_value = next_composite_tag_value;

	num_composite_tags++;
	tag_value = next_composite_tag_value;
	next_composite_tag_value += 1;//4;

	
	//add the new tag to the prefix search tree...
	//only need to add the tree branches from below the subtree root where the composite tag did not match...
	while (policy < 4)
	{
		if ((root->num_subtrees > 0 ) && (root->num_subtrees % 32 == 0))
		{
			root->subtree_root = (struct TagTreeNode **) realloc (root->subtree_root, (root->num_subtrees + 32) * sizeof(struct TagTreeNode *));
			for(i=0;i<32;i++)
			{	
				root->subtree_root[root->num_subtrees+i] = (struct TagTreeNode *) malloc(sizeof(struct TagTreeNode));
			}
		}

		root->subtree_root[root->num_subtrees]->component_tag_value = tagset[policy];
		if(policy!=3)
		{
			root->subtree_root[root->num_subtrees]->subtree_root = (struct TagTreeNode **) malloc(32 * sizeof(struct TagTreeNode *));
			for(i=0;i<32;i++)
			{
				root->subtree_root[root->num_subtrees]->subtree_root[i] = (struct TagTreeNode *) malloc(sizeof(struct TagTreeNode));
			}
		}
		else
		{
			root->subtree_root[root->num_subtrees]->subtree_root = NULL;
		}
		root->subtree_root[root->num_subtrees]->num_subtrees = 0;
		root->subtree_root[root->num_subtrees]->composite_tag_value = tag_value;
		
		root->num_subtrees++;

		root = root->subtree_root[root->num_subtrees-1];

		policy++;
	}

//	printf("created tag %d\n", num_composite_tags);

	return tag_value;
}

int initialize_composite_tags()
{
	unsigned int i;
	//pre-allocate 1024 tags...
	composite_tags = (struct CompositeTag *) malloc(1024 * sizeof(struct CompositeTag));

	//create a global bottom tag
	composite_tags[BOTTOM].tagset[TYPES] = TYPES_BOTTOM;
	composite_tags[BOTTOM].tagset[MEMSAFETY] = MEMSAFETY_BOTTOM;
	composite_tags[BOTTOM].tagset[CCFI] = CCFI_BOTTOM;
	composite_tags[BOTTOM].tagset[TAINT] = TAINT_BOTTOM;

	num_composite_tags = 1;
	next_composite_tag_value = 1;//4;

	tag_prefix_tree_root.component_tag_value = 0;
	tag_prefix_tree_root.subtree_root = (struct TagTreeNode **) malloc(2 * sizeof(struct TagTreeNode *));
	tag_prefix_tree_root.num_subtrees = 2;

	types_bottom.component_tag_value = TYPES_BOTTOM;
	types_bottom.subtree_root = (struct TagTreeNode **) malloc(32 * sizeof(struct TagTreeNode *));
	for(i=0;i<32;i++)
	{
		types_bottom.subtree_root[i] = (struct TagTreeNode *) malloc(sizeof(struct TagTreeNode));
	}
	types_bottom.num_subtrees = 1;
	types_bottom.composite_tag_value = BOTTOM;

	types_insn.component_tag_value = INSN;
	types_insn.subtree_root = (struct TagTreeNode **) malloc(32 * sizeof(struct TagTreeNode *));
	for(i=0;i<32;i++)
	{
		types_insn.subtree_root[i] = (struct TagTreeNode *) malloc(sizeof(struct TagTreeNode));
	}
	types_insn.num_subtrees = 0;
	types_insn.composite_tag_value = BOTTOM;

	memsafety_bottom.component_tag_value = MEMSAFETY_BOTTOM;
	memsafety_bottom.subtree_root = (struct TagTreeNode **) malloc(32 * sizeof(struct TagTreeNode *));
	for(i=0;i<32;i++)
	{
		memsafety_bottom.subtree_root[i] = (struct TagTreeNode *) malloc(sizeof(struct TagTreeNode));
	}
	memsafety_bottom.num_subtrees = 1;
	memsafety_bottom.composite_tag_value = BOTTOM;

	ccfi_bottom.component_tag_value = CCFI_BOTTOM;
	ccfi_bottom.subtree_root = (struct TagTreeNode **) malloc(32 * sizeof(struct TagTreeNode *));
	for(i=0;i<32;i++)
	{
		ccfi_bottom.subtree_root[i] = (struct TagTreeNode *) malloc(sizeof(struct TagTreeNode));
	}
	ccfi_bottom.num_subtrees = 1;
	ccfi_bottom.composite_tag_value = BOTTOM;

	taint_bottom.component_tag_value = TAINT_BOTTOM;
	taint_bottom.subtree_root = NULL;//(struct TagTreeNode **) malloc(128 * sizeof(struct TagTreeNode *));
	taint_bottom.num_subtrees = 0;
	taint_bottom.composite_tag_value = BOTTOM;


	//create constant type tags and the global bottom tag...	
	tag_prefix_tree_root.subtree_root[0] = &types_bottom;
	tag_prefix_tree_root.subtree_root[1] = &types_insn;

	struct TagTreeNode *rooot = tag_prefix_tree_root.subtree_root[0];
	rooot->subtree_root[0] = &memsafety_bottom;

	rooot = rooot->subtree_root[0];
	rooot->subtree_root[0] = &ccfi_bottom;

	rooot = rooot->subtree_root[0];
	rooot->subtree_root[0] = &taint_bottom;

	return 0;
}

unsigned int match_composite_tag(unsigned int *tagset)
{
	unsigned int i;
	unsigned int found, policy, tag_value;
	unsigned int component_tag_to_match;

	struct TagTreeNode *cur_node;

	cur_node = &tag_prefix_tree_root;

	policy = 0;

	while (policy < 4)
	{
		component_tag_to_match = tagset[policy];

		found = 0;
		for (i=0; i<cur_node->num_subtrees; i++)
		{
			if (component_tag_to_match == cur_node->subtree_root[i]->component_tag_value)
			{
				found = 1;
				policy++;

				tag_value = cur_node->subtree_root[i]->composite_tag_value;
				cur_node = cur_node->subtree_root[i];
				break;
			}
		}
		if (found == 0)
		{
			break;
		}
	}

	if ((found == 1) && (policy == 4))
	{
		return tag_value;
	}

	{
	//need to create a new tag if reach here...
	//first add component tags
	return create_composite_tag(policy, cur_node, tagset);
	}

}

unsigned int pat_server (
		  unsigned int *mvec
		, unsigned int *rvec
#ifdef MH_CACHE
		, FILE *mhcache_hits
#endif
#ifdef RESTAG_CACHE
		, FILE *rescache_hits
#endif
		)
{
	rvec[R_PC] = BOTTOM;
	rvec[R_RES] = BOTTOM;

	unsigned int allow_types, allow_ccfi, allow_memsafety, allow_taint;
	unsigned int allowed = 0;

	unsigned int m[6];
#ifdef MH_CACHE
	unsigned int hit = 0;
	char mhcache_string[128];
#endif
#ifdef RESTAG_CACHE
	char rescache_string[128];
#endif

	//resolve for types policy
	unsigned int r_types[2] = {TYPES_BOTTOM, TYPES_BOTTOM};
	m[M_PC]  = composite_tags[mvec[M_PC]].tagset[TYPES];
	m[M_CI]  = composite_tags[mvec[M_CI]].tagset[TYPES];
	m[M_OP]  = mvec[M_OP];
	m[M_OP1] = composite_tags[mvec[M_OP1]].tagset[TYPES];
	m[M_OP2] = composite_tags[mvec[M_OP2]].tagset[TYPES];
	m[M_MR]  = composite_tags[mvec[M_MR]].tagset[TYPES];
#ifdef MH_CACHE
	//lookup in the per-policy cache first
	allow_types = mh_cache_lookup(&hit, TYPES, m, r_types);
	if(hit == 0)
	{
		allow_types = lm_types(m, r_types);
		install_into_mh_cache(TYPES, m, r_types, allow_types);
	}
	sprintf(mhcache_string, "%d ", hit);
#else
	allow_types = lm_types(m, r_types);
#endif

	//resolve for memory safety policy
	unsigned int r_memsafety[2] = {MEMSAFETY_BOTTOM, MEMSAFETY_BOTTOM};
	m[M_PC]  = composite_tags[mvec[M_PC]].tagset[MEMSAFETY];
	m[M_CI]  = composite_tags[mvec[M_CI]].tagset[MEMSAFETY];
	//m[M_OP]  = mvec[M_OP];
	m[M_OP1] = composite_tags[mvec[M_OP1]].tagset[MEMSAFETY];
	m[M_OP2] = composite_tags[mvec[M_OP2]].tagset[MEMSAFETY];
	m[M_MR]  = composite_tags[mvec[M_MR]].tagset[MEMSAFETY];
#ifdef MH_CACHE
	//lookup in the per-policy cache first
	allow_memsafety = mh_cache_lookup(&hit, MEMSAFETY, m, r_memsafety);
	if(hit == 0)
	{
		allow_memsafety = lm_memsafety(m, r_memsafety);
		install_into_mh_cache(MEMSAFETY, m, r_memsafety, allow_memsafety);
	}
	sprintf(mhcache_string, "%s %d ", mhcache_string, hit);
#else
	allow_memsafety = lm_memsafety(m, r_memsafety);
#endif

	//resolve for cfi policy
	unsigned int r_ccfi[2] = {CCFI_BOTTOM, CCFI_BOTTOM};
	m[M_PC]  = composite_tags[mvec[M_PC]].tagset[CCFI];
	m[M_CI]  = composite_tags[mvec[M_CI]].tagset[CCFI];
	//m[M_OP]  = mvec[M_OP];
	m[M_OP1] = composite_tags[mvec[M_OP1]].tagset[CCFI];
	m[M_OP2] = composite_tags[mvec[M_OP2]].tagset[CCFI];
	m[M_MR]  = composite_tags[mvec[M_MR]].tagset[CCFI];
#ifdef MH_CACHE
	//lookup in the per-policy cache first
	allow_ccfi = mh_cache_lookup(&hit, CCFI, m, r_ccfi);
	if(hit == 0)
	{
		allow_ccfi = lm_ccfi(m, r_ccfi);
		install_into_mh_cache(CCFI, m, r_ccfi, allow_ccfi);
	}
	sprintf(mhcache_string, "%s %d ", mhcache_string, hit);
#else
	allow_ccfi = lm_ccfi(m, r_ccfi);
#endif

	//resolve for taint-tracking policy
	unsigned int r_taint[2] = {TAINT_BOTTOM, TAINT_BOTTOM};
	m[M_PC]  = composite_tags[mvec[M_PC]].tagset[TAINT];
	m[M_CI]  = composite_tags[mvec[M_CI]].tagset[TAINT];
	//m[M_OP]  = mvec[M_OP];
	m[M_OP1] = composite_tags[mvec[M_OP1]].tagset[TAINT];
	m[M_OP2] = composite_tags[mvec[M_OP2]].tagset[TAINT];
	m[M_MR]  = composite_tags[mvec[M_MR]].tagset[TAINT];
#ifdef MH_CACHE
	//lookup in the per-policy cache first
	allow_taint = mh_cache_lookup(&hit, TAINT, m, r_taint);
	if(hit == 0)
	{
		allow_taint = lm_taint(m, r_taint);
		install_into_mh_cache(TAINT, m, r_taint, allow_taint);
	}
	sprintf(mhcache_string, "%s %d", mhcache_string, hit);
	fprintf(mhcache_hits, "%s\n", mhcache_string);
#else
	allow_taint = lm_taint(m, r_taint);
#endif

	unsigned int rpc_composite[4];
	rpc_composite[TYPES] = r_types[R_PC];
	rpc_composite[MEMSAFETY] = r_memsafety[R_PC];
	rpc_composite[CCFI] = r_ccfi[R_PC];
	rpc_composite[TAINT] = r_taint[R_PC];
	
	unsigned int rres_composite[4];
	rres_composite[TYPES] = r_types[R_RES];
	rres_composite[MEMSAFETY] = r_memsafety[R_RES];
	rres_composite[CCFI] = r_ccfi[R_RES];
	rres_composite[TAINT] = r_taint[R_RES];


#ifdef RESTAG_CACHE
	rvec[R_PC] = restag_cache_lookup(&hit, rpc_composite);
	if(hit == 0)
	{
		rvec[R_PC] = match_composite_tag(rpc_composite);
		install_into_restag_cache(rpc_composite, rvec[R_PC]);
	}
	sprintf(rescache_string, "%d ", hit);


	rvec[R_RES] = restag_cache_lookup(&hit, rres_composite);
	if(hit == 0)
	{
		rvec[R_RES] = match_composite_tag(rres_composite);
		install_into_restag_cache(rres_composite, rvec[R_RES]);
	}
	sprintf(rescache_string, "%s %d", rescache_string, hit);
	fprintf(rescache_hits, "%s\n", rescache_string);
#else
	rvec[R_PC] = match_composite_tag(rpc_composite);
	rvec[R_RES] = match_composite_tag(rres_composite);
#endif

	allowed = allow_types && allow_ccfi && allow_memsafety && allow_taint;

	return allowed;
}	


unsigned int initialize_LMServers(unsigned int ncolors)
{
	//initialize_LM_Types();
	initialize_LM_MemSafety(ncolors);
	//initialize_LM_CCFI();
	initialize_LM_TaintTracking();

	initialize_composite_tags();

	return 0;
}


void destroy_LMServer()
{
	free(composite_tags);
	free(tag_prefix_tree_root.subtree_root);
}
