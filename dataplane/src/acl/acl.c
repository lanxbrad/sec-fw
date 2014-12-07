#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
//#include <pthread.h>
#include <stdbool.h>
#include <signal.h>
#include <sys/time.h>
#include "hs.h"
#include <time.h>

#include <acl_rule.h>


extern int BuildHSTree(rule_set_t* ruleset, hs_node_t* node, unsigned int depth);
extern int PreBuildHSTree(rule_set_t* ruleset, hs_node_t* node, unsigned int depth,time_t second,FILE* dir,uint32_t* p_rule_complex);
extern int LookupHSTree(unsigned int packet[DIM][4] ,rule_set_t* ruleset, hs_node_t* root, hs_node_t** hitnode,unsigned int* pdepth);


int  FLAG[DIM] = {2,2,0,0,0,0,0};
int  BITNUMBER[] = {32,128,64};

struct TFRAG* ptrTfrag[2];                      // released after tMT[2] is generated
struct FRAG* ptrfrag[2];
unsigned int fragNum[2];
struct LISTEqS* listEqs[6];
struct PNODE portNodes[2];

//int CurrentWstNodeNum = 0;
//int MaxWstNodeNum = 4;
//int InitialMaxWstNodeNum = 4;
uint32_t gWstRange[DIM][2][4];

uint32_t mem_stat1 = 0;
uint32_t num1 = 0;
uint32_t mem_stat2 = 0;
uint32_t num2 = 0;
uint32_t mem_stat3 = 0;
uint32_t num3 = 0;
//wst_rule_t* WstRuleNode = NULL;
time_t  hs_start_time;
#define  DEBUGv2
uint32_t	gChildCount = 0; //the total number of tree's child nodes

uint32_t	gNumTreeNode = 0;//the total number of tree's nodes
uint32_t	gNumLeafNode = 0;//the total number of tree's leaf nodes

uint32_t	gWstDepth = 0;//the worst depth = 0
uint32_t	gAvgDepth = 0;//the average depth

#define MAX_DEPTH   30
/*#define MEM_LIMIT*/
unsigned int	gNodeDepth[256][2];
unsigned int	gBucket[MAX_DEPTH];

unsigned int	gNumNonOverlappings[DIM];
//unsigned long long	gNumTotalNonOverlappings = 1;

struct timeval	gStartTime,gEndTime;

uint32_t number_bits;
unit_tree g_acltree;


/*****************************************************************************/
/* Function     : range_alloc                                               */
/* Description  : 申请匹配因子的内存                                                 */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
bool range_alloc(unsigned int* dim[DIM][2])    
{
    int i,j;
    bool fail_flag = false;

    for(i = 0;i < DIM;i++)
    {
        for(j = 0;j<2;j++)
        {
            dim[i][j] = NULL;
        }
    }

	dim[0][0] = (unsigned int*)malloc(16);
	dim[0][1] = (unsigned int*)malloc(16);
	dim[1][0] = (unsigned int*)malloc(16);
	dim[1][1] = (unsigned int*)malloc(16);
	dim[2][0] = (unsigned int*)malloc(8);
	dim[2][1] = (unsigned int*)malloc(8);
	dim[3][0] = (unsigned int*)malloc(8);
	dim[3][1] = (unsigned int*)malloc(8);
	dim[4][0] = (unsigned int*)malloc(sizeof(int));
	dim[4][1] = (unsigned int*)malloc(sizeof(int));
	dim[5][0] = (unsigned int*)malloc(sizeof(int));
	dim[5][1] = (unsigned int*)malloc(sizeof(int));
	dim[6][0] = (unsigned int*)malloc(sizeof(int));
	dim[6][1] = (unsigned int*)malloc(sizeof(int));
	dim[7][0] = (unsigned int*)malloc(8);
	dim[7][1] = (unsigned int*)malloc(8);
	dim[8][0] = (unsigned int*)malloc(sizeof(int));
	dim[8][1] = (unsigned int*)malloc(sizeof(int));
	dim[9][0] = (unsigned int*)malloc(sizeof(int));
	dim[9][1] = (unsigned int*)malloc(sizeof(int));
	dim[10][0] = (unsigned int*)malloc(sizeof(int));
	dim[10][1] = (unsigned int*)malloc(sizeof(int));


    for(i=0;i<DIM;i++)
    {
        for(j=0;j<2;j++)
        {
            if( NULL == dim[i][j] )
            {
                fail_flag = true;
            }
        }
    }

    //printf("\ncheck over\n");
    if(true == fail_flag)
    {
        printf("\nload rule malloc erro,begin to clear\n");
        for(i=0;i<DIM;i++)
        {
            for(j=0;j<2;j++)
            {
                if( NULL != dim[i][j] )
                {

                	  free(dim[i][j]);
                    dim[i][j] = NULL;
                }
            }
        }

        return false;
    }
    else
    {
        return true;
    }
}
/*****************************************************************************/
/* Function     : range_free                                               */
/* Description  : 释放匹配因子                                                 */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
void range_free(unsigned int* dim[DIM][2])
{
    int i = 0;
    int j = 0;
    for(i = 0;i < DIM;i++)
    {
        for(j = 0;j < 2;j++)
        {
            if(dim[i][j])
            {

                free(dim[i][j]);
                dim[i][j] = NULL;
            }
        }
    }

}

/*****************************************************************************/
/* Function     : hs_init                                                    */
/* Description  : 初始化该树                                                 */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
int hs_init()
{
    //CurrentWstNodeNum = 0;
    //MaxWstNodeNum = 4;
    //InitialMaxWstNodeNum = 4;

    mem_stat1 = 0;
    num1 = 0;
    mem_stat2 = 0;
    num2 = 0;
    mem_stat3 = 0;
    num3 = 0;

    gChildCount = 0; //the total number of tree's child nodes

    gNumTreeNode = 0;//the total number of tree's nodes
    gNumLeafNode = 0;//the total number of tree's leaf nodes

    gWstDepth = 0;//the worst depth = 0
    gAvgDepth = 0;//the average depth
    //gNumTotalNonOverlappings = 1;
    return 0;
}
/*****************************************************************************/
/* Function     : Show                                                    */
/* Description  : 查看标识                                                 */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static void Show(uint32_t* array,uint32_t length,uint32_t bit)
{
    printf("\nShow length = %u,bit = %u\n",length,bit);
    uint32_t i,j;
    for(i = 0;i < length; i++)
    {
        printf(" %u",array[i*(bit>>5)]);
        for(j = 1;j<(bit>>5);j++)
        {
            printf(":%u",array[i*(bit>>5)+j]);
        }
    }
}
 
/*****************************************************************************/
/* Function     : TempSegPointCompare32                                      */
/* Description  : for qsort 32                                                 */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int TempSegPointCompare32 (const void * a1, const void * b1)
{
	segmentpt32_t* a = (segmentpt32_t *)a1;
	segmentpt32_t* b = (segmentpt32_t *)b1;
	if ( a->segmentpoint < b->segmentpoint )
		return -1;
	else if ( a->segmentpoint == b->segmentpoint )
		return 0;
	else
		return 1;
}
/*****************************************************************************/
/* Function     : TempSegPointCompare64                                      */
/* Description  : for qsort 64                                                 */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int TempSegPointCompare64 (const void * a1, const void * b1)
{
	int i;
	segmentpt64_t* a = (segmentpt64_t *)a1;
	segmentpt64_t* b = (segmentpt64_t *)b1;

	for(i=0;i<2;i++){
        if ( a->segmentpoint[i] < b->segmentpoint[i] )
            return -1;
        else if ( a->segmentpoint[i] == b->segmentpoint[i] )
            continue;
        else
            return 1;
	}
	return 0;
}
/*****************************************************************************/
/* Function     : TempSegPointCompare128                                      */
/* Description  : for qsort 128                                                */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int TempSegPointCompare128 (const void * a1, const void * b1)
{
	int i;
	segmentpt128_t* a = (segmentpt128_t *)a1;
	segmentpt128_t* b = (segmentpt128_t *)b1;

	for(i=0;i<4;i++){
        if ( a->segmentpoint[i] < b->segmentpoint[i] )
            return -1;
        else if ( a->segmentpoint[i] == b->segmentpoint[i] )
            continue;
        else
            return 1;
	}
	return 0;
}

/*****************************************************************************/
/* Function     : SegPointCompare32                                      */
/* Description  : for qsort 32                                                */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int SegPointCompare32 (const void * a, const void * b)
{
	if ( *(uint32_t*)a < *(uint32_t*)b )
		return -1;
	else if ( *(uint32_t*)a == *(uint32_t*)b )
		return 0;
	else 
		return 1;
}
/*****************************************************************************/
/* Function     : SegPointCompare_64_128                                     */
/* Description  : for qsort 64、128                                          */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int SegPointCompare_64_128 (const void * a1, const void * b1)
{
	uint32_t i;
	uint32_t* a = (uint32_t*)a1;
	uint32_t* b = (uint32_t*)b1;

	if((number_bits & 0x1F) !=0){
		printf("\n>>[err]SegPointCompare: number_bits should be multiple of 32 bits!");
		exit(-1);
	}

	for(i=0;i<(number_bits>>5);i++){
		if( a[i] < b[i] )
			return -1;
		else if( a[i] == b[i] )
			continue;
		else
			return 1;
	}
	return 0;
}
/*****************************************************************************/
/* Function     : SegPointCompare_64_128_nsf                                 */
/* Description  : for qsort 64、128                                         */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int SegPointCompare_64_128_nsf (const void * a1, const void * b1,unsigned int number_bits)
{
	uint32_t i;
	uint32_t* a = (uint32_t*)a1;
	uint32_t* b = (uint32_t*)b1;

	if((number_bits & 0x1F) !=0){
		printf("\n>>[err]SegPointCompare: number_bits should be multiple of 32 bits!");
		exit(-1);
	}

	for(i=0;i<(number_bits>>5);i++){
		if( a[i] < b[i] )
			return -1;
		else if( a[i] == b[i] )
			continue;
		else
			return 1;
	}
	return 0;
}
/*****************************************************************************/
/* Function     : SegPointCompare128                                     */
/* Description  : for qsort 128                                          */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int SegPointCompare128 (const void * a1, const void * b1)
{
	uint32_t i;
	uint32_t* a = (uint32_t*)a1;
	uint32_t* b = (uint32_t*)b1;

	if((number_bits & 0x1F) !=0){
		printf("\n>>[err]SegPointCompare: number_bits should be multiple of 32 bits!");
		exit(-1);
	}

	for(i=0;i<(number_bits>>5);i++){
		if( a[i] < b[i] )
			return -1;
		else if( a[i] == b[i] )
			continue;
		else 
			return 1;
	}
	return 0;
}

/*****************************************************************************/
/* Function     : SegPointCompare128_nsf                                     */
/* Description  : for qsort 128                                          */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int SegPointCompare128_nsf (const void * a1, const void * b1,uint32_t num_bits)
{
	uint32_t i;
	uint32_t* a = (uint32_t*)a1;
	uint32_t* b = (uint32_t*)b1;

	if((num_bits & 0x1F) !=0){
		printf("\n>>[err]SegPointCompare: num_bits should be multiple of 32 bits!");
		exit(-1);
	}

	for(i=0;i<(num_bits>>5);i++){
		if( a[i] < b[i] )
			return -1;
		else if( a[i] == b[i] )
			continue;
		else 
			return 1;
	}
	return 0;
}

/*****************************************************************************/
/* Function     : SegPointCompare                                     */
/* Description  : for qsort 128                                          */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int SegPointCompare(const void * a1, const void * b1)
{
	uint32_t i;
	uint32_t* a = (uint32_t*)a1;
	uint32_t* b = (uint32_t*)b1;

	if((number_bits & 0x1F) !=0 ){
		printf("\n>>[err]SegPointCompare: number_bits should be multiple of 32 bits!");
		exit(-1);
	}

	for(i=0; i<(number_bits>>5); i++){
		if(a[4-(number_bits>>5)+i] < b[i])
			return -1;
		else if(a[4-(number_bits>>5)+i] == b[i])
			continue;
		else 
			return 1;
	}
	return 0;
}
/*****************************************************************************/
/* Function     : Assign                                                     */
/* Description  : for qsort                                                  */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int Assign(uint32_t* destination, uint32_t* source, int number_of_bits)
{
	int i;
	/* if(number_bits % 32 !=0){ */
	//printf("\nAssign, number_bits = %u\n",number_of_bits);
    if((number_bits & 0x1F) !=0){
		printf("\n>>Assign: number_of_bits should be multiple of 32 bits!");
		exit(-1);
	}
	for(i=0;i<(number_of_bits>>5);i++){
		destination[i] = source[i];
	}
	return(1);
}

/*****************************************************************************/
/* Function     : Assign_DiffBits                                                     */
/* Description  :                                                   */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int Assign_DiffBits(uint32_t* destination, int bits_destination, uint32_t* source, int bits_source)
{
	int i;
	int d_bits = bits_destination>>5;
	int s_bits = bits_source>>5;
	/* if(bits_destination % 32 !=0 || bits_source % 32 !=0 || bits_destination < bits_source){ */
	if((bits_destination & 0x1F) !=0 || (bits_source & 0x1F) !=0 || bits_destination < bits_source){
		printf("\n>>Assign_DiffBits: number_of_bits should be multiple of 32 bits!");
		exit(-1);
	}

	/* set higher bits */
	for(i=0;i<d_bits-s_bits;i++){
		destination[i] = 0;
	}
	/* set lower bits */
	for(i=d_bits-s_bits; i < d_bits; i++) {
		destination[i] = source[i-(d_bits-s_bits)];
	}
	return(1);
}
/*****************************************************************************/
/* Function     : Assign_RuleList                                                     */
/* Description  :                                                   */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int Assign_RuleList(rule_t* rd, rule_t* rs)
{
	int i, number_of_bits;
	rd->pri = rs->pri;
	rd->rule_id = rs->rule_id;
    //printf("\nassign list\n");
    for(i=0;i<DIM;i++){
		number_of_bits = BITNUMBER[FLAG[i]];

		rd->range[i][0] = (uint32_t*)malloc(number_of_bits/32 * sizeof(uint32_t));
        if(NULL == rd->range[i][0])
        {
            printf("\nmalloc erro0\n");
        }
        //mem_stat1 += number_of_bits/32 * sizeof(uint32_t);
		memset(rd->range[i][0],0,number_of_bits/32 * sizeof(uint32_t));
        Assign(rd->range[i][0], rs->range[i][0], number_of_bits);

        rd->range[i][1] = (uint32_t*)malloc(number_of_bits/32 * sizeof(uint32_t));
        //memset(rd->range[i][1],0,number_of_bits/32 * sizeof(uint32_t));
        if(NULL == rd->range[i][1])
        {
            printf("\nmalloc erro1\n");
        }

        //mem_stat1 += number_of_bits/32 * sizeof(uint32_t);
        //sleep(20);
        num3++;
        mem_stat3 += number_of_bits/32 * sizeof(uint32_t) * 2;
        memset(rd->range[i][1],0,number_of_bits/32 * sizeof(uint32_t));
        Assign(rd->range[i][1], rs->range[i][1], number_of_bits);
	}
	return(1);
}

/*****************************************************************************/
/* Function     : ShowRule                                                     */
/* Description  :  Show a Rule                                                  */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
void ShowRule(rule_set_t* ruleset, uint32_t ruleNum)
{
	int i;
    
    if(!ruleset)
    {
        printf("\nno ruleset\n");
        return;
    }

    for(i=0; i < DIM; i++) {
		printf("\ndim=%d:",i);
		if(FLAG[i] == 0) {
			printf("[%x %x]", 
					ruleset->ruleList[ruleNum].range[i][0][0], 
					ruleset->ruleList[ruleNum].range[i][1][0]);
		}
		else if(FLAG[i] == 1){
			printf("[%u:%u:%u:%u %u:%u:%u:%u]", 
					ruleset->ruleList[ruleNum].range[i][0][0],
					ruleset->ruleList[ruleNum].range[i][0][1], 
					ruleset->ruleList[ruleNum].range[i][0][2],
					ruleset->ruleList[ruleNum].range[i][0][3], 
					ruleset->ruleList[ruleNum].range[i][1][0], 
					ruleset->ruleList[ruleNum].range[i][1][1], 
					ruleset->ruleList[ruleNum].range[i][1][2],
					ruleset->ruleList[ruleNum].range[i][1][3]);
		}
		else if (FLAG[i] == 2){
			printf("[%x%x %x%x]", 
					ruleset->ruleList[ruleNum].range[i][0][0], 
					ruleset->ruleList[ruleNum].range[i][0][1],
					ruleset->ruleList[ruleNum].range[i][1][0], 
					ruleset->ruleList[ruleNum].range[i][1][1]);
		} 
                else
                {
                        printf("[%u-%u %u-%u]", 
					ruleset->ruleList[ruleNum].range[i][0][0], 
					ruleset->ruleList[ruleNum].range[i][0][1],
					ruleset->ruleList[ruleNum].range[i][1][0], 
					ruleset->ruleList[ruleNum].range[i][1][1]);
                }
    }
}
/*****************************************************************************/
/* Function     : ShowRule                                                     */
/* Description  :  Show a Rule                                                  */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
int Assign_SegmentPoints_32(rule_set_t* ruleset, uint32_t dim, uint32_t* segPoints, segmentpt32_t* tempPoints_32)
{

    uint32_t number_bits = BITNUMBER[FLAG[dim]];
    uint32_t num, pos;
    uint32_t diffSegPts = 1;


    if(number_bits > 32){
        printf(">>err: Assign_SegmentPoints_32, bit above 32\n");
        exit(-1);
    }
    for (num=0; num<ruleset->num; num++) {
        segPoints[(num<<1)] = ruleset->ruleList[num].range[dim][0][0];
        segPoints[(num<<1)+1] = ruleset->ruleList[num].range[dim][1][0];
    }
    tempPoints_32[0].segmentpoint = segPoints[0];
    //Assign(&(tempPoints_32[0]->segmentpoint), &segPoints[0], number_bits);
    //SegFlag[0] = 1;
    tempPoints_32[0].flag = 1;

    for(num=1; num < 2*ruleset->num; num++) {
        int if_unique = 1;
        for(pos=0; pos < diffSegPts; pos++) {
            if(segPoints[num] == tempPoints_32[pos].segmentpoint){
                if(tempPoints_32[pos].flag == 3 || ((tempPoints_32[pos].flag == ((num & 1)+1)))){
                    if_unique = 0;
                    break;
                }

                else{
                    if((tempPoints_32[pos].flag | ((num & 1)+1))!=3){
                        printf(">>[err] tempPoints_32[pos].flag has a problem!\n");
                        exit(-1);
                    }
                    tempPoints_32[pos].flag = 3;
                    if_unique = 0;
                    break;
                }
            }
        }
        if(if_unique) {
            tempPoints_32[diffSegPts].segmentpoint = segPoints[num];
            tempPoints_32[diffSegPts].flag = (num & 1)+1;
            diffSegPts++;
        }
    }

    return(1);
}
/*****************************************************************************/
/* Function     : Assign_SegmentPoints_64                              */
/* Description  :                                                    */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
int Assign_SegmentPoints_64(rule_set_t* ruleset, uint32_t dim, uint32_t* segPoints, segmentpt64_t* tempPoints_128)
{

    uint32_t number_bits = BITNUMBER[FLAG[dim]];
    uint32_t num, pos;
    uint32_t diffSegPts = 1;
    uint32_t i;

    if(number_bits <= 32){
        printf(">>err: Assign_SegmentPoints_128, bit equal 32\n");
        exit(-1);
    }
    for (num=0; num<ruleset->num; num++) {
        for(i=0;i<(number_bits>>5);i++){
            segPoints[(number_bits>>4)*num + i] = ruleset->ruleList[num].range[dim][0][i];
            segPoints[(number_bits>>4)*num + i + (number_bits>>5)] = ruleset->ruleList[num].range[dim][1][i];
        }
    }

    Assign(&(tempPoints_128[0].segmentpoint[0]), &segPoints[0], number_bits);
    tempPoints_128[0].flag = 1;

    for(num=1; num < 2*ruleset->num; num++) {
        int if_unique = 1;
        for(pos=0; pos < diffSegPts; pos++) {
            if(SegPointCompare_64_128(&segPoints[num*number_bits/32],&(tempPoints_128[pos].segmentpoint[0])) == 0){
                if(tempPoints_128[pos].flag == 3 || ((tempPoints_128[pos].flag == ((num & 1)+1)))){
                    if_unique = 0;
                    break;
                }
                else{
                    if((tempPoints_128[pos].flag | ((num & 1)+1))!=3){
                        printf("%u, %u\n", tempPoints_128[pos].flag, (num & 1)+1);
                        printf(">>[err] tempPoints_128[pos].flag has a problem!\n");
                        exit(-1);
                    }
                    tempPoints_128[pos].flag = 3;
                    if_unique = 0;
                    break;
                }
            }

        }
        if(if_unique) {
            Assign(&(tempPoints_128[diffSegPts].segmentpoint[0]), &segPoints[num*number_bits/32], number_bits);
            tempPoints_128[diffSegPts].flag = (num & 1)+1;
            diffSegPts++;
        }
    }
    return(1);
}

/*****************************************************************************/
/* Function     : Assign_SegmentPoints_128                              */
/* Description  :                                                    */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
int Assign_SegmentPoints_128(rule_set_t* ruleset, uint32_t dim, uint32_t* segPoints, segmentpt128_t* tempPoints_128)
{

    uint32_t number_bits = BITNUMBER[FLAG[dim]];
    uint32_t num, pos;
    uint32_t diffSegPts = 1;
    uint32_t i;


    if(number_bits <= 32){
        printf(">>err: Assign_SegmentPoints_128, bit equal 32\n");
        exit(-1);
    }
    for (num=0; num<ruleset->num; num++) {
        for(i=0;i<(number_bits>>5);i++){
            segPoints[(number_bits>>4)*num + i] = ruleset->ruleList[num].range[dim][0][i];
            segPoints[(number_bits>>4)*num + i + (number_bits>>5)] = ruleset->ruleList[num].range[dim][1][i];
        }
    }

    Assign(&(tempPoints_128[0].segmentpoint[0]), &segPoints[0], number_bits);
    tempPoints_128[0].flag = 1;

    for(num=1; num < 2*ruleset->num; num++) {
        int if_unique = 1;
        for(pos=0; pos < diffSegPts; pos++) {
            if(SegPointCompare_64_128(&segPoints[num*number_bits/32], &(tempPoints_128[pos].segmentpoint[0])) == 0){
                if(tempPoints_128[pos].flag == 3 || ((tempPoints_128[pos].flag == ((num & 1)+1)))){
                    if_unique = 0;
                    break;
                }

                else{
                    if((tempPoints_128[pos].flag | ((num & 1)+1))!=3){
                        printf(">>[err] tempPoints_128[pos].flag has a problem!\n");
                        exit(-1);
                    }
                    tempPoints_128[pos].flag = 3;
                    if_unique = 0;
                    break;
                }
            }
        }
        if(if_unique) {
            Assign(&(tempPoints_128[diffSegPts].segmentpoint[0]), &segPoints[num*number_bits/32], number_bits);
            tempPoints_128[diffSegPts].flag = (num & 1)+1;
            diffSegPts++;
        }
    }

    return(1);
}
/*****************************************************************************/
/* Function     : Subtract                              */
/* Description  :                                                    */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline void Subtract(uint32_t* array, int bits)
{
	int length = bits/32;
	int i;
	for(i=length-1; i >= 0; i--){
		if(array[i] == 0){
			array[i] --;
			continue;
		}
		array[i]--;
		break;
	}
}
/*****************************************************************************/
/* Function     : Add                              */
/* Description  :                                                    */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/
static inline int Add(uint32_t* array, int bits)
{
	int length = bits/32;
	int i;
	for(i = length-1;i>=0;i--){
		array[i]++;
		if(array[i] > 0){
			return(1);
		}
	}
	return(0);
}
/*****************************************************************************/
/* Function     : release_ruleset                              */
/* Description  :                                                    */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/                 
void release_ruleset(rule_set_t* childRuleSet)
{
    uint32_t num;
    for(num = 0; num < childRuleSet->num; num++)
    {
        int k;
        for(k=0;k<DIM;k++)
        {
            free(childRuleSet->ruleList[num].range[k][0]);
            free(childRuleSet->ruleList[num].range[k][1]);
        }
    }
    
    if(childRuleSet->ruleList)
        free(childRuleSet->ruleList);
    if(childRuleSet)
        free(childRuleSet);
}
/*****************************************************************************/
/* Function     : Prediction                              */
/* Description  :                                                    */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/                                                                                     
double	Prediction (rule_set_t* ruleset)
{
	/* generate segments for input filtset */
	uint32_t i;
	uint32_t	dim, num, pos;
	//uint32_t	if_start_is_end[DIM], if_end_is_start[DIM];
	uint32_t	diffSegPts = 1; /* at least there are one differnt segment point */
	uint32_t    diffSegPts_All = 1; /* distinguish start and end when a point's flag is both*/

	uint32_t    *segPoints;
	uint32_t	*tempSegPoints;
	uint32_t    *SegFlag;
    
    uint32_t    pre_number_bits = 0;

    double      overlapping_count = 0;
    
	/*** find the splitting dimension ***/
	for(dim=0; dim < DIM; dim++) {
		pre_number_bits = BITNUMBER[FLAG[dim]]; /* int BITNUMBER[4] = {32, 32, 128, 64}; */
		diffSegPts = 1; /* at least there are one differnt segment point */
        diffSegPts_All = 1;

		segPoints = (uint32_t*) malloc( (ruleset->num<<1) * (pre_number_bits>>5) * sizeof(uint32_t));
        tempSegPoints  = (uint32_t*) malloc( (ruleset->num<<1) * (pre_number_bits>>5) * sizeof(uint32_t));
		SegFlag = (uint32_t*)malloc((ruleset->num<<1)*sizeof(uint32_t));

		
        if(!segPoints || !tempSegPoints || !SegFlag)
        {
            if(segPoints)
            {
                free(segPoints);
            }
            if(SegFlag)
            {
                free(SegFlag);
            }
            if(tempSegPoints)
            {
                free(tempSegPoints);
            }
            return -1;
        }

        for (num=0; num<ruleset->num; num++) {
			for(i=0; i<(pre_number_bits>>5); i++){
				segPoints[(pre_number_bits>>4)*num + i] = ruleset->ruleList[num].range[dim][0][i];
				segPoints[(pre_number_bits>>4)*num + i + (pre_number_bits>>5)] = ruleset->ruleList[num].range[dim][1][i];
			}
		}
//		Show(segPoints, (ruleset->num)<<1, number_bits);
		Assign(&tempSegPoints[0], &segPoints[0], pre_number_bits);
		SegFlag[0] = 1;

		/* count number of different segment points */
		/*The underlyng assumption is that odd point in segPoints is an end point, the even point in segPoints is a start point*/
		/**you can optimize this part. To make the programme clear, I just let it go as below. A hash table??**/
		pre_number_bits = BITNUMBER[FLAG[dim]];
		for(num=1; num < 2*ruleset->num; num++) {
			int if_unique = 1;
			for(pos=0; pos < diffSegPts; pos++) {
				if (pre_number_bits <= 32) {
					if(SegPointCompare32(&segPoints[num*pre_number_bits/32], &tempSegPoints[pos*pre_number_bits/32]) == 0){
						//if it is "both" or the same with the points in Seg
						if(SegFlag[pos] == 3 || ((SegFlag[pos] == ((num & 1)+1)))){
                            if_unique = 0;
                            break;
						}
						else{
                            if((SegFlag[pos] | ((num & 1)+1))!=3){
                                printf(">>[err] SegFlag has a problem!\n");
                                exit(-1);
                            }
                            SegFlag[pos] = 3;
                            diffSegPts_All++;
                            if_unique = 0;
                            break;
						}
					}
				}
				else {
					if(SegPointCompare_64_128(&segPoints[num*number_bits/32], &tempSegPoints[pos*number_bits/32]) == 0){
						if(SegFlag[pos] == 3 || ((SegFlag[pos] == ((num & 1)+1)))){
                            if_unique = 0;
                            break;
						}
						else{
                            if((SegFlag[pos] | ((num & 1)+1))!=3){
                                printf(">>[err] SegFlag has a problem!\n");
                                exit(-1);
                            }
                            SegFlag[pos] = 3;
                            diffSegPts_All++;
                            if_unique = 0;
                            break;
						}
					}
				}
			}
			if(if_unique) {
				Assign(&tempSegPoints[diffSegPts*number_bits/32], &segPoints[num*number_bits/32], number_bits);
				SegFlag[diffSegPts] = (num & 1)+1;
				diffSegPts++;
				diffSegPts_All++;
                }
		}
		/*different variable num compared to above*/
        for(num=0; num < ruleset->num; num++){
            int temp1, temp2;
            for(pos=0; pos < diffSegPts; pos++) {
				if (number_bits <= 32) {
				    temp1 = SegPointCompare32(&segPoints[2*num*number_bits/32], &tempSegPoints[pos*number_bits/32]);
				    temp2 = SegPointCompare32(&segPoints[(2*num+1)*number_bits/32], &tempSegPoints[pos*number_bits/32]);
				    if(temp1 <= 0 && temp2 >= 0){
                        overlapping_count++;
                        if(SegFlag[pos] == 3){
                            overlapping_count++;
                        }
				    }
				}
				else {
                    temp1 = SegPointCompare_64_128(&segPoints[2*num*number_bits/32], &tempSegPoints[pos*number_bits/32]);
				    temp2 = SegPointCompare_64_128(&segPoints[(2*num+1)*number_bits/32], &tempSegPoints[pos*number_bits/32]);
				    if(temp1 <=0 && temp2 >= 0){
                        overlapping_count++;
                        if(SegFlag[pos] == 3){
                            overlapping_count++;
                        }
				    }
				}
			}
			overlapping_count--;
        }

		if(segPoints)
        {
            free(segPoints);
        }
        if(SegFlag)
        {
            free(SegFlag);
        }
        if(tempSegPoints)
        {
            free(tempSegPoints);
        }
	}
	overlapping_count /= ruleset->num;
	return	overlapping_count;
}
                                    
//static time_t start_time;
static uint32_t max_rule_id = 0;

/*****************************************************************************/
/* Function     : PreBuildHSTree                              */
/* Description  :  pre building hyper-splitting tree via recursion           */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/   
int	PreBuildHSTree (rule_set_t* ruleset, hs_node_t* currNode, uint32_t depth, time_t second,FILE* dir,uint32_t* p_rule_complex)
{

	//printf("\nruleset = %p, node = %p, depth = %u\n",ruleset,currNode,depth);

    time_t cur_time;
    uint32_t i; 
	uint32_t worst_rule;
    
    struct timeval end;

    if(0 == depth)
    {
        hs_init();
    }

    if(0 == hs_start_time)
    {
        hs_start_time = time(NULL);
    }

    //printf("\ndepth = %u\n",depth);
    if(0 == depth)
    {
		 for (i = 0; i < ruleset->num; i++) {
            if (ruleset->ruleList[i].rule_id > max_rule_id) {
                max_rule_id = ruleset->ruleList[i].rule_id;
            }
        }

	/*  
        build_time = calloc(max_rule_id + 1, sizeof(*build_time));
        printf("\nBuild time = %p\n",build_time);
        if (build_time == NULL) {
            exit(-1);
        }
        */
    }

    cur_time = time(NULL);

    if(cur_time - hs_start_time >= second)
    {
        printf("\ntime out\n");
        uint32_t i = 0;
       
        
        FILE* fp = NULL;
        char* result_file = "/tmp/fw_rule/result.txt";
        fp = fopen(result_file,"w");

        fwrite("2",1,1,fp);

        for (i = 0, worst_rule = 0; i <= max_rule_id; i++) {
            
            //printf("\ntmp id = %u,count = %u\n",i,p_rule_complex[i]);
            /*  
            if (build_time[i] > build_time[worst_rule]) {
                worst_rule = i;
            }
            */
            if (p_rule_complex[i] > p_rule_complex[worst_rule]) {
                worst_rule = i;
            }
        }
		
		/*
		for(;i < ruleset->num;i++)
        {
            if(tmp_ruleid != ruleset->ruleList[i].rule_id && 0 != ruleset->ruleList[i].rule_id )
            {
        */    
            if(NULL == dir)
            {
                printf("\nMost complex rule: %u\n",worst_rule);
            }
            else
            {
                if(100000 <= worst_rule)
                {
                    printf("\nRule number is too large, worst_rule = [%u]\n",worst_rule);
                    return HS_FAILURE; 
                }

                char tmp_array[5];
                memset((void*)tmp_array,0,5);
                sprintf(tmp_array,"%u",worst_rule);
                fwrite(tmp_array,5,1,dir);
                fwrite("\r\n",2,1,dir);
            }
        /*        
                tmp_ruleid = ruleset->ruleList[i].rule_id;
                
            }
        }
		*/
		
		//printf("timeout\nworst rule: %d\n", worst_rule);
        
        if(p_rule_complex)
        {
            free(p_rule_complex);
        }

        if(dir)
        {
            fclose(dir);
        }

        if(fp)
        {
            fclose(fp);
        }
		/*
		if(build_time)
		{
			free(build_time);
		}
        */
        printf("\nout over\n");
        exit(-1);
    }

    /* generate segments for input filtset */
	//printf("\nBuild tree\n");
    //int i;
	uint32_t	dim, num, pos;
	uint32_t	maxDiffSegPts = 1;	/* maximum different segment points */
	uint32_t    maxDiffSegPts_All = 1;
	uint32_t	d2s = 0;		/* dimension to split (with max diffseg) */
	uint32_t	p2s[4];
	uint32_t	*t_p2s;
	//uint32_t	if_start_is_end[DIM], if_end_is_start[DIM];
	uint32_t	range[2][2][4];	/* sub-space ranges for child-nodes */
	uint32_t	diffSegPts = 1; /* at least there are one differnt segment point */
	uint32_t    diffSegPts_All = 1;
    uint32_t	*segPoints;
	uint32_t	*tempSegPoints;
    uint32_t    *SegFlag;
	uint32_t	*tempRuleNumList;
	rule_set_t	*childRuleSet;

    //printf("\nrule num = %u\n",ruleset->num);
#ifdef DEBUGv2
	printf("\n\n>>--------------new node in depth=%u -----------------", depth);
	printf("\n>>current rules:");
	for(num=0; num < ruleset->num; num++) {
		printf("\n>>rules#%u", num+1);
		ShowRule(ruleset, num);
	}
#endif

	/*Initialize the worst-case range */
	if(depth == 0) {
		for(dim=0; dim < DIM; dim ++) {
			Assign_DiffBits(gWstRange[dim][0], 128, ruleset->ruleList[ruleset->num-1].range[dim][0],number_bits);
			Assign_DiffBits(gWstRange[dim][1], 128, ruleset->ruleList[ruleset->num-1].range[dim][1],number_bits);
		}
	}
    if(ruleset->num <= Rule_Num_Threshold){
        currNode->flag = 1;
		currNode->d2s = 0;
		currNode->depth = depth;
		currNode->p2s = NULL;

		currNode->num_rule = ruleset->num;
        currNode->rule = (uint32_t*) malloc(ruleset->num*sizeof(uint32_t));

		uint32_t i;
        for(i = 0; i<ruleset->num; i++){
            currNode->rule[i] = ruleset->ruleList[i].pri;
        }

		currNode->child[0] = NULL;
		currNode->child[1] = NULL;


		gChildCount ++;
		gNumLeafNode ++;
		if ((gNumLeafNode & 0xFFFF) == 0) {
#ifdef DEBUGv2
			printf("\n>>number leaf-nodes: %u (k)", gNumLeafNode>>10); fflush(stdout);
#endif
        }
		/* worst-case depth and range update */
		if (gWstDepth < depth){
			gWstDepth = depth;
			/*TODO: worst-case range setup */
			for(dim=0; dim < DIM; dim ++) {
				number_bits = BITNUMBER[FLAG[dim]];
				/* getting the worst from the last ^_^ */
				Assign_DiffBits(gWstRange[dim][0], 128, ruleset->ruleList[ruleset->num-1].range[dim][0], number_bits);
				Assign_DiffBits(gWstRange[dim][1], 128, ruleset->ruleList[ruleset->num-1].range[dim][1], number_bits);
			}
		}
		gAvgDepth += depth;

		/* freeing */
		/* for(num=0; num<ruleset->num; num ++) { */
			/* free(ruleset->ruleList[num] */
        gettimeofday(&end, NULL);

        //duration = end.tv_sec * 1000000 + end.tv_usec -
        //    begin.tv_sec * 1000000 - begin.tv_usec;
        /*  
        for (i = 0; i < ruleset->num; i++) {
            build_time[ruleset->ruleList[i].rule_id] += duration;
        }
        */
		return	HS_SUCCESS;
    }
	/*** find the splitting dimension ***/
	for(dim=0; dim < DIM; dim++) {
		//printf("\nzwz005 dim = %u\n",dim);
        number_bits = BITNUMBER[FLAG[dim]]; /* int BITNUMBER[4] = {32, 32, 128, 64}; */
		diffSegPts = 1; /* at least there are one differnt segment point */
        diffSegPts_All = 1;
        
		segPoints = (uint32_t*) malloc( (ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
		tempSegPoints  = (uint32_t*) malloc( (ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
        memset(segPoints,0,(ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
        memset(tempSegPoints,0,(ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
        //printf("\nzwz dim = %u,malloc seg = %p, temp = %p\n",dim,segPoints[dim],tempSegPoints);
        //printf("\nzwz dim = %u\n",dim);
        //mem_stat2 += (ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t) * 2;
        //sleep(20);
		SegFlag = (uint32_t*)malloc((ruleset->num<<1)*sizeof(uint32_t));
		for (num=0; num<ruleset->num; num++) {
			for(i = 0; i<(number_bits>>5); i++){
				segPoints[(number_bits>>4)*num + i] = ruleset->ruleList[num].range[dim][0][i];
				segPoints[(number_bits>>4)*num + i + (number_bits>>5)] = ruleset->ruleList[num].range[dim][1][i];
			}
		}
//		Show(segPoints, (ruleset->num)<<1, number_bits);
		Assign(&tempSegPoints[0], &segPoints[0], number_bits);
		SegFlag[0] = 1;

		/* count number of different segment points */
		number_bits = BITNUMBER[FLAG[dim]];
		for(num=1; num < 2*ruleset->num; num++) {
			//printf("\ndim = %u,num = %u,number_bits = %u\n",dim,num,number_bits);

            int if_unique = 1;
			for(pos=0; pos < diffSegPts; pos++) {
				//printf("zz = %u, %u, %u\n",zz,((uint32_t*)(&segPoints[dim][num*number_bits/32]))[zz],((uint32_t*)(&tempSegPoints[pos*number_bits/32]))[zz]);
                if (number_bits <= 32) {
					if(SegPointCompare32(&segPoints[num*number_bits/32], &tempSegPoints[pos*number_bits/32]) == 0){
						if(SegFlag[pos] == 3 || ((SegFlag[pos] == ((num & 1)+1)))){
						if_unique = 0; 
							break;
						}
						else{
                            if((SegFlag[pos] | ((num & 1)+1))!=3){
                                printf(">>[err] SegFlag has a problem!\n");
                                exit(-1);
                            }
                            SegFlag[pos] = 3;
                            diffSegPts_All++;
                            if_unique = 0;
                            break;
						}
					}
				}
				else {
					if(SegPointCompare_64_128(&segPoints[num*number_bits/32], &tempSegPoints[pos*number_bits/32]) == 0){
						if(SegFlag[pos] == 3 || ((SegFlag[pos] == ((num & 1)+1)))){
                            if_unique = 0;
                            break;
						}
						else{
                            if((SegFlag[pos] | ((num & 1)+1))!=3){
                                printf(">>[err] SegFlag has a problem!\n");
                                exit(-1);
                            }
                            SegFlag[pos] = 3;
                            diffSegPts_All++;
                            if_unique = 0;
                            break;
						}
					}
				}
			}
			if(if_unique) {
				Assign(&tempSegPoints[diffSegPts*number_bits/32], &segPoints[num*number_bits/32], number_bits);
				SegFlag[diffSegPts] = (num & 1)+1;
				diffSegPts++;
				diffSegPts_All++;
			}
		}

#ifdef DEBUGv2
		printf("\n>>different seg: dim=%u, diffsegpts#=%u, diffsegpts_all#=%u, diffsegvalue(flag)=", dim, diffSegPts, diffSegPts_All);
		for(pos=0; pos < diffSegPts; pos++) {
			Show(&tempSegPoints[pos*number_bits/32], 1, number_bits);
			printf("(%u)", SegFlag[pos]);
		}
		printf("\n");
#endif

		//printf("\ndim = %d, diffSegPts = %d, diffSegPts_All = %d", dim, diffSegPts, diffSegPts_All);
		if (maxDiffSegPts_All < diffSegPts_All) {
			maxDiffSegPts_All = diffSegPts_All;
			maxDiffSegPts     = diffSegPts;
			d2s = dim;
		}
		free(segPoints);
		free(SegFlag);
		free(tempSegPoints);
	}

	number_bits = BITNUMBER[FLAG[d2s]];


	/**Have not refind the following code... Go to sleep for a while... Dorm! I am coming! **/
	/* For 3 points, directly split at the center */
	if(maxDiffSegPts_All >= 3){
        segmentpt32_t* tempPoints_32;
        segmentpt64_t* tempPoints_64;
        segmentpt128_t* tempPoints_128;
	    segPoints = (uint32_t*) malloc( (ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
        if(number_bits == 32){
            tempPoints_32 = (segmentpt32_t*)malloc(maxDiffSegPts * sizeof(segmentpt32_t));
            Assign_SegmentPoints_32(ruleset, d2s, segPoints, tempPoints_32);
        }
        else{
            if(number_bits == 64){
                tempPoints_64 = (segmentpt64_t*)malloc(maxDiffSegPts*sizeof(segmentpt64_t));
                Assign_SegmentPoints_64(ruleset, d2s, segPoints, tempPoints_64);
                //Show(segPoints, (ruleset->num)<<1, 64);
                //printf("\n");
            }
            else{
                tempPoints_128 = (segmentpt128_t*)malloc(maxDiffSegPts*sizeof(segmentpt128_t));
                Assign_SegmentPoints_128(ruleset, d2s, segPoints, tempPoints_128);
            }
        }


		/*Sort the Segment Points*/
		if (number_bits == 32) //forgive me...
			qsort(&(tempPoints_32[0]), maxDiffSegPts, sizeof(segmentpt32_t), TempSegPointCompare32);
		else{
            if(number_bits == 64){
                //printf("%u\n", maxDiffSegPts);
                //ShowSeg(&(tempPoints_64[0]), maxDiffSegPts);
                //printf("\n");
                qsort(&(tempPoints_64[0]), maxDiffSegPts, sizeof(segmentpt64_t), TempSegPointCompare64);
                //ShowSeg(&(tempPoints_64[0]), maxDiffSegPts);
                //printf("\n");
            }
            else{
                qsort(&(tempPoints_128[0]), maxDiffSegPts, sizeof(segmentpt128_t), TempSegPointCompare128);
            }

		}


#ifdef DEBUGv2
		/* printf("\n>>different seg points after qsort:"); */
		/* for(num=0; num < 2*ruleset->num; num++) { */
		/* Show(&segPoints[d2s][num*number_bits/32], 1, number_bits); */
		/* } */
#endif
//		tempSegPoints  = (uint32_t*) malloc(maxDiffSegPts * number_bits/32 * sizeof(uint32_t));
        //printf("malloc2 = %p,max num = %d",tempSegPoints,maxDiffSegPts);
//
//		diffSegPts = 1;
//		Assign(&tempSegPoints[0], &segPoints[d2s][0], number_bits);
//		for(num=1; num<2*ruleset->num; num++){
//			if (number_bits <= 32) {
//				if(SegPointCompare32(&segPoints[d2s][num*number_bits/32], &tempSegPoints[(diffSegPts-1)*number_bits/32]) != 0){
//					Assign(&tempSegPoints[diffSegPts*number_bits/32], &segPoints[d2s][num*number_bits/32], number_bits);
//					diffSegPts++;
//				}
//			}
//			else {
//				if(SegPointCompare128(&segPoints[d2s][num*number_bits/32], &tempSegPoints[(diffSegPts-1)*number_bits/32]) != 0){
//					Assign(&tempSegPoints[diffSegPts*number_bits/32], &segPoints[d2s][num*number_bits/32], number_bits);
//					diffSegPts++;
//				}
//			}
//		}
#ifdef DEBUGv2
//		printf("\n>>unique seg points after qsort:");
//		for(num=0; num < maxDiffSegPts; num++) {
//
//			Show(&tempSegPoints[num*number_bits/32], 1, number_bits);
//		}
		//printf("\n>>maxDiffseg=%u", maxDiffSegPts);
#endif

        uint32_t temp_pos = (maxDiffSegPts_All>>1);
        uint32_t pos      = 0;
        uint32_t pos_all  = 0;
        uint8_t  flag_pos = 0;//flag show whether the point is both.
        uint8_t  flag_segment = 0;
        if(number_bits == 32){
            Assign_DiffBits(range[0][0], 128, &(tempPoints_32[0].segmentpoint), number_bits);
            Assign_DiffBits(range[1][1], 128, &(tempPoints_32[maxDiffSegPts-1].segmentpoint), number_bits);
            while(pos_all < temp_pos){
                if(tempPoints_32[pos].flag == 3){
                    pos_all++;
                    if(pos_all == temp_pos){
                        flag_pos = 1;
                        break;
                    }
                }
                pos++;
                pos_all++;
            }
            t_p2s = (&tempPoints_32[pos].segmentpoint);
            flag_segment = tempPoints_32[pos].flag;
        }
        else{
            if(number_bits == 64){
                Assign_DiffBits(range[0][0], 128, &(tempPoints_64[0].segmentpoint[0]), number_bits);
                Assign_DiffBits(range[1][1], 128, &(tempPoints_64[maxDiffSegPts-1].segmentpoint[0]), number_bits);
                while(pos_all < temp_pos){
                    if(tempPoints_64[pos].flag == 3){
                        pos_all++;
                        if(pos_all == temp_pos){
                            flag_pos = 1;
                            break;
                        }
                    }
                    pos++;

                    pos_all++;
                }
                t_p2s = (&tempPoints_64[pos].segmentpoint[0]);
                flag_segment = tempPoints_64[pos].flag;
            }
            else{
                Assign_DiffBits(range[0][0], 128, &(tempPoints_128[0].segmentpoint[0]), number_bits);
                Assign_DiffBits(range[1][1], 128, &(tempPoints_128[maxDiffSegPts-1].segmentpoint[0]), number_bits);
                while(pos_all < temp_pos){
                    if(tempPoints_128[pos].flag == 3){
                        pos_all++;
                        if(pos_all == temp_pos){
                            flag_pos = 1;
                            break;
                        }
                    }
                    pos++;
                    pos_all++;
                }
                t_p2s = &(tempPoints_128[pos].segmentpoint[0]);
                flag_segment = tempPoints_128[pos].flag;
            }
        }
        Assign_DiffBits(p2s, 128, t_p2s, number_bits);
        //start point
        if(flag_segment == 1){
            Subtract(p2s, 128);
            Assign_DiffBits(range[0][1], 128, p2s, 128);
            Add(p2s,128);
            Assign_DiffBits(range[1][0], 128, p2s, 128);
        }
        else{
            if(flag_segment == 2){
                Assign_DiffBits(range[0][1], 128, p2s, 128);
                Add(p2s,128);
                Assign_DiffBits(range[1][0], 128, p2s, 128);
            }
            else{
                if(flag_pos == 0){
                    Subtract(p2s, 128);
                    Assign_DiffBits(range[0][1], 128, p2s, 128);
                    Add(p2s,128);
                    Assign_DiffBits(range[1][0], 128, p2s, 128);
                }
                else{
                    Assign_DiffBits(range[0][1], 128, p2s, 128);
                    Add(p2s,128);
                    Assign_DiffBits(range[1][0], 128, p2s, 128);
                }
            }
        }
        if(number_bits == 32)free(tempPoints_32);
        if(number_bits == 64)free(tempPoints_64);
        if(number_bits == 128)free(tempPoints_128);

#ifdef DEBUGv2
		printf("\n>>split decision: d2s=%u, p2s=", d2s);
		Show(p2s, 1, 128);
		printf("\n>>left child range:");
		Show(range[0][0], 1, 128); printf("\t");
		Show(range[0][1], 1, 128);
		printf("\n>>right child range:");
		Show(range[1][0], 1, 128); printf("\t");
		Show(range[1][1], 1, 128);
#endif
        free(segPoints);
	}



	/*Update Leaf node*/
	if ( maxDiffSegPts_All <= 2) {
	    currNode->flag = 0;
		currNode->d2s = 0;
		currNode->depth = depth;
		currNode->p2s = NULL;
        currNode->num_rule = ruleset->num;
        currNode->rule = (uint32_t*) malloc(ruleset->num*sizeof(uint32_t));
        uint32_t i;
        for(i=0;i<ruleset->num;i++){
            currNode->rule[i] = ruleset->ruleList[i].pri;
        }
		currNode->child[0] = NULL;
		currNode->child[1] = NULL;


		gChildCount ++;
		gNumLeafNode ++;
		if ((gNumLeafNode & 0xFFFF) == 0) {
#ifdef DEBUGv2
			printf("\n>>number leaf-nodes: %u (k)", gNumLeafNode>>10); fflush(stdout);
#endif
        }

		/* worst-case depth and range update */
		if (gWstDepth < depth){
			gWstDepth = depth;
			/*TODO: worst-case range setup */
			for(dim=0; dim < DIM; dim ++) {
				number_bits = BITNUMBER[FLAG[dim]];
				/* getting the worst from the last ^_^ */
				Assign_DiffBits(gWstRange[dim][0], 128, ruleset->ruleList[ruleset->num-1].range[dim][0], number_bits);
				Assign_DiffBits(gWstRange[dim][1], 128, ruleset->ruleList[ruleset->num-1].range[dim][1], number_bits);
			}
		}
		gAvgDepth += depth;

		/* freeing */
		/* for(num=0; num<ruleset->num; num ++) { */
			/* free(ruleset->ruleList[num] */
        gettimeofday(&end, NULL);

        //duration = end.tv_sec * 1000000 + end.tv_usec -
        //    begin.tv_sec * 1000000 - begin.tv_usec;
        /*
        for (i = 0; i < ruleset->num; i++) {
            build_time[ruleset->ruleList[i].rule_id] += duration;
        }
        */
		return	HS_SUCCESS;
	}

	/**** Update currNode ****/
	number_bits = BITNUMBER[FLAG[d2s]];
	gNumTreeNode ++;
	currNode->d2s = (uint8_t) d2s;
	currNode->depth = (uint8_t) depth;
	currNode->p2s = (uint32_t*) malloc(number_bits/32 * sizeof(uint32_t));
	//printf("\nmalloc4 = %p\n",currNode->p2s);
	int j;
    for(j=number_bits/32-1; j >= 0; j--){
		currNode->p2s[j] = p2s[i+4-number_bits/32];
	}

	currNode->num_rule = 0;
	currNode->rule = NULL;
	currNode->child[0] = (hs_node_t *) malloc(sizeof(hs_node_t));
    //printf("\nmalloc5 = %p\n",currNode->child[0]);
	/*Generate left child rule list*/
	tempRuleNumList = (uint32_t*) malloc(ruleset->num * sizeof(uint32_t)); /* need to be freed */
	//printf("\nmalloc6 = %p\n",tempRuleNumList);
    //sleep(20);
    pos = 0;
    //printf("\npos ruleset num = %u\n",ruleset->num);
	for (num = 0; num < ruleset->num; num++) {
		int temp1, temp2;
		if (number_bits <= 32) {
			temp1 = SegPointCompare32(ruleset->ruleList[num].range[d2s][0], &range[0][1][4-number_bits/32]);
			temp2 = SegPointCompare32(ruleset->ruleList[num].range[d2s][1], &range[0][0][4-number_bits/32]);
		} 
		else {
			temp1 = SegPointCompare_64_128(ruleset->ruleList[num].range[d2s][0], &range[0][1][4-number_bits/32]);
			temp2 = SegPointCompare_64_128(ruleset->ruleList[num].range[d2s][1], &range[0][0][4-number_bits/32]);
		}

		if (temp1 <=0 && temp2 >= 0)
		{
			tempRuleNumList[pos] = num;
            pos++;
            //printf("\nnum = %u,pos = %u\n",num,pos);
		}
	}

	childRuleSet = (rule_set_t*) malloc(sizeof(rule_set_t));
	//printf("\nmalloc7 = %p\n",childRuleSet);
    childRuleSet->num = pos;
	childRuleSet->ruleList = (rule_t*) malloc( childRuleSet->num * sizeof(rule_t) );
	//printf("\nmalloc8 = %p\n",childRuleSet->ruleList);
    num1++;
    mem_stat1 += childRuleSet->num * sizeof(rule_t);
    number_bits = BITNUMBER[FLAG[d2s]];
    //sleep(20);
	for (num = 0; num < childRuleSet->num; num++) {
		Assign_RuleList(&childRuleSet->ruleList[num], &ruleset->ruleList[tempRuleNumList[num]]);
		/* in d2s dim, the search space needs to be trimmed off */
		if (number_bits <= 32) {
			if (SegPointCompare32(childRuleSet->ruleList[num].range[d2s][0] ,&range[0][0][4-number_bits/32])<0)
				Assign(childRuleSet->ruleList[num].range[d2s][0], &range[0][0][4-number_bits/32], number_bits);
			if (SegPointCompare32(childRuleSet->ruleList[num].range[d2s][1], &range[0][1][4-number_bits/32])>0)
				Assign(childRuleSet->ruleList[num].range[d2s][1] ,&range[0][1][4-number_bits/32], number_bits);
		}
		else {
			if (SegPointCompare128(childRuleSet->ruleList[num].range[d2s][0] ,&range[0][0][4-number_bits/32])<0)
				Assign(childRuleSet->ruleList[num].range[d2s][0],&range[0][0][4-number_bits/32],number_bits);
			if (SegPointCompare128(childRuleSet->ruleList[num].range[d2s][1], &range[0][1][4-number_bits/32])>0)
				Assign(childRuleSet->ruleList[num].range[d2s][1] ,&range[0][1][4-number_bits/32],number_bits);
		}
	}
	//printf("\nhs free add4 = %p\n",tempRuleNumList);
    //printf("\nhs free add4\n");
    free (tempRuleNumList);
    tempRuleNumList = NULL;
#ifdef DEBUGv2
	  printf("\n>>depth #%d 1subrules# %d:",depth+1, num);
	  for(i=0;i<num;i++)printf("%d ", childRuleSet->ruleList[i].pri);
#endif
    gettimeofday(&end, NULL);

    //duration = end.tv_sec * 1000000 + end.tv_usec -
    //    begin.tv_sec * 1000000 - begin.tv_usec;
    /*  
    for (i = 0; i < ruleset->num; i++) {
        build_time[ruleset->ruleList[i].rule_id] += duration;
    }
    */
	PreBuildHSTree(childRuleSet, currNode->child[0], depth+1,second,dir,p_rule_complex);

    release_ruleset(childRuleSet);
	/**** Generate right child rule list ****/
	currNode->child[1] = (hs_node_t *) malloc(sizeof(hs_node_t));
	//printf("\nmalloc9 = %p\n",currNode->child[1]);
    tempRuleNumList = (uint32_t*) malloc(ruleset->num * sizeof(uint32_t)); /* need to be free */
	//printf("\nmalloc10 = %p\n",tempRuleNumList);
    pos = 0;

    //sleep(20);
	number_bits = BITNUMBER[FLAG[d2s]];
	for (num = 0; num < ruleset->num; num++) {
		int temp1, temp2;
		if (number_bits <= 32) {
			temp1 = SegPointCompare32(ruleset->ruleList[num].range[d2s][0], &range[1][1][4-number_bits/32]);
			temp2 = SegPointCompare32(ruleset->ruleList[num].range[d2s][1], &range[1][0][4-number_bits/32]);
		}
		else {
			temp1 = SegPointCompare_64_128(ruleset->ruleList[num].range[d2s][0], &range[1][1][4-number_bits/32]);
			temp2 = SegPointCompare_64_128(ruleset->ruleList[num].range[d2s][1], &range[1][0][4-number_bits/32]);
		}
		if ( temp1<=0 && temp2 >= 0)
		{
			tempRuleNumList[pos] = num;
			pos++;
		}
	}

	childRuleSet = (rule_set_t*) malloc(sizeof(rule_set_t));
	//printf("\nmalloc11 = %p\n",childRuleSet);
    childRuleSet->num = pos;
	childRuleSet->ruleList = (rule_t*) malloc( childRuleSet->num * sizeof(rule_t) );
    //printf("\nmalloc12 = %p\n",childRuleSet->ruleList);
    //printf("\nnum = %u\n",childRuleSet->num);
    num2++;
    mem_stat2 += childRuleSet->num * sizeof(rule_t);
    //sleep(20);
	for (num = 0; num < childRuleSet->num; num++) {
		Assign_RuleList(&childRuleSet->ruleList[num], &ruleset->ruleList[tempRuleNumList[num]]);
		/* printf("\n>>ruleset->num %u", tempRuleNumList[num]); */
		/* in d2s dim, the search space needs to be trimmed off */
		if (number_bits <= 32) {
			if (SegPointCompare32(childRuleSet->ruleList[num].range[d2s][0] ,&range[1][0][4-number_bits/32])<0)
				Assign(childRuleSet->ruleList[num].range[d2s][0],&range[1][0][4-number_bits/32],number_bits);
			if (SegPointCompare32(childRuleSet->ruleList[num].range[d2s][1], &range[1][1][4-number_bits/32])>0)
				Assign(childRuleSet->ruleList[num].range[d2s][1] ,&range[1][1][4-number_bits/32],number_bits);
		}
		else {
			if (SegPointCompare_64_128(childRuleSet->ruleList[num].range[d2s][0] ,&range[1][0][4-number_bits/32])<0)
				Assign(childRuleSet->ruleList[num].range[d2s][0],&range[1][0][4-number_bits/32],number_bits);
			if (SegPointCompare_64_128(childRuleSet->ruleList[num].range[d2s][1], &range[1][1][4-number_bits/32])>0)
				Assign(childRuleSet->ruleList[num].range[d2s][1] ,&range[1][1][4-number_bits/32],number_bits);
		}
	}
    //printf("\nhs free add5 = %p\n",tempRuleNumList);
	//printf("\nhs free add5\n");
    free(tempRuleNumList);
    tempRuleNumList = NULL;
#ifdef DEBUGv2
    printf("\n>>depth #%d, 2subrules# %d:",depth+1, num);
    for(i=0;i<num;i++)printf("%d ", childRuleSet->ruleList[i].pri);
#endif
    PreBuildHSTree(childRuleSet, currNode->child[1], depth+1,second,dir,p_rule_complex);
	release_ruleset(childRuleSet);
    return	HS_SUCCESS;
}

/*****************************************************************************/
/* Function     : BuildHSTree                                      */
/* Description  :  building hyper-splitting tree via recursion        */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/  
int	BuildHSTree (rule_set_t* ruleset, hs_node_t* currNode, uint32_t depth)
{

	printf("in BuildHSTree !\n");
    if(0 == depth)
    {
        hs_init();
    }
    /* generate segments for input filtset */
	//printf("\nBuild tree\n");
    uint32_t i;
	uint32_t	dim, num, pos;
	uint32_t	maxDiffSegPts = 1;	/* maximum different segment points */
	uint32_t    maxDiffSegPts_All = 1;
	uint32_t	d2s = 0;		/* dimension to split (with max diffseg) */
	uint32_t	p2s[4];
	uint32_t	*t_p2s;
	//uint32_t	if_start_is_end[DIM], if_end_is_start[DIM];
	uint32_t	range[2][2][4];	/* sub-space ranges for child-nodes */
	uint32_t	diffSegPts = 1; /* at least there are one differnt segment point */
	uint32_t    diffSegPts_All = 1;
    uint32_t	*segPoints;
	uint32_t	*tempSegPoints;
    uint32_t    *SegFlag;
	uint32_t	*tempRuleNumList;
	rule_set_t	*childRuleSet;

    //printf("\nrule num = %u\n",ruleset->num);
#ifdef DEBUGv2
	printf("\n\n>>--------------new node in depth=%u -----------------", depth);
	printf("\n>>current rules:");
	for(num=0; num < ruleset->num; num++) {
		printf("\n>>rules#%u", num+1);
		ShowRule(ruleset, num);
	}
#endif

	/*Initialize the worst-case range */
	if(depth == 0) {
		for(dim=0; dim < DIM; dim ++) {
			Assign_DiffBits(gWstRange[dim][0], 128, ruleset->ruleList[ruleset->num-1].range[dim][0],number_bits);
			Assign_DiffBits(gWstRange[dim][1], 128, ruleset->ruleList[ruleset->num-1].range[dim][1],number_bits);
		}
	}
    if(ruleset->num <= Rule_Num_Threshold){
        currNode->flag = 1;
		currNode->d2s = 0;
		currNode->depth = depth;
		currNode->p2s = NULL;

		currNode->num_rule = ruleset->num;
        currNode->rule = (uint32_t*) malloc(ruleset->num*sizeof(uint32_t));

		uint32_t i;
        for(i=0;i<ruleset->num;i++){
            currNode->rule[i] = ruleset->ruleList[i].pri;
        }

		currNode->child[0] = NULL;
		currNode->child[1] = NULL;


		gChildCount ++;
		gNumLeafNode ++;
		if ((gNumLeafNode & 0xFFFF) == 0) {
#ifdef DEBUGv2
			printf("\n>>number leaf-nodes: %u (k)", gNumLeafNode>>10); fflush(stdout);
#endif
        }
		/* worst-case depth and range update */
		if (gWstDepth < depth){
			gWstDepth = depth;
			/*TODO: worst-case range setup */
			for(dim=0; dim < DIM; dim ++) {
				number_bits = BITNUMBER[FLAG[dim]];
				/* getting the worst from the last ^_^ */
				Assign_DiffBits(gWstRange[dim][0], 128, ruleset->ruleList[ruleset->num-1].range[dim][0], number_bits);
				Assign_DiffBits(gWstRange[dim][1], 128, ruleset->ruleList[ruleset->num-1].range[dim][1], number_bits);
			}
		}
		gAvgDepth += depth;

		/* freeing */
		/* for(num=0; num<ruleset->num; num ++) { */
			/* free(ruleset->ruleList[num] */
		return	HS_SUCCESS;
    }
	/*** find the splitting dimension ***/
	for(dim=0; dim < DIM; dim++) {
		//printf("\nzwz005 dim = %u\n",dim);
        number_bits = BITNUMBER[FLAG[dim]]; /* int BITNUMBER[4] = {32, 32, 128, 64}; */
		diffSegPts = 1; /* at least there are one differnt segment point */
        diffSegPts_All = 1;
        
		segPoints = (uint32_t*) malloc( (ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
		tempSegPoints  = (uint32_t*) malloc( (ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
        memset(segPoints,0,(ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
        memset(tempSegPoints,0,(ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
        //printf("\nzwz dim = %u,malloc seg = %p, temp = %p\n",dim,segPoints[dim],tempSegPoints);
        //printf("\nzwz dim = %u\n",dim);
        //mem_stat2 += (ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t) * 2;
        //sleep(20);
		SegFlag = (uint32_t*)malloc((ruleset->num<<1)*sizeof(uint32_t));
		for (num=0; num<ruleset->num; num++) {
			for(i=0;i<(number_bits>>5);i++){
				segPoints[(number_bits>>4)*num + i] = ruleset->ruleList[num].range[dim][0][i];
				segPoints[(number_bits>>4)*num + i + (number_bits>>5)] = ruleset->ruleList[num].range[dim][1][i];
			}
		}
//		Show(segPoints, (ruleset->num)<<1, number_bits);
		Assign(&tempSegPoints[0], &segPoints[0], number_bits);
		SegFlag[0] = 1;

		/* count number of different segment points */
		number_bits = BITNUMBER[FLAG[dim]];
		for(num=1; num < 2*ruleset->num; num++) {
			//printf("\ndim = %u,num = %u,number_bits = %u\n",dim,num,number_bits);

            int if_unique = 1;
			for(pos=0; pos < diffSegPts; pos++) {
				//printf("zz = %u, %u, %u\n",zz,((uint32_t*)(&segPoints[dim][num*number_bits/32]))[zz],((uint32_t*)(&tempSegPoints[pos*number_bits/32]))[zz]);
                if (number_bits <= 32) {
					if(SegPointCompare32(&segPoints[num*number_bits/32], &tempSegPoints[pos*number_bits/32]) == 0){
						if(SegFlag[pos] == 3 || ((SegFlag[pos] == ((num & 1)+1)))){
						if_unique = 0; 
							break;
						}
						else{
                            if((SegFlag[pos] | ((num & 1)+1))!=3){
                                printf(">>[err] SegFlag has a problem!\n");
                                exit(-1);
                            }
                            SegFlag[pos] = 3;
                            diffSegPts_All++;
                            if_unique = 0;
                            break;
						}
					}
				}
				else {
					if(SegPointCompare_64_128(&segPoints[num*number_bits/32], &tempSegPoints[pos*number_bits/32]) == 0){
						if(SegFlag[pos] == 3 || ((SegFlag[pos] == ((num & 1)+1)))){
                            if_unique = 0;
                            break;
						}
						else{
                            if((SegFlag[pos] | ((num & 1)+1))!=3){
                                printf(">>[err] SegFlag has a problem!\n");
                                exit(-1);
                            }
                            SegFlag[pos] = 3;
                            diffSegPts_All++;
                            if_unique = 0;
                            break;
						}
					}
				}
			}
			if(if_unique) {
				Assign(&tempSegPoints[diffSegPts*number_bits/32], &segPoints[num*number_bits/32], number_bits);
				SegFlag[diffSegPts] = (num & 1)+1;
				diffSegPts++;
				diffSegPts_All++;
			}
		}

#ifdef DEBUGv2
		printf("\n>>different seg: dim=%u, diffsegpts#=%u, diffsegpts_all#=%u, diffsegvalue(flag)=", dim, diffSegPts, diffSegPts_All);
		for(pos=0; pos < diffSegPts; pos++) {
			Show(&tempSegPoints[pos*number_bits/32], 1, number_bits);
			printf("(%u)", SegFlag[pos]);
		}
		printf("\n");
#endif

		//printf("\ndim = %d, diffSegPts = %d, diffSegPts_All = %d", dim, diffSegPts, diffSegPts_All);
		if (maxDiffSegPts_All < diffSegPts_All) {
			maxDiffSegPts_All = diffSegPts_All;
			maxDiffSegPts     = diffSegPts;
			d2s = dim;
		}
		free(segPoints);
		free(SegFlag);
		free(tempSegPoints);
	}

	number_bits = BITNUMBER[FLAG[d2s]];


	/**Have not refind the following code... Go to sleep for a while... Dorm! I am coming! **/
	/* For 3 points, directly split at the center */
	if(maxDiffSegPts_All >= 3){
        segmentpt32_t* tempPoints_32;
        segmentpt64_t* tempPoints_64;
        segmentpt128_t* tempPoints_128;
	    segPoints = (uint32_t*) malloc( (ruleset->num<<1) * (number_bits>>5) * sizeof(uint32_t));
        if(number_bits == 32){
            tempPoints_32 = (segmentpt32_t*)malloc(maxDiffSegPts * sizeof(segmentpt32_t));
            Assign_SegmentPoints_32(ruleset, d2s, segPoints, tempPoints_32);
        }
        else{
            if(number_bits == 64){
                tempPoints_64 = (segmentpt64_t*)malloc(maxDiffSegPts*sizeof(segmentpt64_t));
                Assign_SegmentPoints_64(ruleset, d2s, segPoints, tempPoints_64);
                //Show(segPoints, (ruleset->num)<<1, 64);
                //printf("\n");
            }
            else{
                tempPoints_128 = (segmentpt128_t*)malloc(maxDiffSegPts*sizeof(segmentpt128_t));
                Assign_SegmentPoints_128(ruleset, d2s, segPoints, tempPoints_128);
            }
        }


		/*Sort the Segment Points*/
		if (number_bits == 32) //forgive me...
			qsort(&(tempPoints_32[0]), maxDiffSegPts, sizeof(segmentpt32_t), TempSegPointCompare32);
		else{
            if(number_bits == 64){
                //printf("%u\n", maxDiffSegPts);
                //ShowSeg(&(tempPoints_64[0]), maxDiffSegPts);
                //printf("\n");
                qsort(&(tempPoints_64[0]), maxDiffSegPts, sizeof(segmentpt64_t), TempSegPointCompare64);
                //ShowSeg(&(tempPoints_64[0]), maxDiffSegPts);
                //printf("\n");
            }
            else{
                qsort(&(tempPoints_128[0]), maxDiffSegPts, sizeof(segmentpt128_t), TempSegPointCompare128);
            }

		}


#ifdef DEBUGv2
		/* printf("\n>>different seg points after qsort:"); */
		/* for(num=0; num < 2*ruleset->num; num++) { */
		/* Show(&segPoints[d2s][num*number_bits/32], 1, number_bits); */
		/* } */
#endif

#ifdef DEBUGv2
//		printf("\n>>unique seg points after qsort:");
//		for(num=0; num < maxDiffSegPts; num++) {
//
//			Show(&tempSegPoints[num*number_bits/32], 1, number_bits);
//		}
		//printf("\n>>maxDiffseg=%u", maxDiffSegPts);
#endif

        uint32_t temp_pos = (maxDiffSegPts_All>>1);
        uint32_t pos      = 0;
        uint32_t pos_all  = 0;
        uint8_t  flag_pos = 0;//flag show whether the point is both.
        uint8_t  flag_segment = 0;
        if(number_bits == 32){
            Assign_DiffBits(range[0][0], 128, &(tempPoints_32[0].segmentpoint), number_bits);
            Assign_DiffBits(range[1][1], 128, &(tempPoints_32[maxDiffSegPts-1].segmentpoint), number_bits);
            while(pos_all < temp_pos){
                if(tempPoints_32[pos].flag == 3){
                    pos_all++;
                    if(pos_all == temp_pos){
                        flag_pos = 1;
                        break;
                    }
                }
                pos++;
                pos_all++;
            }
            t_p2s = (&tempPoints_32[pos].segmentpoint);
            flag_segment = tempPoints_32[pos].flag;
        }
        else{
            if(number_bits == 64){
                Assign_DiffBits(range[0][0], 128, &(tempPoints_64[0].segmentpoint[0]), number_bits);
                Assign_DiffBits(range[1][1], 128, &(tempPoints_64[maxDiffSegPts-1].segmentpoint[0]), number_bits);
                while(pos_all < temp_pos){
                    if(tempPoints_64[pos].flag == 3){
                        pos_all++;
                        if(pos_all == temp_pos){
                            flag_pos = 1;
                            break;
                        }
                    }
                    pos++;

                    pos_all++;
                }
                t_p2s = (&tempPoints_64[pos].segmentpoint[0]);
                flag_segment = tempPoints_64[pos].flag;
            }
            else{
                Assign_DiffBits(range[0][0], 128, &(tempPoints_128[0].segmentpoint[0]), number_bits);
                Assign_DiffBits(range[1][1], 128, &(tempPoints_128[maxDiffSegPts-1].segmentpoint[0]), number_bits);
                while(pos_all < temp_pos){
                    if(tempPoints_128[pos].flag == 3){
                        pos_all++;
                        if(pos_all == temp_pos){
                            flag_pos = 1;
                            break;
                        }
                    }
                    pos++;
                    pos_all++;
                }
                t_p2s = &(tempPoints_128[pos].segmentpoint[0]);
                flag_segment = tempPoints_128[pos].flag;
            }
        }
        Assign_DiffBits(p2s, 128, t_p2s, number_bits);
        //start point
        if(flag_segment == 1){
            Subtract(p2s, 128);
            Assign_DiffBits(range[0][1], 128, p2s, 128);
            Add(p2s,128);
            Assign_DiffBits(range[1][0], 128, p2s, 128);
        }
        else{
            if(flag_segment == 2){
                Assign_DiffBits(range[0][1], 128, p2s, 128);
                Add(p2s,128);
                Assign_DiffBits(range[1][0], 128, p2s, 128);
            }
            else{
                if(flag_pos == 0){
                    Subtract(p2s, 128);
                    Assign_DiffBits(range[0][1], 128, p2s, 128);
                    Add(p2s,128);
                    Assign_DiffBits(range[1][0], 128, p2s, 128);
                }
                else{
                    Assign_DiffBits(range[0][1], 128, p2s, 128);
                    Add(p2s,128);
                    Assign_DiffBits(range[1][0], 128, p2s, 128);
                }
            }
        }
        if(number_bits == 32)free(tempPoints_32);
        if(number_bits == 64)free(tempPoints_64);
        if(number_bits == 128)free(tempPoints_128);

#ifdef DEBUGv2
		printf("\n>>split decision: d2s=%u, p2s=", d2s);
		Show(p2s, 1, 128);
		printf("\n>>left child range:");
		Show(range[0][0], 1, 128); printf("\t");
		Show(range[0][1], 1, 128);
		printf("\n>>right child range:");
		Show(range[1][0], 1, 128); printf("\t");
		Show(range[1][1], 1, 128);
#endif
        free(segPoints);
	}



	/*Update Leaf node*/
	if ( maxDiffSegPts_All <= 2) {
	    currNode->flag = 0;
		currNode->d2s = 0;
		currNode->depth = depth;
		currNode->p2s = NULL;
        currNode->num_rule = ruleset->num;
        currNode->rule = (uint32_t*) malloc(ruleset->num*sizeof(uint32_t));
        uint32_t i;
        for(i=0;i<ruleset->num;i++){
            currNode->rule[i] = ruleset->ruleList[i].pri;
        }
		currNode->child[0] = NULL;
		currNode->child[1] = NULL;


		gChildCount ++;
		gNumLeafNode ++;
		if ((gNumLeafNode & 0xFFFF) == 0) {
#ifdef DEBUGv2
			printf("\n>>number leaf-nodes: %u (k)", gNumLeafNode>>10); fflush(stdout);
#endif
        }

		/* worst-case depth and range update */
		if (gWstDepth < depth){
			gWstDepth = depth;
			/*TODO: worst-case range setup */
			for(dim=0; dim < DIM; dim ++) {
				number_bits = BITNUMBER[FLAG[dim]];
				/* getting the worst from the last ^_^ */
				Assign_DiffBits(gWstRange[dim][0], 128, ruleset->ruleList[ruleset->num-1].range[dim][0], number_bits);
				Assign_DiffBits(gWstRange[dim][1], 128, ruleset->ruleList[ruleset->num-1].range[dim][1], number_bits);
			}
		}
		gAvgDepth += depth;

		/* freeing */
		/* for(num=0; num<ruleset->num; num ++) { */
			/* free(ruleset->ruleList[num] */
		return	HS_SUCCESS;
	}

	/**** Update currNode ****/
	number_bits = BITNUMBER[FLAG[d2s]];
	gNumTreeNode ++;
	currNode->d2s = (uint8_t) d2s;
	currNode->depth = (uint8_t) depth;
	currNode->p2s = (uint32_t*) malloc(number_bits/32 * sizeof(uint32_t));
    //printf("\nmalloc4 = %p\n",currNode->p2s);
    int j;
    for(j=number_bits/32-1; j >= 0; j--){
		currNode->p2s[j] = p2s[j+4-number_bits/32];
	}

	currNode->num_rule = 0;
	currNode->rule = NULL;
	currNode->child[0] = (hs_node_t *) malloc(sizeof(hs_node_t));
    //printf("\nmalloc5 = %p\n",currNode->child[0]);
	/*Generate left child rule list*/
	tempRuleNumList = (uint32_t*) malloc(ruleset->num * sizeof(uint32_t)); /* need to be freed */
	//printf("\nmalloc6 = %p\n",tempRuleNumList);
    //sleep(20);
    pos = 0;
    //printf("\npos ruleset num = %u\n",ruleset->num);
	for (num = 0; num < ruleset->num; num++) {
		int temp1, temp2;
		if (number_bits <= 32) {
			temp1 = SegPointCompare32(ruleset->ruleList[num].range[d2s][0], &range[0][1][4-number_bits/32]);
			temp2 = SegPointCompare32(ruleset->ruleList[num].range[d2s][1], &range[0][0][4-number_bits/32]);
		} 
		else {
			temp1 = SegPointCompare_64_128(ruleset->ruleList[num].range[d2s][0], &range[0][1][4-number_bits/32]);
			temp2 = SegPointCompare_64_128(ruleset->ruleList[num].range[d2s][1], &range[0][0][4-number_bits/32]);
		}

		if (temp1 <=0 && temp2 >= 0)
		{
			tempRuleNumList[pos] = num;
            pos++;
            //printf("\nnum = %u,pos = %u\n",num,pos);
		}
	}

	childRuleSet = (rule_set_t*) malloc(sizeof(rule_set_t));
	//printf("\nmalloc7 = %p\n",childRuleSet);
    childRuleSet->num = pos;
	childRuleSet->ruleList = (rule_t*) malloc( childRuleSet->num * sizeof(rule_t) );
	//printf("\nmalloc8 = %p\n",childRuleSet->ruleList);
    num1++;
    mem_stat1 += childRuleSet->num * sizeof(rule_t);
    number_bits = BITNUMBER[FLAG[d2s]];
    //sleep(20);
	for (num = 0; num < childRuleSet->num; num++) {
		Assign_RuleList(&childRuleSet->ruleList[num], &ruleset->ruleList[tempRuleNumList[num]]);
		/* in d2s dim, the search space needs to be trimmed off */
		if (number_bits <= 32) {
			if (SegPointCompare32(childRuleSet->ruleList[num].range[d2s][0] ,&range[0][0][4-number_bits/32])<0)
				Assign(childRuleSet->ruleList[num].range[d2s][0], &range[0][0][4-number_bits/32], number_bits);
			if (SegPointCompare32(childRuleSet->ruleList[num].range[d2s][1], &range[0][1][4-number_bits/32])>0)
				Assign(childRuleSet->ruleList[num].range[d2s][1] ,&range[0][1][4-number_bits/32], number_bits);
		}
		else {
			if (SegPointCompare128(childRuleSet->ruleList[num].range[d2s][0] ,&range[0][0][4-number_bits/32])<0)
				Assign(childRuleSet->ruleList[num].range[d2s][0],&range[0][0][4-number_bits/32],number_bits);
			if (SegPointCompare128(childRuleSet->ruleList[num].range[d2s][1], &range[0][1][4-number_bits/32])>0)
				Assign(childRuleSet->ruleList[num].range[d2s][1] ,&range[0][1][4-number_bits/32],number_bits);
		}
	}
	//printf("\nhs free add4 = %p\n",tempRuleNumList);
    //printf("\nhs free add4\n");
    free (tempRuleNumList);
    tempRuleNumList = NULL;
#ifdef DEBUGv2
	  printf("\n>>depth #%d 1subrules# %d:",depth+1, num);
	  for(i=0;i<num;i++)printf("%d ", childRuleSet->ruleList[i].pri);
#endif
	BuildHSTree(childRuleSet, currNode->child[0], depth+1);

    release_ruleset(childRuleSet);
	/**** Generate right child rule list ****/
	currNode->child[1] = (hs_node_t *) malloc(sizeof(hs_node_t));
	//printf("\nmalloc9 = %p\n",currNode->child[1]);
    tempRuleNumList = (uint32_t*) malloc(ruleset->num * sizeof(uint32_t)); /* need to be free */
	//printf("\nmalloc10 = %p\n",tempRuleNumList);
    pos = 0;

    //sleep(20);
	number_bits = BITNUMBER[FLAG[d2s]];
	for (num = 0; num < ruleset->num; num++) {
		int temp1, temp2;
		if (number_bits <= 32) {
			temp1 = SegPointCompare32(ruleset->ruleList[num].range[d2s][0], &range[1][1][4-number_bits/32]);
			temp2 = SegPointCompare32(ruleset->ruleList[num].range[d2s][1], &range[1][0][4-number_bits/32]);
		}
		else {
			temp1 = SegPointCompare_64_128(ruleset->ruleList[num].range[d2s][0], &range[1][1][4-number_bits/32]);
			temp2 = SegPointCompare_64_128(ruleset->ruleList[num].range[d2s][1], &range[1][0][4-number_bits/32]);
		}
		if ( temp1<=0 && temp2 >= 0)
		{
			tempRuleNumList[pos] = num;
			pos++;
		}
	}

	childRuleSet = (rule_set_t*) malloc(sizeof(rule_set_t));
	//printf("\nmalloc11 = %p\n",childRuleSet);
    childRuleSet->num = pos;
	childRuleSet->ruleList = (rule_t*) malloc( childRuleSet->num * sizeof(rule_t) );
    //printf("\nmalloc12 = %p\n",childRuleSet->ruleList);
    //printf("\nnum = %u\n",childRuleSet->num);
    num2++;
    mem_stat2 += childRuleSet->num * sizeof(rule_t);
    //sleep(20);
	for (num = 0; num < childRuleSet->num; num++) {
		Assign_RuleList(&childRuleSet->ruleList[num], &ruleset->ruleList[tempRuleNumList[num]]);
		/* printf("\n>>ruleset->num %u", tempRuleNumList[num]); */
		/* in d2s dim, the search space needs to be trimmed off */
		if (number_bits <= 32) {
			if (SegPointCompare32(childRuleSet->ruleList[num].range[d2s][0] ,&range[1][0][4-number_bits/32])<0)
				Assign(childRuleSet->ruleList[num].range[d2s][0],&range[1][0][4-number_bits/32],number_bits);
			if (SegPointCompare32(childRuleSet->ruleList[num].range[d2s][1], &range[1][1][4-number_bits/32])>0)
				Assign(childRuleSet->ruleList[num].range[d2s][1] ,&range[1][1][4-number_bits/32],number_bits);
		}
		else {
			if (SegPointCompare_64_128(childRuleSet->ruleList[num].range[d2s][0] ,&range[1][0][4-number_bits/32])<0)
				Assign(childRuleSet->ruleList[num].range[d2s][0],&range[1][0][4-number_bits/32],number_bits);
			if (SegPointCompare_64_128(childRuleSet->ruleList[num].range[d2s][1], &range[1][1][4-number_bits/32])>0)
				Assign(childRuleSet->ruleList[num].range[d2s][1] ,&range[1][1][4-number_bits/32],number_bits);
		}
	}
    //printf("\nhs free add5 = %p\n",tempRuleNumList);
	//printf("\nhs free add5\n");
    free(tempRuleNumList);
    tempRuleNumList = NULL;
#ifdef DEBUGv2
    printf("\n>>depth #%d, 2subrules# %d:",depth+1, num);
    for(i=0;i<num;i++)printf("%d ", childRuleSet->ruleList[i].pri);
#endif
    BuildHSTree(childRuleSet, currNode->child[1], depth+1);
	release_ruleset(childRuleSet);
    return	HS_SUCCESS;
}




/*****************************************************************************/
/* Function     : LinearSearch                                      */
/* Description  :  linear search for packets; Brute force algorithm, compare each dim respectively*/
/*                Input: num_rule and rule_pri in current nodes           */
/*                Return: num_matched rules and matched_rule_pri        */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/  
int LinearSearch(rule_set_t* ruleset, uint32_t num_rule, uint32_t* rule_pri, uint32_t packet[DIM][4], uint32_t* num_matched, uint32_t* matched_rule_pri){
    uint32_t i;
    *num_matched = 0;
    int flag = 0;
    for(i = 0; i < num_rule; i++){
        if(Matched(ruleset->ruleList[rule_pri[i]], packet) == 1){
            matched_rule_pri[*num_matched] = rule_pri[i];
            *num_matched = (*num_matched)+1;
            flag = 1;
        }
    }
    if(flag == 0){
        printf(">>err: LinearSearch: No matched Rules!");
        printf("\n");
        //ShowPacket(packet);
        printf("\nrules: ");
        for(i=0; i<num_rule;i++){
            printf("%d ", rule_pri[i]);
        }
        printf("\n");
        exit(-1);
    }
    return(1);
}

/*****************************************************************************/
/* Function     : Matched                                      */
/* Description  :  used in linear search to find out if the packet and the rule matched        */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/  
int Matched(rule_t rule, uint32_t packet[DIM][4]){
    int i;
    int temp1, temp2;
    for(i=0;i<DIM;i++){
        number_bits = BITNUMBER[FLAG[i]];
        temp1 = SegPointCompare_64_128(&packet[i][4-(number_bits>>5)], rule.range[i][0]);
        temp2 = SegPointCompare_64_128(rule.range[i][1], &packet[i][4-(number_bits>>5)]);
        if((temp1!=-1) && (temp2!=-1))continue;
        return(0);
    }
    return(1);
}

/*****************************************************************************/
/* Function     : LookupHSTtree                                             */
/* Description  :  test the hyper-split-tree with give 7-tuple packet        */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/ 
int	LookupHSTree(unsigned int packet[DIM][4] ,rule_set_t* ruleset, hs_node_t* root, hs_node_t** hitnode,unsigned int* pdepth)
{
   
    printf("\npacket[9] = %u,[10] = %u\n",((unsigned int)packet[9][3]),((unsigned int)packet[10][3]));
    unsigned int number_bits;
    if(!ruleset->num)
    {
        //printf("\nruleset not ready\n");
        return 0;
    }
    /*for (ruleNum = ruleset->num-1; ruleNum < ruleset->num; ruleNum ++) {*/
	if(!ruleset || !root || !hitnode)
    {
        printf("\nLookupHSTree erro\n");
        return 0;
    }

    hs_node_t*	node = root;
    int loop = 0;
        
    while (node->child[0] != NULL) {
            number_bits = BITNUMBER[FLAG[node->d2s]];
            loop++;
			//printf("\nthresh = %u %u %u %u\n",node->thresh[0],node->thresh[1],node->thresh[2],node->thresh[3]);
			printf("\nd2s = %u\n",node->d2s);
#ifdef DEBUGv2
			printf("\n>>depth=%u, packet[%u]=", node->depth, node->d2s);
			Show(packet[node->d2s], 1, 128);
			printf(" p2s=");
			Show(node->p2s,1, number_bits);
#endif
			if(SegPointCompare_64_128_nsf(&packet[node->d2s][4-number_bits/32], node->p2s,number_bits) <= 0) {
                node = node->child[0];
            }
			else
            {
				node = node->child[1];
            }
            printf("\nnode num = %u\n",node->num_rule);
		}
         
        //printf("loop = %u",loop);
        if(!node->rule)
        {
            printf("\nno rule\n");
            return 0;
        }
        
         //printf("\nzwz matched\n");
        *hitnode = node;
		
	return	HS_SUCCESS;
}


/*****************************************************************************/
/* Function     : LookupPacket                                             */
/* Description  :  Lookup single packet. if expect_flag = 1, means we have an expect rule.*/
/*                if expect_flag = 0, we do not have an expect rule match        */
/* Input        :                                                            */
/* Output       :                                                            */
/* return       :                                                            */
/* others       :                                                            */
/*****************************************************************************/ 
int LookupPacket(rule_set_t* ruleset, hs_node_t* root, uint32_t packet[DIM][4], hs_node_t** hitnode)
{
	int i;
	hs_node_t*	node = root;
    unsigned int number_bits;

    node = root;

    if(!ruleset || !root || !hitnode)
    {
        printf("\nLookupHSTree erro\n");
        return 0;
    }

    /*find leaf node*/
    while (node->child[0] != NULL) {
        number_bits = BITNUMBER[FLAG[node->d2s]];
#ifdef DEBUGv2
        printf("\n>>depth=%u, packet[%u]=", node->depth, node->d2s);
        Show(packet[node->d2s], 1, 128);
        printf(" p2s=");
        Show(node->p2s,1, number_bits);
#endif
        if(SegPointCompare_64_128_nsf(&packet[node->d2s][4-number_bits/32], node->p2s,number_bits) < 0) {
            node = node->child[0];
#ifdef DEBUGv2
            printf(" goto the LEFT child");
#endif
        }
        else {
            node = node->child[1];
#ifdef DEBUGv2
            printf(" goto the RIGHT child");
#endif
        }
    }
    //count_depth = count_depth+node->depth;
#ifdef DEBUGv2
    printf("\n");
    for(i=0;i<node->num_rule;i++)printf("%u ", node->rule[i]);
    printf("\n");
#endif
    /*find matched rules*/
    uint32_t num_matched;
    //printf("\nnum_matched = %u\n",node->num_rule);
    if(node->flag == 1){
        uint32_t* matched_rule_pri = (uint32_t*)malloc(sizeof(uint32_t)*node->num_rule);
        if(matched_rule_pri)
        {
            LinearSearch(ruleset, node->num_rule, node->rule, packet, &num_matched, matched_rule_pri);
            free(matched_rule_pri);
        }
    }
#ifdef DEBUGv2
    printf("\n");
    //for(i=0;i<num_matched;i++)printf("%u ", matched_rule_pri[i]);
    printf("\n");
#endif
    if(!node->rule)
    {
        printf("\nno rule\n");
        return 0;
    }
        
            //printf("\nzwz matched\n");
    *hitnode = node;
            /*printf("\nmatched rule: ");
            for(i=0;i<num_matched;i++){
                printf("\n%u ", matched_rule_pri[i]+1);
            }
            printf("\n");*/
    return(HS_SUCCESS);
}


static int _FreeRootNode(hs_node_t *rootnode, int depth);


int FreeRuleset(rule_set_t * ruleset) 
{
    //return HS_SUCCESS;
    uint32_t i,j;
    printf("\nfree ruleset = %p, ruleList add = %p\n",ruleset,ruleset->ruleList);
    if(ruleset && ruleset->ruleList) {
        //printf("\nenter\n");
        for(i = 0;i < ruleset->num;i++)
        {
            free(ruleset->ruleList[i].range[0][0]);
            printf("$$$$$$$$$$$$$$$$$$$$$$$ free add1 = %p $$$$$$$$$$$$$$$$$$$$$$$",ruleset->ruleList[i].range[0][0]);
            for(j = 0;j < DIM;j++)
            {
                if(ruleset->ruleList[i].range[j][0])
                {
                    //printf("$$$$$$$$$$$$$$$$$$$$$$$ free add1 = %p $$$$$$$$$$$$$$$$$$$$$$$",ruleset->ruleList[i].range[j][0]);
                    //free(ruleset->ruleList[i].range[j][0]);
                    ruleset->ruleList[i].range[j][0] = NULL;
                }
                if(ruleset->ruleList[i].range[j][1])
                {
                    //printf("$$$$$$$$$$$$$$$$$$$$$$$ free add2 = %p $$$$$$$$$$$$$$$$$$$$$$$",ruleset->ruleList[i].range[j][1]);
                    //free(ruleset->ruleList[i].range[j][1]);
                    ruleset->ruleList[i].range[j][1] = NULL;
                }
            }
            printf("\n");
        }
        printf("\nHS free = %p\n",ruleset->ruleList);
        free(ruleset->ruleList);
        ruleset->ruleList = NULL;
    }
    return HS_SUCCESS;
}

int FreeRootNode(hs_node_t *rootnode)
{
    _FreeRootNode(rootnode, 0);
    return HS_SUCCESS;
}

static int _FreeRootNode(hs_node_t *rootnode, int depth)
{
    //return HS_SUCCESS;
    if(rootnode == 0) {
        return HS_FAILURE;
    }
    
    if(depth == 0 && NULL == rootnode->child[0] && NULL == rootnode->child[1])
    {
        return HS_SUCCESS;
    }

    uint32_t i=0;
    for(; i<sizeof(rootnode->child)/sizeof(hs_node_t*); i++) {
        //printf("\ntesti = %u\n",i);
        if(rootnode->child[i] != 0) {
            _FreeRootNode(rootnode->child[i], depth+1);
        }
    }
    if(depth != 0 && rootnode) {
        if(rootnode->rule)
        {
            //printf("\nHS free = %p\n",rootnode->rule);
            //free(rootnode->rule);
            free(rootnode->rule);
        }
        
        if(rootnode->p2s)
        {
            //printf("\nHS free = %p\n",rootnode->p2s);
            //free(rootnode->p2s);
            free(rootnode->p2s);
        }

        //printf("\nHS free = %p\n",rootnode);
        //free(rootnode);
        free(rootnode);
    } else {
        if(rootnode->p2s)
        {
            //printf("\nHS free = %p\n",rootnode->p2s);
            //free(rootnode->p2s);
            free(rootnode->p2s);
        }
    }

    return HS_SUCCESS;
}

void ReadIPRange(uint32_t ipnet, uint32_t ipmask, unsigned int* IPrange, unsigned int* IPrange1)
{
	/*asindmemacces IPv4 prefixes*/
	/*temporary variables to store IP range */
	unsigned int trange[4];	
	unsigned int mask;
	int masklit1;
	unsigned int masklit2,masklit3;
	unsigned int ptrange[4];
	int i;
	
    mask = ipmask;
	trange[0] = ipnet>>24;
	trange[1] = ipnet>>16 & 0x00FF;
    trange[2] = ipnet>>8 & 0x0000FF;
	trange[3] = ipnet & 0x000000FF;
    
	mask = 32 - mask;
	masklit1 = mask / 8;
	masklit2 = mask % 8;
	
	for(i=0;i<4;i++)
		ptrange[i] = trange[i];

	/*count the start IP */
	for(i=3;i>3-masklit1;i--)
		ptrange[i] = 0;
	if(masklit2 != 0){
		masklit3 = 1;
		masklit3 <<= masklit2;
		masklit3 -= 1;
		masklit3 = ~masklit3;
		ptrange[3-masklit1] &= masklit3;
	}
	/*store start IP */
	IPrange[0] = ptrange[0];
	IPrange[0] <<= 8;
	IPrange[0] += ptrange[1];
	IPrange[0] <<= 8;
	IPrange[0] += ptrange[2];
	IPrange[0] <<= 8;
	IPrange[0] += ptrange[3];
	printf("%x\n", IPrange[0]);
	/*count the end IP*/
	for(i=3;i>3-masklit1;i--)
		ptrange[i] = 255;
	if(masklit2 != 0){
		masklit3 = 1;
		masklit3 <<= masklit2;
		masklit3 -= 1;
		ptrange[3-masklit1] |= masklit3;
	}
	/*store end IP*/
	IPrange1[0] = ptrange[0];
	IPrange1[0] <<= 8;
	IPrange1[0] += ptrange[1];
	IPrange1[0] <<= 8;
	IPrange1[0] += ptrange[2];
	IPrange1[0] <<= 8;
	IPrange1[0] += ptrange[3];
    printf("%x\n", IPrange1[0]);
}


bool load_rule(rule_list_t *rule_list,rule_set_t* ruleset, hs_node_t* node)
{
	uint32_t i = 0;
	int count = 0;
	unsigned int v = 0;
	
    struct FILTSET* filtset = (struct FILTSET*)malloc(sizeof(struct FILTSET));
    if(!filtset)
    {
        return false;
    }

    struct FILTER *tempfilt,tempfilt1;

    filtset->numFilters = 0;
    memset(filtset,0,sizeof(struct FILTSET));

	tempfilt = &tempfilt1;

    memset(tempfilt->dim,0,DIM*2*sizeof(int*));
    range_alloc(tempfilt->dim);

    if(ruleset == NULL || node == NULL)
    {
        printf("\nwrong parameters\n");
        free(filtset);
        return false;
    }

	
	for( i = 0; i < RULE_ENTRY_MAX; i++ )
	{
		if(rule_list->rule_entry[i].entry_status == RULE_ENTRY_STATUS_FREE)
		{
			continue;
		}

		count++;
		tempfilt->action = (unsigned int)rule_list->rule_entry[i].rule_tuple.action;
		tempfilt->id = i;
		tempfilt->rule_id = i;

		*tempfilt->dim[0][0] = rule_list->rule_entry[i].rule_tuple.smac[0] * 256 + rule_list->rule_entry[i].rule_tuple.smac[1];
		*tempfilt->dim[0][1] = rule_list->rule_entry[i].rule_tuple.smac[2]*16777216 + rule_list->rule_entry[i].rule_tuple.smac[3] * 65536 + rule_list->rule_entry[i].rule_tuple.smac[4] * 256 + rule_list->rule_entry[i].rule_tuple.smac[5];
		*tempfilt->dim[1][0] = rule_list->rule_entry[i].rule_tuple.dmac[0] * 256 + rule_list->rule_entry[i].rule_tuple.dmac[1];
		*tempfilt->dim[1][1] = rule_list->rule_entry[i].rule_tuple.dmac[2]*16777216 + rule_list->rule_entry[i].rule_tuple.dmac[3] * 65536 + rule_list->rule_entry[i].rule_tuple.dmac[4] * 256 + rule_list->rule_entry[i].rule_tuple.dmac[5];
		
		*tempfilt->dim[4][0] = rule_list->rule_entry[i].rule_tuple.sport_start;
		*tempfilt->dim[4][1] = rule_list->rule_entry[i].rule_tuple.sport_end;
		*tempfilt->dim[5][0] = rule_list->rule_entry[i].rule_tuple.dport_start;
		*tempfilt->dim[5][1] = rule_list->rule_entry[i].rule_tuple.dport_end;

		*tempfilt->dim[8][0] = rule_list->rule_entry[i].rule_tuple.protocol_start;
		*tempfilt->dim[8][1] = rule_list->rule_entry[i].rule_tuple.protocol_end;

		ReadIPRange(rule_list->rule_entry[i].rule_tuple.sip,rule_list->rule_entry[i].rule_tuple.sip_mask, tempfilt->dim[9][0], tempfilt->dim[9][1]);
		ReadIPRange(rule_list->rule_entry[i].rule_tuple.dip,rule_list->rule_entry[i].rule_tuple.dip_mask, tempfilt->dim[10][0], tempfilt->dim[10][1]);	

		
		memcpy(&(filtset->filtArr[filtset->numFilters]),tempfilt,sizeof(struct FILTER));
		filtset->numFilters++;

	}
    
    ruleset->num = filtset->numFilters;
    ruleset->ruleList = (rule_t*) malloc((ruleset->num+1) * sizeof(rule_t));
	
    memset(ruleset->ruleList,0,(ruleset->num + 1) * sizeof(rule_t));
    for (i = 0; i < ruleset->num; i++) 
	{
        ruleset->ruleList[i].pri = i;
        ruleset->ruleList[i].action = filtset->filtArr[i].action;
        ruleset->ruleList[i].id = filtset->filtArr[i].id;
        ruleset->ruleList[i].rule_id = filtset->filtArr[i].rule_id;

        for (v = 0; v < DIM; v++) {
            ruleset->ruleList[i].range[v][0] = filtset->filtArr[i].dim[v][0];
            ruleset->ruleList[i].range[v][1] = filtset->filtArr[i].dim[v][1];
        }
    }

    
    if(ruleset->num + 1 < RULE_ENTRY_MAX)
    {
        ruleset->ruleList[ruleset->num].pri = ruleset->num;
        ruleset->num++;
    }
    else
    {
        range_free(ruleset->ruleList[ruleset->num].range);
    }
    
    
    if(BuildHSTree(ruleset,node,0) != HS_SUCCESS)
    {
        free(filtset);
        return false;
    }
    
    range_free(tempfilt->dim);
   
    free(filtset);
    return true;
}




