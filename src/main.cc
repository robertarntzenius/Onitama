/* Onitama (main.cc)
   C++ program that plays Onitama

   Call program with:
   ./onitama -f FILENAME [-p] [-1 [0|1]] [-2 [0|1]]

   -f      : file with card data
   -p      : print boards
   -1 type : set player type of player 1
   -2 type : set player type of player 2

   type can be 0 for RandomPlayer
               1 for MonteCarlosPlayer

   code by: Robert Arntzenius - s2046652
*/

#include <iostream>
#include <getopt.h>

#include "onitama.h"


/* Flag for printing boards */
bool gPrintFlag = false;
int  gRepeats   = 10000;
int  gPlayouts  = 1000;
int  gMaxTurns  = 100;


int main ( int argc, char *argv[] )
{ int opt;

  int wincount  = 0,
      movecount = 0;

  Onitama * onitama = NULL,
          * onicopy = NULL;


/* TODO close file */

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
              << argv[0] << " -f FILENAME [-p] [-1 [0|1]] [-2 [0|1]]" << std::endl
              << "\t-f      : file with card data"                    << std::endl
              << "\t-p      : print boards"                           << std::endl
              << "\t-1 type : set player type of player 1"            << std::endl
              << "\t-2 type : set player type of player 2"            << std::endl;
    return -1;
  }

  // onitama->printCards ( );
  onicopy = new Onitama ( *onitama );

  if ( gPrintFlag )
  { onicopy->printBoard ( );
    onicopy->printCards ( );
  }

  for ( int i = 0; i < gRepeats; ++i )
  { do
    { if (onicopy->getTurn ( ) == BLUE)
        onicopy->MCMove ( gPlayouts, gMaxTurns );
      else
        onicopy->randomMove ( );

      if ( gPrintFlag || movecount / (i + 1) > 15 )
      { onicopy->printBoard ( );
        onicopy->printCards ( );
      }

      ++movecount;
    } while( !onicopy->wayOfTheStone ( ) && !onicopy->wayOfTheStream ( ) );


    /* Check winner */
    if ( onicopy->getTurn ( ) == RED )
      ++wincount;

    std::cout << "\tI've won (" << wincount << "/" << i+1 << ") games already!" << std::endl;

    /* Reset positions */
    *onicopy = *onitama;
  }

  std::cout << "Blue won " << wincount << "/" << gRepeats << " times with an average of " << movecount / gRepeats << " moves per game" << std::endl;

  delete onitama;
  delete onicopy;

  return 0;
}
