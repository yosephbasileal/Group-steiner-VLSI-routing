#include "partialStar.h"
#include "remSpanned.h"

struct Solution {
	int cost;
	int root;
};

int twostar(int root,int *groupIds,int *onestar,int numGroups,int remGroups,int V, int *D, int *partialStar1,int *intermSet,int *newGroupIds) {		
	int TOTAL_COST = 0;
	remGroups = numGroups;
	for(int i = 0; i < numGroups;i++)
		groupIds[i] = i;
	intermSet[0] = 0;

	while(remGroups > 0) { //Find two star
		partialStar(root,groupIds,onestar,numGroups,remGroups, V, D + (root * V),partialStar1,intermSet); //find current partial star

		intermSet[0] += 1; //increase counter for number of intermediates
		intermSet[intermSet[0]] = partialStar1[0]; //save current intermediate

		for(int i = 0; i < partialStar1[1]; i++) {
			TOTAL_COST += onestar[(partialStar1[0] * numGroups) + partialStar1[i+2]]; //intermediate to groups cost
		}

		TOTAL_COST += D[root * V + partialStar1[0]]; //root to intermediate cost

		remGroups -= partialStar1[1]; //update number of remaining groups
		if(remGroups == 0) //if all groups spanned
			break;

		remSpanned(groupIds,partialStar1,newGroupIds,remGroups,numGroups); //remove those already spanned
		groupIds = newGroupIds;
		if (root == 116) {
			printf("cur total cost: %d\n", TOTAL_COST);
		}
	}//rooted two star loop
	
	return TOTAL_COST;
}

void twostarwrapper(int V, int numGroups, int perChild, int perParent, int numProc, int procId, int * D, int * onestar, struct Solution *solution) { 
	//struct Solution solution;
	int TOTAL_COST;
	int MINIMUM = INT_MAX;
	int minRoot = INT_MAX;

	//Allocate memory for 2-star
	int * intermSet = (int *) malloc(sizeof(int) * (V + 1)); //index 0 shows how many intermediates are in the set
	int * groupIds = (int *) malloc(sizeof(int) * numGroups); //keeps track of groups already spanned
	int * newGroupIds = (int *) malloc(sizeof(int) * numGroups); //helper buffer when modifying groupIds
	int * partialStar1 = (int *) malloc(sizeof(int) * (2 + numGroups));//[0] -> interm v,  [1] -> numGroups it spans, the rest are the groups V spans

	int remGroups; //number of groups not spanned yer
	int root;

	for(int i = 0; i < perChild; i++) {
		//printf("ProcId: %d\n", procId);
		root = (perParent - perChild) + (i * numProc) + procId;
		TOTAL_COST = twostar(root,groupIds,onestar,numGroups,remGroups,V,D, partialStar1,intermSet,newGroupIds);		
		if(TOTAL_COST < MINIMUM) { //update minimum
			if(procId==8) printf("rooooooot %2d Tot: %d\n",root, TOTAL_COST);
			MINIMUM = TOTAL_COST;
			minRoot = root;
		}
	}//all two star loop
	
	if (!procId) {
		for(int j = 0; j < (perParent - perChild); j++) {
			root = j;
			TOTAL_COST = twostar(root,groupIds,onestar,numGroups,remGroups,V,D, partialStar1,intermSet,newGroupIds);
			if(TOTAL_COST < MINIMUM) { //update minimum
				MINIMUM = TOTAL_COST;
				minRoot = root;
			}
		}//for any remaining roots - by proc 0 only	
	}
	
	solution->root = minRoot;
	solution->cost = MINIMUM;
	
	printf("\nMINIMUM STEINER COST: %d,   root: %d, proc ID: %d\n", MINIMUM, minRoot,procId);
	//return MINIMUM;
	//return solution;
}

void buildPrintSolution(int minRoot, int V, int numGroups, int * D, int * onestar) {
	//printf("minRoot: %d\n", minRoot);
	int TOTAL_COST = 0;
	int remGroups;
	int root = minRoot;
	//Allocate memory for 2-star
	int * intermSet = (int *) malloc(sizeof(int) * (V + 1)); //index 0 shows how many intermediates are in the set
	int * groupIds = (int *) malloc(sizeof(int) * numGroups); //keeps track of groups already spanned
	int * newGroupIds = (int *) malloc(sizeof(int) * numGroups); //helper buffer when modifying groupIds
	int * partialStar1 = (int *) malloc(sizeof(int) * (2 + numGroups) * V);//[0] -> interm v,  [1] -> numGroups it spans, the rest are the groups V spans

	remGroups = numGroups;
	for(int i = 0; i < numGroups;i++)
		groupIds[i] = i;
	intermSet[0] = 0;
	int c = 0; //count partial stars
	while(remGroups > 0) { //Find two star
		int * curParStar = partialStar1 + (c * (2 + numGroups));
		partialStar(root,groupIds,onestar,numGroups,remGroups, V, D + (root * V),curParStar,intermSet); //find current partial star
		intermSet[0] += 1; //increase counter for number of intermediates
		intermSet[intermSet[0]] = curParStar[0]; //save current intermediate

		for(int i = 0; i < curParStar[1]; i++) {
			TOTAL_COST += onestar[(curParStar[0] * numGroups) + curParStar[i+2]]; //intermediate to groups cost
		}

		TOTAL_COST += D[root * V + curParStar[0]]; //root to intermediate cost
		c++;

		remGroups -= curParStar[1]; //update number of remaining groups
		if(remGroups == 0) //if all groups spanned
			break;

		remSpanned(groupIds,curParStar,newGroupIds,remGroups,numGroups); //remove those already spanned
		groupIds = newGroupIds;
	}//rooted two star loop
	
	printf("Total Cost: %d\n",TOTAL_COST);
	for(int i = 0; i < c; i++) {
		int * curStar = partialStar1 + (i * (2 + numGroups));
		printf("intem: %d NumGr: %d IDs ",curStar[0],curStar[1]);
		for(int i = 0; i < curStar[1]; i++)
			printf(" %d ", curStar[i+2]);
		printf("\n");
	}	
}
