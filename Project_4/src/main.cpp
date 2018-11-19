
#include <iostream>
#include <random>
#include <fstream>
#include <iomanip>
#include "analytical.h"
#include "metropolissampling.h"
#include <mpi.h>
#include <chrono>

using namespace std;
ofstream outfile;

void output( int dim, double T, double *ExpectVal, int MCcycles, double timing );
void output2( int dim, double T, double *ExpectVal, int MCcycles, double timing );

int main( int argc, char *argv[] )
{
    double *ExpectVal = new double[5];
    double *TotalExpectVal = new double[5];
    for( int i = 0; i < 5; i++ ) TotalExpectVal[i] = 0;

    //-------------------------------------------------------------------------
    //    Project 4c)
    string filename;
    int ordered = 1; //  Choose 1 for ordered matrix, choose 0 for random matrix
    // int dim = 100;   //  Dimension of the matrix L
    int MCcycles = 10;
    double InitialTemp = 1.0;
    double FinalTemp = 1.0;
    double TimeStep = 0.01;
    double timing;
    chrono::high_resolution_clock::time_point t1;
    chrono::high_resolution_clock::time_point t2;


    //  Initialize parallellization
    int nProcs;
    int my_rank;

    MPI_Init (&argc, &argv);
    MPI_Comm_size ( MPI_COMM_WORLD, &nProcs );
    MPI_Comm_rank ( MPI_COMM_WORLD, &my_rank );

    int cycleInterval = MCcycles/nProcs;
    int loopStart = my_rank*cycleInterval;
    int loopStop = (my_rank+1)*cycleInterval;
    t1 = chrono::high_resolution_clock::now();

    for(int dim = 2; dim < 3; dim+=2){

      if (my_rank == 0){
          outfile.open("HIGHRESdim" + to_string(dim) + "CriticalTemps" +  to_string(MCcycles) + "Cycles" + ".dat", std::ios_base::app);
          outfile << setw(15) << setprecision(8) << "T";
          outfile << setw(15) << setprecision(8) << "E";
          outfile << setw(15) << setprecision(8) << "M";
          outfile << setw(15) << setprecision(8) << "C_V";
          outfile << setw(15) << setprecision(8) << "chi" << endl;
        }//end initial print

      for ( double T = InitialTemp; T <= FinalTemp; T += TimeStep) {
          double acceptRatio = 0;

          MetropolisSampling( dim, MCcycles, loopStart, loopStop, T, ExpectVal, ordered, acceptRatio );
          for ( int i = 0; i < 5; i++ ) {
              MPI_Reduce(&ExpectVal[i], &TotalExpectVal[i], 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
          }
          if (my_rank==0) {
              output2( dim, T, TotalExpectVal, MCcycles, timing );
              cout << "T = " << T << " complete...\n";
          }
      }//end temp loop


        if(my_rank == 0){
          outfile.close();
          cout << dim << "x" << dim << "-lattice complete.." << endl;
        }
    }//end dim loop

    if(my_rank == 0){
      t2 = chrono::high_resolution_clock::now();
      chrono::duration<double> time_span = std::chrono::duration_cast<chrono::duration<double>>(t2 - t1);
      cout << "Program time: " << time_span.count() << endl;
    }
    MPI_Finalize ();
    if(my_rank == 0){
  }
    delete [] ExpectVal;
    delete [] TotalExpectVal;
    return 0;
}


void output( int dim, double T, double *ExpectVal, int MCcycles, double timing ) {

  for( int i = 0; i < 5; i++ ) ExpectVal[i] /= (MCcycles);

  double E_variance = (ExpectVal[1] - ExpectVal[0]*ExpectVal[0]);
  double M_variance = (ExpectVal[3] - ExpectVal[4]*ExpectVal[4]);

  // cout << "E_variance: " << E_variance << endl;
  // cout << "<E>: " << ExpectVal[0]/dim/dim << endl;

  outfile << setw(15) << setprecision(8) << T;                   //Temperature
  outfile << setw(15) << setprecision(8) << ExpectVal[0];        //Mean energy
  outfile << setw(15) << setprecision(8) << ExpectVal[3];        //Mean abs magnetization
  outfile << setw(15) << setprecision(8) << E_variance/T/T;       //Heat capaticy
  outfile << setw(15) << setprecision(8) << M_variance/T << endl;  //Susceptibility
}

void output2( int dim, double T, double *ExpectVal, int MCcycles, double timing ) {

  for( int i = 0; i < 5; i++ ) ExpectVal[i] /= (MCcycles);

  double E_variance = (ExpectVal[1] - ExpectVal[0]*ExpectVal[0]);
  double M_variance = (ExpectVal[3] - ExpectVal[4]*ExpectVal[4]);

  // cout << "E_variance: " << E_variance << endl;
  // cout << "<E>: " << ExpectVal[0]/dim/dim << endl;

  cout << setw(15) << setprecision(8) << T;                   //Temperature
  cout << setw(15) << setprecision(8) << ExpectVal[0];        //Mean energy
  cout << setw(15) << setprecision(8) << ExpectVal[2];        //Mean abs magnetization
  cout << setw(15) << setprecision(8) << E_variance/T/T;       //Heat capaticy
  cout << setw(15) << setprecision(8) << M_variance/T << endl;  //Susceptibility
}
