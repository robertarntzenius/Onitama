/* Onitama (main.cc)
   C++ program that plays Onitama

   Call program with:
   ./onitama -f FILENAME [-p] [-n <int>] [-r <int>] [-m <int>]

   -f      : file with card data
   -p      : print boards
   -n      : number of games                              [default = 100]
   -r      : number of playouts (MC)                      [default = 1000]
   -m      : maximum number of moves in random games (MC) [default = 100]

   type can be 0 for RandomPlayer
               1 for MonteCarlosPlayer

   code by: Robert Arntzenius - s2046652
*/

#include <iostream>
#include <getopt.h>

#include "onitama.h"


bool gPrintFlag = false;
int  gRepeats   = 100;


int main ( int argc, char *argv[] )
{ int opt;

  uint wincount  = 0,
       movecount = 0,
       totalmovecount = 0;

  Onitama * onitama = NULL,
          * onicopy = NULL;

  /* Look for optional operators */
  while ( ( opt = getopt ( argc, argv, "f:n:m:r:p" ) ) != -1 )
  { /* Handle single operator at a time */
    switch ( opt )
    { case 'f': /* Get filename for cards */
                onitama = new Onitama ( optarg );
                break;
      case 'n': /* Set number of games */
                gRepeats = atoi ( optarg );
                break;
      case 'm': /* Set maximum number of turns in random game */
                gMaxTurns = atoi ( optarg );
                break;
      case 'r': /* Set number of playouts */
                gPlayouts = atoi ( optarg );
                break;
      case 'p': /* Toggle print flag */
                gPrintFlag = true;
                break;
      default:  /* Faulty inputs */
                if (onitama != NULL)
                { delete onitama;
                  onitama = NULL;
                }
                goto out;
    }
  }

  out:

  if ( onitama == NULL )
  { std::cerr << "Please call program as follows:" << std::endl
              << argv[0] << " -f FILENAME [-p] [-n <int>] [-r <int>] [-m <int>]" << std::endl
              << "\t-f      : file with card data"                               << std::endl
              << "\t-p      : print boards"                                      << std::endl
              << "\t-n      : number of games"                                   << std::endl
              << "\t-r      : number of playouts (MC)"                           << std::endl
              << "\t-m      : maximum number of moves in random games (MC)"      << std::endl;

    return -1;
  }

  /* MCTS */
  // onitama->createMCTree ( );

  onicopy = new Onitama ( *onitama );

  for ( int i = 0; i < gRepeats; ++i )
  { movecount = 0;
    do
    { if ( gPrintFlag )
        onicopy->printBoard ( );

      if (onicopy->getTurn ( ) == BLUE)
        onicopy->MCTSMove ( );
        // onicopy->MCMove ( );
      else
        onicopy->randomMove ( );

      movecount++;

      if ( movecount >= gMaxTurns )
      {
        std::cout << "Turn limit reached!" << std::endl;

        onicopy->printBoard ( );

        if ( onicopy->getTurn ( ) == RED )
          wincount--;
        break;
      }

    } while( !onicopy->wayOfTheStone ( ) && !onicopy->wayOfTheStream ( ) );

    if ( gPrintFlag )
        onicopy->printBoard ( );
        
    totalmovecount += movecount;

    /* Check winner */
    if ( onicopy->getTurn ( ) == RED )
      wincount++;

    std::cout << "Blue won (" << wincount << "/" << i+1 << ") games already!" << std::endl;

    /* Reset positions */
    *onicopy = *onitama;
  }

  std::cout << "Blue won " << wincount << "/" << gRepeats << " times with an average of " << (float) totalmovecount / gRepeats << " moves per game" << std::endl;

  delete onicopy;

  delete onitama;

  return 0;
}
