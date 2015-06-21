//headers
#include "mpi.h"
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <limits.h>
#include <getopt.h>
#include <utmpx.h>


//constant
#define INF 1061109567

//File headers
#include "utils.h"
#include "readFile.h"
#include "onestar.h"
#include "twostar.h"
#include "floydWarshall.h"
#include "floydSerial.h"

//Global variables
bool gprint = false; // print graph and metric closure
bool debug = false;	// print more deatails for debugging
bool parallel = false; //construct metric closure in serial

//main
int main(int argc, char *argv[])
{	
	//initialize MPI variables
	int numProc, procId;

	MPI_Status status;
	MPI_Request request;
	
	MPI_Init(&argc,&argv);  
	MPI_Comm_size(MPI_COMM_WORLD,&numProc);  
	MPI_Comm_rank(MPI_COMM_WORLD,&procId);
	
	//graph variables
	unsigned int V, E, numTer, numGroups;
	int *D, *G, *term, *groups, *D_sub, *onestar, *onestar_sub;
	
	int MINIMUM, overall_min;
	
	int *pars;
	int perParent;
	int perChild;
	
	double starttime, endtime;	
	
	printf( "prooocId = %d   cpu = %d\n", procId, sched_getcpu() );

	//parent process
	if(!procId)  {
		int r;
		while ((r = getopt(argc, argv, "odp")) != -1) { //command line args
			switch(r)
			{
				case 'o':
					gprint = true;
					break;
				case 'd':
					debug = true;
					break;
				case 'p':
					parallel = true;
					break;
				default:
					//printUsage();
					exit(1);
			}
		}
		
		//read from file and allocate memory
		readFile(&D, &G, &term, &groups, &V, &E, &numTer, &numGroups);
	
		//broadcast size variables
		MPI_Bcast(&V, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&numTer, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&numGroups, 1, MPI_INT, 0, MPI_COMM_WORLD);

		//buffer for combined onestar cost matrix
		onestar = (int *) malloc(sizeof(int) * V * numGroups);

		//broadbast groups
		MPI_Bcast(groups, numTer, MPI_INT, 0, MPI_COMM_WORLD);
	
		//validate number of processes
		if(!validNumProc(V, numProc)) {
			printf("Error: More number of compute nodes than needed.\n");
			MPI_Finalize();
			return 0;
		}

		//calculate roots per process
		pars = calcLaunchPar(numProc, V);
		perParent = pars[1];
		perChild = pars[0];
		printf("p: %d   c: %d \n", perParent, perChild);

		
		if(debug) {	
			printf("Number of vertices: %d\n", V);
			printf("Parent process gets: %d\n", pars[1]);
			printf("%d child processes get: %d\n", numProc - 1, pars[0]);
		}

		//construct metric closure
		if(parallel) {
			printf("parallel floyd warshal\n");
			fw_gpu(V, G, D);
		} else{
			printf("serial floyd warshal\n");
			floydWarshall(V, G, D);
		}

		//broadcast metric closure
		MPI_Ibcast(D,V*V, MPI_INT, 0, MPI_COMM_WORLD,&request);

		//output metric closure
		if(gprint) {
			print(D,V);
			printTermGroups(numTer,numGroups,groups,term);
		}
		//reciveing buffer for distributing the metric closure
		D_sub = (int *) malloc(sizeof(int) * V * perChild);

		//buffer for sub onestar matrix in each process
		onestar_sub = (int *) malloc(sizeof(int) * perChild * numGroups);

		//construct one star
		onestarWrapper(V,numTer,perChild,perParent,numProc,procId,numGroups,D,D_sub,onestar,onestar_sub,groups);
		//printf( "procId = %d   cpuuuuuuuuu = %d\n", procId, sched_getcpu() );
		//broadbast onestar
		MPI_Bcast(onestar, V * numGroups, MPI_INT, 0, MPI_COMM_WORLD);
		
		//output onestar
		if(debug) printOnestar(onestar,numGroups,V);
		
		//construct two star
		//MINIMUM = twostarwrapper(V,numGroups,perChild,perParent,numProc,procId,D,onestar);					
		//struct Solution * solution = malloc(2 * sizeof(int));
		struct Solution solution;
		twostarwrapper(V,numGroups,perChild,perParent,numProc,procId,D,onestar,&solution);
		//printf("ProcId: %d   cost: %d root: %d  cpu: %d\n", procId, solution.cost, solution.root, sched_getcpu());
		//MINIMUM = solution.cost;
		//get minimum of all
		struct Solution minSolution;
		MPI_Reduce(&solution,&minSolution,1,MPI_2INT,MPI_MINLOC,0,MPI_COMM_WORLD);
		//MPI_Reduce(&MINIMUM,&overall_min,1,MPI_2INT,MPI_MINLOC,0,MPI_COMM_WORLD);
		//MPI_Allreduce(&MINIMUM,&overall_min,1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);

		printf("\nOVERALL MINIMUM STEINER COST: %d Root: %d\n", minSolution.cost, minSolution.root);
		//printf("\nOVERALL MINIMUM STEINER COST: %d\n", overall_min);

		buildPrintSolution(minSolution.root,V,numGroups,D,onestar);
	}//end parent process
	
	
	//child processes
	if(procId) {
		//printf("I am a child___________________________\n");
		//broadcast size variables
		MPI_Bcast(&V, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&numTer, 1, MPI_INT, 0, MPI_COMM_WORLD);
		MPI_Bcast(&numGroups, 1, MPI_INT, 0, MPI_COMM_WORLD);

		//allocate memory
		D = (int *) malloc(sizeof(int) * V * V);
		groups = (int *) malloc (sizeof(int) * numTer);

		//buffer for combined onestar cost matrix
		onestar = (int *) malloc(sizeof(int) * V * numGroups);

		//broadbast groups
		MPI_Bcast(groups, numTer, MPI_INT, 0, MPI_COMM_WORLD);

		//validate number of processes
		if (!validNumProc(V, numProc)) {
			MPI_Finalize();
			return 0;
		}

		//calculate number of roots per process
		pars = calcLaunchPar(numProc, V);
		perParent = pars[1];
		perChild = pars[0];

		//broadcast metric closure
		MPI_Ibcast(D,V*V, MPI_INT, 0, MPI_COMM_WORLD,&request);

		//buffer for reciving rows of metric closure
		D_sub = (int *) malloc(sizeof(int) * V * perChild);

		//buffer for sub onestar matrix in each process
		onestar_sub = (int *) malloc(sizeof(int) * perChild * numGroups);

		//construct one star
		onestarWrapper(V,numTer,perChild,perParent,numProc,procId,numGroups,D,D_sub,onestar,onestar_sub,groups);
		//printf( "procId = %d   cpuuuuuu = %d\n", procId, sched_getcpu() );
		//recieve onestar
		MPI_Bcast(onestar, V * numGroups, MPI_INT, 0, MPI_COMM_WORLD);

		//construct two star
		//MINIMUM = twostarwrapper(V,numGroups,perChild,perParent,numProc,procId,D,onestar);					
		//struct Solution *solution = malloc(2 * sizeof(int));
		struct Solution solution;
		//twostarwrapper(V,numGroups,perChild,perParent,numProc,procId,D,onestar,solution);
		twostarwrapper(V,numGroups,perChild,perParent,numProc,procId,D,onestar,&solution);		
		//MINIMUM = solution.cost;
		//get minimum of all
		//printf("ProcId: %d   cost: %d root: %d  cpu: %d\n", procId, solution.cost, solution.root, sched_getcpu());
		//struct Solution *minSolution = malloc(2 * sizeof(int));
		struct Solution minSolution;
		MPI_Reduce(&solution,&minSolution,1,MPI_2INT,MPI_MINLOC,0,MPI_COMM_WORLD);		
		//MPI_Allreduce(&MINIMUM,&overall_min,1,MPI_INT,MPI_MIN,MPI_COMM_WORLD);
	}//end child processes		
	//printf( "procId = %d   cpu = %d\n", procId, sched_getcpu() );
	MPI_Finalize();
	return 0;
}
