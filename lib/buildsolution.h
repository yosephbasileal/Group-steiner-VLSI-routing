/*void drawEdge(int i, int j, int V, int * S, int * G) {
        S[i * V + j] = G[i * V + j];
        S[j * V + i] = G[j * V + i];
}

int getPath(int i, int j, int V, int * path, int * P, int * G) {
        int count = 0;
        int final = reconstruct_path(V, i,j, P, G,path,&count);
       	return count;
}

int buildSolution(int minRoot, int V, int numGroups, int * D, int * onestar,struct SolutionTree * solutionTree) {
	int TOTAL_COST;

	int * intermSet = (int *) malloc(sizeof(int) * (V + 1)); 
	int * groupIds = (int *) malloc(sizeof(int) * numGroups);
	int * newGroupIds = (int *) malloc(sizeof(int) * numGroups); 
	int * partialStar1 = (int *) malloc(sizeof(int) * (2 + numGroups) * V);

	if(!intermSet || !groupIds || !newGroupIds || !partialStar1) {
		printf("Error: Memory allocation error inside twostar wrapper!\n");
		exit(1);
	}

	int remGroups;
	int root = minRoot;

	int count = twostar(root,groupIds,onestar,numGroups,remGroups,V,D, partialStar1,intermSet,newGroupIds,true);		
	
	//Store solution
	solutionTree->root = minRoot;
	solutionTree->numPar = count;
	solutionTree->partialstars = partialStar1;
	

	free(intermSet);
	free(groupIds);
	free(newGroupIds);
	
	return count;
}

void buildWrapper(struct Solution minSolution, int V, int numGroups, int * P,int * G, int * D, int * onestar) {
	// variables
	struct SolutionTree solutionTree;
	int * partialStar1;
	int * S = (int *) malloc(sizeof(int) * V * V);

	//initialize graph to INF
	for(int i = 0; i < V; i++) {
		for(int j = 0; j < V; j++) {
			S[i * V + j] = INF;
		}
	}

	//construct two star
	int count = buildSolution(minSolution.root,V,numGroups,D,onestar,&solutionTree);
	int root = minSolution.root;	

	//output	
	partialStar1 = solutionTree.partialstars;
	
	printSolutionCost(minSolution.root,minSolution.cost);
	printPartialStars(partialStar1,numGroups,solutionTree.numPar);
	
	int * path = (int *) malloc(sizeof(int) * 2 * V);
	for(int i = 0; i < count; i++) {
		int * curStar = partialStar1 + (i * (2 + numGroups));
		int c = getPath(root,curStar[0],V,path,P,G);
		for(int j = 0; j < c; j++) {
			drawEdge(path[j * 2 + 0], path[j * 2 + 1], V, S, G);
		}

		for(int j = 0; j < curStar[1]; j++) {
			int terminal = groups[
			int cc = getPath(curStar[0], curStar[2 + j],V,path,P,G);
			for(int k = 0; k < cc; k++) {
				drawEdge(path[j * 2 + 0], path[j * 2 + 1], V, S, G);
			}
		}
	}
	print(S,V);
}*/