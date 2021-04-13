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

extern float gExploration;

int main ( int argc, char *argv[] )
{ int opt;

  uint wincount  = 0,
       movecount = 0,
       totalmovecount = 0;

  char * filename = NULL;

  Onitama * onitama = NULL;

  srand(time(NULL));
  // srand(1234);

  /* Look for optional operators */
  while ( ( opt = getopt ( argc, argv, "f:n:m:r:p" ) ) != -1 )
  { /* Handle single operator at a time */
    switch ( opt )
    { case 'f': /* Get filename for cards */
                filename = optarg;
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

  if ( filename == NULL )
  { std::cerr << "Please call program as follows:" << std::endl
              << argv[0] << " -f FILENAME [-p] [-n <int>] [-r <int>] [-m <int>]" << std::endl
              << "\t-f      : file with card data"                               << std::endl
              << "\t-p      : print boards"                                      << std::endl
              << "\t-n      : number of games"                                   << std::endl
              << "\t-r      : number of playouts (MC)"                           << std::endl
              << "\t-m      : maximum number of moves in random games (MC)"      << std::endl;

    return -1;
  }

  for ( gExploration = 1.00; gExploration <= 2.00; gExploration += 0.01 )
  { wincount  = 0,
    movecount = 0,
    totalmovecount = 0;

    for ( int i = 0; i < gRepeats; ++i )
    { /* Create board */
      onitama = new Onitama ( filename );

      movecount = 0;

      do
      { if ( gPrintFlag )
          onitama->printBoard ( );

        if (onitama->getTurn ( ) == BLUE)
          onitama->MCTSMove ( );
        else
          onitama->MCMove ( );
          // onitama->randomMove ( );

        movecount++;

        // if ( movecount >= gMaxTurns )
        // {
        //   std::cout << "Turn limit reached!" << std::endl;
        //
        //   onitama->printBoard ( );
        //
        //   if ( onitama->getTurn ( ) == RED )
        //     wincount--;
        //   break;
        // }

      } while( !onitama->wayOfTheStone ( ) && !onitama->wayOfTheStream ( ) );

      if ( gPrintFlag )
          onitama->printBoard ( );

      totalmovecount += movecount;

      /* Check winner */
      if ( onitama->getTurn ( ) == RED )
        wincount++;

      // std::cout << "Blue won (" << wincount << "/" << i+1 << ") games already!" << std::endl;

      /* Delete board */
      delete onitama;
    }

    std::cout << "Exploration paramater: " << gExploration << "\t Blue won " << wincount << "/" << gRepeats << " times with an average of " << (float) totalmovecount / gRepeats << " moves per game" << std::endl;
  }

  return 0;
}
