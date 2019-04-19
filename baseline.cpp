#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include "pomerize.h"

//run compiled code (for 5 philosophers) with mpirun -n 5 program

using namespace std;

//this is how many poems you want each Phil to construct & save
const int MAXMESSAGES = 20; 

//if you change this base, update the Makefile "clean" accordingly
const string fileBase = "outFile"; 

int main ( int argc, char *argv[] ) 
{
  int id; //my MPI ID
  int p;  //total MPI processes
  MPI::Status status;
  int tag = 1;
  
  //  Initialize MPI.
  MPI::Init ( argc, argv );

  //  Get the number of processes.
  p = MPI::COMM_WORLD.Get_size ( );

  //  Determine the rank of this process.
  id = MPI::COMM_WORLD.Get_rank ( );
  
  //Safety check - need at least 2 philosophers to make sense
  if (p < 2) {
	    MPI::Finalize ( );
	    std::cerr << "Need at least 2 philosophers! Try again" << std::endl;
	    return 1; //non-normal exit
  }

  srand(id + time(NULL)); //ensure different seeds...

  int numWritten = 0;
  
  //setup message storage locations
  int msgIn, msgOut;
  int leftNeighbor = (id + p - 1) % p;
  int rightNeighbor = (id + 1) % p;
  bool firstIteration = true;
  bool isEven = false;
  pomerize P;

  if(p%2 == 0){
	  isEven = true;
  }

  string lFile = fileBase + to_string(id);
  string rFile = fileBase + to_string(rightNeighbor);
  ofstream foutLeft(lFile.c_str(), ios::out | ios::app );
  ofstream foutRight(rFile.c_str(), ios::out | ios::app );

  while (numWritten < MAXMESSAGES) {
	msgOut = rand() % p;

	if(!firstIteration)
	{
		if(id != 0 && id != p-1)
		{
			MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, id+1, tag, status );
			MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, id-1, tag, status );
		}
		else if(id ==0)
		{
			MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, id+1, tag, status );
                        MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, p-1, tag, status );
		}
		else
		{
			MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, 0, tag, status );
                        MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, id-1, tag, status );
		}
	}
	if(isEven)
	{
		MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rightNeighbor, tag );
		MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, leftNeighbor, tag );
	}
	else
	{
		int rosa = p - 1;
		if(id%2 == 0)
		{
			MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rightNeighbor, tag );
                	MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, leftNeighbor, tag );
		}
		else if(id%2 == 1 && id != rosa)
		{
			MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rosa, tag );
		}
		else{
			for(int i = 0; i < p; i++)
			{
				if(i%2 ==0)
					MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, i, tag );
			}
		}

	}
	foutLeft << id << "'s poem:" << endl;
	foutRight << id << "'s poem:" << endl;
	
	string stanza1, stanza2, stanza3;
    stanza1 = P.getLine();
	foutLeft << stanza1 << endl;
    foutRight << stanza1 << endl;

	stanza2 = P.getLine();
	foutLeft << stanza2 << endl;
    foutRight << stanza2 << endl;

	stanza3 = P.getLine();
	foutLeft << stanza3 << endl << endl;
    foutRight << stanza3 << endl << endl;

    firstIteration = false;
    numWritten++;
  }

  foutLeft.close();
  foutRight.close();
  
  //  Terminate MPI.
  MPI::Finalize ( );
  return 0;
}
