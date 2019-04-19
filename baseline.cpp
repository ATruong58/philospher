/*
	Name: Alan Truong
	Name: Lanchau Letran
	Class: Cs 3800 section B
*/

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <cerrno>
#include <unistd.h>
#include "mpi.h"
#include "pomerize.h"

//run compiled code (for 5 philosophers) with mpirun -n 5 program
using namespace std;

//Function to write out the stanza
void write(ofstream &foutLeft, ofstream& foutRight, pomerize &P, int id)
{
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

	return;
}

//this is how many poems you want each Phil to construct &amp; save
const int MAXMESSAGES = 10; 

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

	//If first run through no recieve and even id
	if(id%2 == 0 && firstIteration && id != p-1)
	{
		write(foutLeft, foutRight, P, id);
		//If the total number of philosopher is even send to odd 
		if(isEven)
		{
			MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rightNeighbor, tag );
			MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, leftNeighbor, tag );
		}
		//If the total number of philospher is odd
		else
		{
			//If it is the first id send only to right neighbor
			if(id == 0)
			{
				MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rightNeighbor, tag );
			}
			//else all even will send to left and right 
			else
			{	
				MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rightNeighbor, tag );
				MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, leftNeighbor, tag );
			}
		}
		//set falseIteration to false to indicate first run is over
		firstIteration = false;
	}
	//if it is not firstIteration and there is an even philosopher 
	else if(isEven)
	{	
		//recieve from the odd id
		MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, rightNeighbor, tag, status );
		MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, leftNeighbor, tag, status );
		
		//write the stanza
		write(foutLeft, foutRight, P, id);
		
		//send signal to odd
		MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rightNeighbor, tag );
		MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, leftNeighbor, tag );
	}
	//if it is not firstIteration and there is an oddphilosopher 
	else
	{
		//variable for last id 
		int rosa = p - 1;

		//if we are on even ids and not the last id
		if(id%2 == 0 && id != rosa)
		{
			//if it is first id
			if(id == 0)
			{	
				//recieve from last id
				MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, rosa, tag, status );
				
				write(foutLeft, foutRight, P, id);

				//write to only right neighbor
				MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, 1, tag );
			}
			else
			{	
				//recieve from last id 
				MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, rosa, tag, status );
				
				write(foutLeft, foutRight, P, id);
				
				//even will right to right and left neighbor
				MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rightNeighbor, tag );
				MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, leftNeighbor, tag );
			}
		}
		//else if id is odd
		else if(id%2 != 0)
		{
			//the second to last id
			if(id == rosa-1)
			{
				//recieve from left neighbor only
				MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, id-1, tag, status );
				
				write(foutLeft, foutRight, P, id);
				
				//write to the last id
				MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rosa, tag );
			}
			//if all other odd ids
			if(id != rosa-1)
			{
				//recieve from left and right neighbors
				MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, rightNeighbor, tag, status );
				MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, leftNeighbor, tag, status );
				
				write(foutLeft, foutRight, P, id);
				
				//send to last id
				MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, rosa, tag );
			}
			
		}
		//if last id
		else if(id == rosa)
		{
			//recieve from odd even id
			for (int i = 0; i < p; i++)
			{
				if(i%2 != 0)
				{
					MPI::COMM_WORLD.Recv ( &msgIn, 1, MPI::INT, i , tag, status );
				}
			}

			write(foutLeft, foutRight, P, id);

			//send to every even id
			for (int i = 0; i < p; i++)
			{
				if(i%2 == 0 || i != rosa)
				{
					MPI::COMM_WORLD.Send ( &msgOut, 1, MPI::INT, i, tag );
				}
			}
		}
	}
    numWritten++;
  }

  foutLeft.close();
  foutRight.close();
  //  Terminate MPI.
  MPI::Finalize ( );
  return 0;
}
