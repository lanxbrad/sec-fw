/*-----------------------------------------------------------------------------
 * Name:		hs.h
 * Version:		1.1
 * Description: hyper-split packet classification algorithm
 * Author:		Yaxuan Qi
 * Modifier:    Xiaoqi Ren
 * Date:		10/20/2011
 *-----------------------------------------------------------------------------*/

#include <stdint.h>
#ifndef  _HS_H_
#define  _HS_H_


#define Rule_Num_Threshold   0
/*-----------------------------------------------------------------------------
 *  constant
 *-----------------------------------------------------------------------------*/

//#define	DEBUG
#define	LOOKUP




/* for function return value */

#define     MINFIREWALL            0
#define     TCPFIREWALL            1
#define     UDPFIREWALL            2
#define     ICMPFIREWALL           3
#define     IPFIREWALL             4
#define     ANYFIREWALL            5
#define     VPNFIREWALL            6
//#define     DYNAMICFIREWALL        5
#define     MAXFIREWALL            7
 
#define     EXC_READY              0
#define     EXC_WAIT               1
#define     EXC_FIN                2


#define HS_SUCCESS		1
#define HS_FAILURE		0
#define HS_TRUE		1
#define HS_FALSE		0

/* locate rule files */
#define RULEPATH "./"

/* for bitmap */
#define MAXFILTERS	100000 /* support 100000 rules */
#define WORDLENGTH	32	/* for 32-bit system */
#define BITMAPSIZE	256 /* MAXFILTERS/WORDLENGTH */

/* for K-tuple classification */ 
#define DIM 11
//#define USE_NSF_POOL

//#define TEST
//int  FLAG[DIM] = {1,1,0,0,0,0,0,0,0};
extern int FLAG[DIM];
extern int BITNUMBER[DIM];
/*-----------------------------------------------------------------------------
 *  structure
 *-----------------------------------------------------------------------------*/
typedef struct SEGMENTPT32
{
    uint32_t segmentpoint;
    uint8_t   flag;
}segmentpt32_t;

typedef struct SEGMENTPT64
{
    uint32_t segmentpoint[2];
    uint8_t  flag;
}segmentpt64_t;

typedef struct SEGMENTPT128
{
    uint32_t segmentpoint[4];
    uint8_t   flag;//flag = 1: both end and start; flag = -1: start; flag = 1: end;
}segmentpt128_t;

struct FILTER
{
	uint32_t cost;
	uint32_t* dim[DIM][2];
	uint8_t act;
  uint32_t action;
  uint32_t     id;
  uint32_t rule_id;

};


struct RANGE
{
    uint32_t from;
    uint32_t to;
};

struct FILTSET
{
	uint32_t	numFilters;
	struct FILTER	filtArr[MAXFILTERS];
};


struct TPOINT
{
	unsigned int value;
	unsigned char flag;
};


struct FRAGNODE
{
	unsigned int start;
	unsigned int end;
	struct FRAGNODE *next;
};


struct FRAGLINKLIST

{
	unsigned int fragNum;
	struct FRAGNODE *head;
};


struct TFRAG
{
	unsigned int value;						// end point value
	unsigned int cbm[BITMAPSIZE];					// LENGTH * SIZE bits, CBM
};


//extern struct TFRAG* ptrTfrag[2];						// released after tMT[2] is generated


struct FRAG
{
	unsigned int value;
};


//extern struct FRAG* ptrfrag[2];
//extern unsigned int fragNum[2];

struct CES
{
	unsigned short eqID;					// 2 byte, eqID;
	unsigned int  cbm[BITMAPSIZE];
	struct	CES *next;								// next CES
};

struct LISTEqS
{
	unsigned short	nCES;					// number of CES
	struct			CES *head;								// head pointer of LISTEqS
	struct			CES *rear;								// pointer to end node of LISTEqS
};

//extern struct LISTEqS* listEqs[6];


struct PNODE
{
	unsigned short	cell[65536];			// each cell stores an eqID
	struct			LISTEqS listEqs;					// list of Eqs
};

//extern  portNodes[2];

/*for hyper-splitting tree*/

//extern rule_set_t;


typedef	struct seg_point_s
{
	unsigned int	num;	// number of segment points 
	unsigned int*	pointList;	// points stores here 
} seg_point_t;

//extern seg_point_t;


typedef struct segments_s
{
	unsigned int	num;		// number of segment 
	unsigned int	range[2];	// segment 
} segments_t;

//extern segments_t;


typedef	struct search_space_s
{
	unsigned int	range[DIM][2];
} search_space_t;

//extern search_space_t;

typedef struct hs_node_s
{
	uint8_t     flag;       /*flag = 0 means the node, flag = 1 means needing linear search*/
	uint8_t		d2s;		// dimension to split, 2bit is enough 
	uint8_t		depth;		// tree depth of the node, x bits supports 2^(2^x) segments 
	uint32_t*		p2s;		// thresh value to split the current segments 
	uint16_t        num_rule;   // number of rules that matches in leaf nodes. If it is not a leaf nodes, the num_rule == 0
	uint32_t*       rule;       // the number of each matched rules in leaf nodes
	struct hs_node_s*	child[2];	// pointer to child-node, 2 for binary split 
    //void*               puser_list;
} hs_node_t;

/*
typedef struct WstRule
{
    unsigned int RuleNum;
    unsigned int RuleName[MAXFILTERS];
}wst_rule_t;
*/

typedef	struct rule_s
{
	uint32_t	pri;
    uint32_t    action;
    uint32_t*	range[DIM][2];
    uint32_t    id;
    uint32_t    rule_id;
} rule_t;

typedef struct rule_set_s
{
	uint32_t	num; // number of rules in the rule set 
	rule_t*			ruleList; // rules in the set 
} rule_set_t;

//extern hs_node_t;
//added by zwz
typedef struct {
    rule_set_t TreeSet;
    hs_node_t  TreeNode;
} unit_tree;

//extern unit_tree;
                     
typedef struct {
    unit_tree  mTree;                   //master tree
    unit_tree  sTree;                   //slave tree
    unit_tree*  uTree;                   //tree in use
} split_tree;

/*
typedef struct {
    FirewallAction action;
    int user_flag;
    int ip_flag;
}st_result;
*/
//extern split_tree;
/*-----------------------------------------------------------------------------
 *  function declaration
 *-----------------------------------------------------------------------------*/

/* read rules from file */
//int		ReadFilterFile(rule_set_t* ruleset, char* filename); /* main */
//void	LoadFilters(FILE* fp, struct FILTSET* filtset);
//int		ReadFilter(FILE* fp, struct FILTSET* filtset,	unsigned int cost);
//void	ReadIPRange(FILE* fp, unsigned int* IPrange);
//void	ReadPort(FILE* fp, unsigned int* from, unsigned int* to);
//void    ReadProtocol(FILE* fp, unsigned int* from, unsigned int* to);
//void    ReadExtra6(FILE* fp, unsigned int* from, unsigned int* to);
//void    ReadExtra7(FILE* fp, unsigned int* from, unsigned int* to);

/* build hyper-split-tree */
//extern int BuildHSTree(rule_set_t* ruleset, hs_node_t* node, unsigned int depth); /* main */
//extern int		SegPointCompare(const void * a, const void * b);

/* lookup hyper-split-tree */
//extern int		LookupHSTree(rule_set_t* ruleset, hs_node_t* root);


//int FreeRootNode(hs_node_t *rootnode);
//int FreeRuleset(rule_set_t * ruleset);
int Assign_SegmentPoints_32(rule_set_t* ruleset, uint32_t dim, uint32_t* segmentPoints, segmentpt32_t* tempPoints_32);
int Assign_SegmentPoints_64(rule_set_t* ruleset, uint32_t dim, uint32_t* segmentPoints, segmentpt64_t* tempPoints_128);
int Assign_SegmentPoints_128(rule_set_t* ruleset, uint32_t dim, uint32_t* segmentPoints, segmentpt128_t* tempPoints_128);
void ShowSeg(segmentpt64_t* temp, uint32_t length);
int LinearSearch(rule_set_t* ruleset, uint32_t num_rule, uint32_t* rule_pri, uint32_t packet[DIM][4], uint32_t* num_matched, uint32_t* matched_rule_pri);
int Matched(rule_t rule, uint32_t packet[DIM][4]);

#endif   /* ----- #ifndef _HS_H_ ----- */
