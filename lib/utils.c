/**
 * File: utils.c
 *
 * This is a collection of utility functions used by twostar-mpi
 *
 * @author Basileal Imana
 * @version 
 */

//Libraries
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

//Library for sched_getcpu()
#define _GNU_SOURCE 
#include <utmpx.h>

//Header
#include "macros.h"
#include "utils.h"


/** Validates number of processes
 *
 * @param V total number of vertices
 * @param numProc total number of processes launched
 * @return true if numProc is greater than zero and less than V
 */
bool validNumProc(int V, int numProc) {
	return (numProc > 0 && numProc <= V);
}


/** Calculates how many roots each proc gets
 * 	 
 * @param numProc total number of processes launched 
 * @param V total number of vertices in graph
 * @return number of vertices per parent and per child in an array
 */
int* calcLaunchPar(int numProc, int V) {
	//par[0] = perChild, par[0] = perParent
	int* par = (int *) malloc(sizeof(int) * 2);
	
	int d = V/numProc; //difference
	int r = V%numProc; //remainder
	
	if (r == 0) {
		par[0] = d;
		par[1] = d;
	
	} else {
		par[0] = d;
		par[1] = d + r;
	}

	return par;
}


/** Calculates the ID of process that handled a given root when constructing onestar or twostar
 * 	 
 * @param root rootId
 * @param perChild number of processes assigned per child
 * @param perParent number of processes assigned per parent
 * @param V total number of vertices in graph
 * @return ID of process that handler the given root 
 */
int getProcId(int root, int perChild, int perParent, int numProc, int V) {
	int diff = perParent - perChild; //after the first diff number of roots (which are assigned to proc 0) round robin is used 
	int procId = 0;
	if(root < diff) {
		procId = 0;
	}
	else {
		int i = (root - diff)/(numProc); //get in which round of the round robin process this root get assigned to a process
		procId = root - ((i * numProc) + diff); //rearrange root = (perParent - perChild) + (i * numProc) + procId, taken from onestarwrapper function
	}	
	return procId;
}


/** Prints graph, metric closure and predessors matrices
 * 
 * @param G graph/matrix of size VxV
 * @param V size of matrix
 * @param name name of Graph to print
 */
void print(int* G, int V, char* name) {
	printf("%s: \n\t", name);
	for(int i = 0; i < V; i++, printf("\n\t")) {
		for(int j = 0; j < V; j++) {
		   int out = G[i * V + j];
		   if(out  == INF)
		      printf("%3s " , "INF");
		   else
		      printf("%3d " , out );
		}
	}
	printf("\n");
}


/** Print terminal vertices
 *
 * @param numTer number of terminals vertices in graph
 * @param terminals array of terminal vertices
 */
void printTerm(int numTer, int* terminals) {
	printf("\nTerminal vertices: \n");

	printf("\t# of vertices: %d  ",numTer);
	printf("List of vertices: ");

	for(int i = 0; i < numTer; i++) {
		printf("%d ", terminals[i]);
	}
	printf("\n");
}

/** Print groups
 *
 * @param numGroups number of groups in graph
 * @param numTer number of terminals vertices in graph
 * @param groups array of groups
 */
void printGroups(int numGroups, int numTer, int* groups) {
	printf("\nGroups: \n");

	printf("\tNum of groups: %d\n",numGroups);
	printf("\tPer group: %d\n",numTer/numGroups);

	for(int i = 0; i < numGroups; i++) {
		printf("\t\tGroup %d:", i);
		for(int j = 0; j < numTer/numGroups; j++) {
			printf(" %d ", groups[(i * (numTer/numGroups)) + j]);
		}
		printf("\n");
	}
	printf("\n");
}


/** Print partial stars
 *
 * @param partialstars an array of partial stars
 * @param numGroups number of groups in graph
 * @param count number of partial stars
 */
void printPartialStars(int* partialstars, int numGroups, int count) {
	printf("\nParital stars: \n");

	for(int i = 0; i < count; i++) {
		int* curStar = partialstars + (i * (2 + numGroups));
		printf("\tIntermediate: %d # of Groups: %d Group IDs ",curStar[0],curStar[1]);
		for(int j = 0; j < curStar[1]; j++) {
			printf(" %d ", curStar[j+2]);
		}
		printf("\n");
	}
	printf("\n");
}


/** Print one star
 *
 * @param onestar array of root to group costs
 * @param numGroups number of groups in graph
 * @param V number of vertices in graph
 * @param name name to print
 */
void printOnestar(int* onestar, int numGroups, int V, char* name) {
	printf("\n%s: \n", name);
	
	for(int i = 0; i < V; i++) {
		printf("\tRoot%d:  ", i);
		for(int j = 0; j < numGroups; j++) {
		  printf("%3d  ", onestar[i * numGroups + j]);
		}		 
		printf("\n");
	}
}


/** Print two star cost
 *
 * @param root root ID of two star
 * @param cost cost of two star
 */
void printTwoStarCost(int root, int cost) {
	printf("\nTWOSTAR: \n");
	printf("\tCost: %d Root: %d\n", cost, root);
}


/** Print ID of CPU current proces is running on
 *
 * @param procId process ID of current processs
 */
void printCpuID(int procId) {
	printf("Process ID = %d CPU ID = %d\n", procId, sched_getcpu());
}


/** Calculates total cost of a given graph
 *
 * @param G graph of size VxV
 * @param V size of graph
 * @return total graph cost
 */
int caclGraphCost(int* G, int V) {
	int sum = 0;
	int curr;
	for(int i = 0; i < V; i++) {
		for(int j = 0; j < V; j++) {
			if (i <= j) { //graph is undirected symmetric , just count upper or lower triangle
				continue;
			}
			int curr = G[i * V + j]; //get G[i][j]
			if(curr  == INF) { //if not edge between i and j, continue
			  continue;
			}
			sum += curr; //add to sum
		}
	}
	return sum;
}


/** Checks if a vertex is a terminal vertex
 *
 * @param v vertex in question
 * @param numTer number of terminal vertices
 * @param terminals array of all terminal vertices
 * @return 1 if is terminal and 0 if is non-terminal
 */
int isTerminal(int v, int numTer, int* terminals) {
	for(int i = 0; i < numTer; i++)
		if(v == terminals[i])
			return 1;
	return 0;
}


/** Counts number of non-terminals part of a solution graph
 *
 * @param G graph of size VxV
 * @param V size of graph
 * @param numTer number of terminal verices
 * @param terminals number of terminals
 * @return total count of terminals vertices in solution graph G
 */
int countNonTerminals(int* G, int V, int numTer, int* terminals) {
	int count = 0;
	for(int i = 0; i < V; i++) { //for each vertex 'i' in the graph
		if(isTerminal(i,numTer,terminals)) { //if is known to be terminal skip 
			continue;
		}
		int edge = 0;
		for(int j = 0; j < V; j++) { //for each vetex 'j' in i'th row 
			if(G[i * V + j] != INF) { //if atlease one edge then count
				edge = 1;
				break;
			}
		}
		if(edge == 1) {
			count++;
		}
	}
	return count;
}


/** Copies a partial stars form A to B
 *
 * @param A partial stars array to copy from
 * @param V partial stars array to copy to
 * @param numGroups number of groups in graph
 * @param count number of parital stars in A
 */
void copypartialStar(int* A, int* B, int numGroups, int count) {
	//A - from, B - to
	for(int i = 0; i < count; i++) { //for each partial star
		//get a partial star
		int * curStarA = A + (i * (2 + numGroups));
		int * curStarB = B + (i * (2 + numGroups));

		//copy intermediate and num of groups
		curStarB[0] = curStarA[0];
		curStarB[1] = curStarA[1];

		//copy groups
		for(int j = 0; j < curStarA[1]; j++) {
			curStarB[j+2] = curStarA[j+2];
		}
	}
}