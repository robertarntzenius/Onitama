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

extern uint gPlayouts;
extern float gExploration;

extern uint gCycleCount;

extern uint studentHeatMaps[N_PLAYERS + 1][BOARDHEIGHT][BOARDWIDTH];
extern uint masterHeatMaps[N_PLAYERS + 1][BOARDHEIGHT][BOARDWIDTH];

int main ( int argc, char *argv[] )
{ int opt;

  int winner;

  uint wincount  = 0,
       tiecount = 0,
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

  // for ( gPlayouts = 1; gPlayouts <= 10000; gPlayouts *= 10 )
  // for ( gExploration = 0.4; gExploration < 2.0; gExploration += 0.2 )
  // { wincount  = 0,
    // movecount = 0,
    // totalmovecount = 0;

    for ( int i = 0; i < gRepeats; ++i )
    { /* Create board */
      onitama = new Onitama ( filename );

      movecount = 0;

      do
      { if ( gPrintFlag )
          onitama->printBoard ( );

        if (onitama->getTurn ( ) == BLUE)
          onitama->MCTSMove ( true, 100 );
          // onitama->MCMove ( );
          // onitama->randomMove ( );
        else
          onitama->MCTSMove ( true, 100 );
          // onitama->MCMove ( );
          // onitama->randomMove ( );

        // onitama->updateStats ( );

        movecount++;

        if ( movecount >= gMaxTurns )
        {
          // std::cout << "Turn limit reached!" << std::endl;
          //
          // onitama->printBoard ( );

          if ( onitama->getTurn ( ) == RED )
            wincount--;

          tiecount++;
          break;
        }

      } while( !onitama->wayOfTheStone ( ) && !onitama->wayOfTheStream ( ) );

      if ( gPrintFlag )
          onitama->printBoard ( );

      winner = ( onitama->getTurn ( ) == RED )? BLUE : RED;

      // if ( movecount < gMaxTurns )
      //   for ( int j = 0; j < BOARDHEIGHT; ++j )
      //     for ( int k = 0; k < BOARDWIDTH; ++k )
      //     { studentHeatMaps[N_PLAYERS][j][k] += studentHeatMaps[winner][j][k];
      //       studentHeatMaps[BLUE][j][k] = 0;
      //       studentHeatMaps[RED][j][k] = 0;
      //     }
      //
      // if ( movecount < gMaxTurns )
      //   for ( int j = 0; j < BOARDHEIGHT; ++j )
      //     for ( int k = 0; k < BOARDWIDTH; ++k )
      //     { masterHeatMaps[N_PLAYERS][j][k] += masterHeatMaps[winner][j][k];
      //       masterHeatMaps[BLUE][j][k] = 0;
      //       masterHeatMaps[RED][j][k] = 0;
      //     }

      totalmovecount += movecount;

      /* Check winner */
      if ( winner == BLUE )
        wincount++;

      std::cout << "MCTS won (" << wincount << "/" << i+1 << ") games with " << tiecount << " ties." << std::endl;

      /* Delete board */
      delete onitama;
    }

    std::cout << "MCTS won " << wincount << "/" << gRepeats << " times including " << tiecount << " ties with an average of " << (float) totalmovecount / gRepeats << " moves per game" << std::endl;
    // std::cout << "Exploration parameter: " << gExploration << " - MCTS won " << wincount << "/" << gRepeats << " times including " << tiecount << " ties with an average of " << (float) totalmovecount / gRepeats << " moves per game" << std::endl;

  //   std::cout << "Playouts: " << gPlayouts << "\tMCTS won " << wincount << "/" << gRepeats << " times with an average of " << (float) totalmovecount / gRepeats << " moves per game" << std::endl;
  // }

  // std::cout << "StudentHeatMap:" << std::endl;
  //
  // for ( int i = 0; i < BOARDWIDTH; ++i )
  // { std::cout << "|";
  //   for ( int j = 0; j < BOARDHEIGHT; ++j )
  //     std::cout << studentHeatMaps[N_PLAYERS][i][j] << "
  // |";
  //
  //   std::cout << std::endl;
  // }
  //
  // std::cout << std::endl << "MasterHeatMap:" << std::endl;
  //
  // for ( int i = 0; i < BOARDWIDTH; ++i )
  // { std::cout << "|";
  //   for ( int j = 0; j < BOARDHEIGHT; ++j )
  //     std::cout << masterHeatMaps[N_PLAYERS][i][j] << "|";
  //
  //   std::cout << std::endl;
  // }

  std::cout << "Cyclecount: " << gCycleCount << ", so an average of " << (float) gCycleCount / totalmovecount << " cycles per move" << std::endl;

  return 0;
}
