#include "onitama.h"

#include <stdlib.h>

#include <iostream>
#include <fstream>

float gExploration = 1.41421356237f;
uint gPlayouts  = 1000;
uint gMaxTurns  = 100;

// uint gMaxOptions = 0;
// uint gTotOptions = 0;
// uint gNrTurns    = 0;

/* CLASS ONITAMA */

Onitama::Onitama ( const char * filename ) : extra_(NULL),
                                             turn_(BLUE)
{ /* initialize cards from file with card data */
  initCards ( filename );

  /* initalize pawns from const values in header */
  initPawns ( );

  divideCards ( );

  turn_ = rand() % N_PLAYERS;
}


Onitama::Onitama (const Onitama & onitama ) : extra_(NULL),
                                              turn_(onitama.turn_)
{ uint i,j;

  for ( i = 0; i < N_PLAYERS; ++i )
    for ( j = 0; j < N_PAWNS; ++j )
      pawns_[i][j] = onitama.pawns_[i][j];

  for ( i = 0; i < N_CARDS; ++i )
    cards_[i] = onitama.cards_[i];

  for ( i = 0; i < N_PLAYERS; ++i )
    for ( j = 0; j < CARDS_PLAYER; ++j )
      hands_[i][j] = &cards_[onitama.hands_[i][j] - onitama.cards_];

  extra_ = &cards_[onitama.extra_ - onitama.cards_];
}


Onitama::~Onitama ( void )
{
}


Onitama &
Onitama::operator= (const Onitama & onitama )
{ uint i,j;

  for ( i = 0; i < N_PLAYERS; ++i )
    for ( j = 0; j < N_PAWNS; ++j )
      pawns_[i][j] = onitama.pawns_[i][j];

  for ( i = 0; i < N_PLAYERS; ++i )
    for ( j = 0; j < CARDS_PLAYER; ++j )
      hands_[i][j] = &cards_[onitama.hands_[i][j] - onitama.cards_];

  extra_ = &cards_[onitama.extra_ - onitama.cards_];

  turn_ = onitama.turn_;

  return *this;
}


void
Onitama::randomMove ( void )
{ uint n = 0;

  Option options [MAX_OPTIONS];

  getOptions ( options, n );

  n = rand() % n;

  if ( options[n].pawnnr_ != N_PAWNS )
    movePawn      ( options[n] );

  exchangeCards ( options[n].cardnr_ );
  changeTurn    ( );
}


void
Onitama::MCMove ( void )
{ uint i,
       n = 0;

  uint bestmove = 0,
       wincount = 0,
       tiecount = 0,
       maxwincount = 0;

  const double tievalue = -0.05;

  Onitama onioption = Onitama ( *this ),
          onicopy   = Onitama ( *this );

  Option options [MAX_OPTIONS];

  getOptions ( options, n );

  if ( n == 1 )
  { movePawn      ( options[0] );
    exchangeCards ( options[0].cardnr_ );
    changeTurn    ( );
  }


  for ( i = 0; i < n; ++i )
  { wincount = 0;
    tiecount = 0;

    if ( options[i].pawnnr_ != N_PAWNS )
      movePawn      ( options[i] );

    exchangeCards ( options[i].cardnr_ );
    changeTurn    ( );

    if ( i <= gPlayouts % n )
      randomPlayouts ( onioption, wincount, tiecount, ( gPlayouts / n ) + 1 );
    else
      randomPlayouts ( onioption, wincount, tiecount, gPlayouts / n );

    if ( wincount + ( tievalue * tiecount ) >= maxwincount )
    { maxwincount = wincount + ( tievalue * tiecount );
      bestmove = i;
    }

    *this = onicopy;
  }

  if ( options[bestmove].pawnnr_ != N_PAWNS )
    movePawn      ( options[bestmove] );

  exchangeCards ( options[bestmove].cardnr_ );
  changeTurn    ( );
}


void
Onitama::MCTSMove ( void )
{ MCTreeNode * root = new MCTreeNode ( *this );

  Onitama onicopy = *this;

  Option options [MAX_OPTIONS];
  uint   bestmove,
         n = 0;

  getOptions ( options, n );

  if ( n == 1 )
  { movePawn      ( options[0] );
    exchangeCards ( options[0].cardnr_ );
    changeTurn    ( );
  }

  /* force first nodecycle (otherwise gPlayouts = 0 would cause segfault) */

  root->nodeCycle ( *this );
  for ( uint i = 1; i < gPlayouts; ++i )
  { *this = onicopy;

    root->nodeCycle ( *this );

    // std::cout << std::endl;
    // root->printTree ( );
  }

  *this = onicopy;

  bestmove = root->getBestMove ( );

  if ( options[bestmove].pawnnr_ != N_PAWNS )
    movePawn      ( options[bestmove] );

  exchangeCards ( options[bestmove].cardnr_ );
  changeTurn    ( );

  delete root;

  // std::cout << "average n.o. options:\t" << gTotOptions / gNrTurns << std::endl;
  // std::cout << "maximum n.o. options:\t" << gMaxOptions << std::endl;

}

uint
Onitama::getTurn ( void ) const
{ return turn_;
}


bool
Onitama::wayOfTheStone ( void ) const
{ uint i;

  for ( i = 0; i < N_MASTERS; ++i )
    if ( pawns_[turn_][i].type_ != ePawnType::Dead )
      return false;
  return true;
}


bool
Onitama::wayOfTheStream ( void ) const
{ if ( turn_ == BLUE )
  { for ( uint i = 0; i < N_MASTERS; ++i )
    { if ( pawns_[RED][i].y_ == BOARDHEIGHT - 1 )
      { for ( uint j = (BOARDWIDTH - N_MASTERS) / 2; j < N_MASTERS; ++j )
        { if ( pawns_[RED][i].x_ == j )
            return true;
        }
      }
    }
  }

  else
  { for ( uint i = 0; i < N_MASTERS; ++i )
    { if ( pawns_[BLUE][i].y_ == 0 )
      { for ( uint j = (BOARDWIDTH - N_MASTERS) / 2; j < N_MASTERS; ++j )
        { if ( pawns_[BLUE][i].x_ == j )
            return true;
        }
      }
    }
  }

  return false;
}


void
Onitama::initCards ( const char * filename )
{ uint n = 0,
       m = 0,
       r;

  std::ifstream ifs ( filename, std::ifstream::in );
  std::string line;

  std::size_t numberOfCards;

  if ( !ifs.good ( ) )
    return;

  ifs >> numberOfCards;

  bool selection[numberOfCards] = {false};

  while ( n < N_CARDS )
  { r = rand() % numberOfCards;
    if ( selection[r] == false )
    { selection[r] = true;
      ++n;
    }
  }

  ifs.ignore ( );

  for ( n = 0; n < numberOfCards; ++n )
  { ifs.ignore();

    if ( selection[n] == true )
    { cards_[m].size_ = 0;
      for ( uint y = 0; y < MAX_RANGE; ++y )
      { getline ( ifs, line );
        for ( uint x = 0; x < MAX_RANGE; ++x )
          if ( line[x] == '1' )
          { cards_[m].moves_[cards_[m].size_].x_ = x - ( MAX_RANGE / 2 );
            cards_[m].moves_[cards_[m].size_].y_ = ( MAX_RANGE / 2 ) - y;
            cards_[m].size_++;
          }
      }
      m++;
    }
    else
    { for ( uint y = 0; y < MAX_RANGE; ++y )
        getline ( ifs, line );
    }
  }
  ifs.close ( );
}


void
Onitama::initPawns ( void )
{ uint i,
       j = (BOARDWIDTH - N_MASTERS) / 2;

  for ( i = 0; i < N_MASTERS; ++i )
  { pawns_[BLUE][i].type_ = ePawnType::Master;
    pawns_[RED ][i].type_ = ePawnType::Master;
    pawns_[BLUE][i].x_ = j + i;
    pawns_[RED ][i].x_ = j + i;
    pawns_[BLUE][i].y_ = 0;
    pawns_[RED ][i].y_ = BOARDHEIGHT - 1;
  }

  for ( i = i; i < BOARDWIDTH; ++i )
  { pawns_[BLUE][i].type_ = ePawnType::Student;
    pawns_[RED ][i].type_ = ePawnType::Student;
    pawns_[BLUE][i].x_ = (j + i) % BOARDWIDTH;
    pawns_[RED ][i].x_ = (j + i) % BOARDWIDTH;
    pawns_[BLUE][i].y_ = 0;
    pawns_[RED ][i].y_ = BOARDHEIGHT - 1;
  }
}


void
Onitama::randomPlayouts (Onitama & onitama, uint & wincount, uint & tiecount, uint playouts )
{ uint i, turns;
  onitama = *this;

  wincount = 0;
  tiecount = 0;

  for ( i = 0; i < playouts; ++i )
  { turns = 0;

    while( !wayOfTheStone ( ) && !wayOfTheStream ( ) && turns < gMaxTurns )
    { randomMove ( );
      ++turns;
    }

    if ( turns >= gMaxTurns )
      ++tiecount;
    else if ( turns % N_PLAYERS == 0 )
      ++wincount;

    *this = onitama;
  }
}


void
Onitama::getOptions ( Option ( & options ) [MAX_OPTIONS], uint & size )
{ uint i,j,k,l,
       x,y;

  Card * card;

  for ( i = 0; i < N_PAWNS; ++i )
    if ( pawns_[turn_][i].type_ != ePawnType::Dead )
      for ( j = 0; j < CARDS_PLAYER; ++j )
      { card = hands_[turn_][j];
        for ( k = 0; k < card->size_; ++k )
        { options[size].pawnnr_ = i;
          options[size].cardnr_ = j;
          options[size].movenr_ = k;
          if ( turn_ == BLUE )
          { x = pawns_[turn_][i].x_ - card->moves_[k].x_;
            y = pawns_[turn_][i].y_ + card->moves_[k].y_;
          }
          else
          { x = pawns_[turn_][i].x_ + card->moves_[k].x_;
            y = pawns_[turn_][i].y_ - card->moves_[k].y_;
          }

          if ( x >= BOARDWIDTH || y >= BOARDHEIGHT )
            break;

          ++size;
          for ( l = 0; l < N_PAWNS; ++l )
            if (  ( turn_ == BLUE && x == pawns_[BLUE][l].x_ && y == pawns_[BLUE][l].y_ )
               || ( turn_ == RED  && x == pawns_[RED ][l].x_ && y == pawns_[RED ][l].y_ ) )
            { --size;
              break;
            }

          if ( size == MAX_OPTIONS )
            return;
        }
      }

  if ( size == 0 )
  { for ( i = 0; i < CARDS_PLAYER; ++i )
    { options[i].pawnnr_ = N_PAWNS;
      options[i].movenr_ = 0;
      options[i].cardnr_ = 0;
    }

    size = CARDS_PLAYER;
  }

  // gTotOptions += size;
  // gNrTurns++;
}


void
Onitama::movePawn ( const Option & option )
{ uint i,j;

  Pawn * pawn = &pawns_[turn_][option.pawnnr_];
  Card * card = hands_[turn_][option.cardnr_];
  Move * move = &card->moves_[option.movenr_];

  if (turn_ == BLUE)
  { pawn->x_ -= move->x_;
    pawn->y_ += move->y_;
  }
  else
  { pawn->x_ += move->x_;
    pawn->y_ -= move->y_;
  }

  for ( i = 0; i < N_PAWNS; ++i )
  { if ( pawns_[BLUE][i].type_ != ePawnType::Dead )
    { for ( j = 0; j < N_PAWNS; ++j )
      { if ( pawns_[RED][j].type_ != ePawnType::Dead )
        { if ( pawns_[BLUE][i].x_ == pawns_[RED][j].x_ && pawns_[BLUE][i].y_ == pawns_[RED][j].y_)
          { if (turn_ == BLUE)
              pawns_[RED ][j].type_ = ePawnType::Dead;
            else
              pawns_[BLUE][i].type_ = ePawnType::Dead;
          }
        }
      }
    }
  }
}


void
Onitama::exchangeCards ( uint cardnr )
{ Card ** card = &hands_[turn_][cardnr],
       *  helpcard;

  helpcard = *card;
  *card = extra_;
  extra_ = helpcard;
}


void
Onitama::changeTurn ( void )
{ turn_ = (turn_ == BLUE)? RED : BLUE;
}


void
Onitama::divideCards ( void )
{ uint i,
       n = 0;

  std::size_t divided[N_PLAYERS+1];
  int  r;

  bool done = true;

  for ( i = 0 ; i < N_PLAYERS + 1; ++i )
    divided[i] = 0;

  while (n < N_CARDS)
  { r = rand() % (N_CARDS - n);

    for ( i = 0; i < N_PLAYERS; ++i )
    { if ( r + divided[i] < CARDS_PLAYER )
      { hands_[i][divided[i]] = &cards_[n];
        ++divided[i];
        ++n;
        done = true;
        break;
      }
      r -= CARDS_PLAYER;
    }
    if (!done)
    { extra_ = &cards_[n];
      ++divided[N_PLAYERS];
      ++n;
      done = true;
    }
  }
}


void
Onitama::refreshBoard ( char board [BOARDHEIGHT][BOARDWIDTH] ) const
{ uint half,
       x,y;

  ePawnType type;

  for ( uint i = 0; i < BOARDHEIGHT; ++i )
    for ( uint j = 0; j < BOARDWIDTH; ++j )
      board[i][j] = '.';

  half = (BOARDWIDTH - N_MASTERS) / 2;

  for ( uint i = 0; i < N_MASTERS; ++i )
  { board[0][i + half]               = '_';
    board[BOARDHEIGHT - 1][i + half] = '_';
  }

  for ( uint i = 0; i < N_PLAYERS; ++i )
    for ( uint j = 0; j < N_PAWNS; ++j )
    { x = pawns_[i][j].x_;
      y = pawns_[i][j].y_;
      type = pawns_[i][j].type_;

      if ( type == ePawnType::Student )
        board[y][x] = ( i == BLUE )? 's' : 'S';
      else if ( type == ePawnType::Master )
        board[y][x] = ( i == BLUE )? 'm' : 'M';
    }
}


void
Onitama::printBoard ( void ) const
{ char board [BOARDHEIGHT][BOARDWIDTH];

  refreshBoard ( board );

  for ( uint x = 0; x < (MAX_RANGE + 5) * (CARDS_PLAYER + CARDS_EXTRA) - 1; ++x )
    std::cout << "=";

  printPlayerCards ( BLUE );

  std::cout << "BLUE" << std::endl;

  for ( uint y = 0; y < BOARDHEIGHT; ++y )
  { for ( uint x = 0; x < BOARDWIDTH; ++x )
      std::cout << board[y][x];

    std::cout << std::endl;
  }
  std::cout << "RED";

  printPlayerCards ( RED );
}


void
Onitama::printPlayerCards ( uint playernr ) const
{ uint i,
       n[CARDS_PLAYER + CARDS_EXTRA] = {0};
  int  x,y;
  Card * card;
  Move move;

  std::cout << std::endl << std::endl;

  /* Print labels */
  for ( i = 0; i < CARDS_PLAYER + CARDS_EXTRA; ++i )
  { if ( i < CARDS_PLAYER )
      std::cout << "| CARD" << i << " | ";
    else if ( playernr == turn_ )
      std::cout << "| EXTRA |";
    else
      break;
  }

  std::cout << std::endl;

  for ( y = 0; y < MAX_RANGE; ++y )
  { for ( i = 0; i < CARDS_PLAYER + CARDS_EXTRA; ++i )
    { if ( i < CARDS_PLAYER )
        card = hands_[playernr][i];
      else if ( playernr == turn_ )
        card = extra_;
      else
        break;

      std::cout << "| ";
      for ( x = 0; x < MAX_RANGE; ++x )
      { if ( playernr == BLUE )
          move = card->moves_[card->size_ - n[i] - 1];
        else
          move = card->moves_[n[i]];

        if ( ( playernr == BLUE && move.x_ == ( MAX_RANGE / 2 ) - x && move.y_ == y - ( MAX_RANGE / 2 ) )
          || ( playernr == RED  && move.x_ == x - ( MAX_RANGE / 2 ) && move.y_ == ( MAX_RANGE / 2 ) - y ) )
        { std::cout << "1";
          if ( n[i] < card->size_ - 1 )
            ++n[i];
        }

        else
          std::cout << "0";
      }
      std::cout << " | ";
    }
    std::cout << std::endl;
  }

  std::cout << std::endl;
}


/* CLASS MCTREENODE */

MCTreeNode::MCTreeNode ( Onitama & onitama ) : children_(NULL),
                                               nrplayed_(0),
                                               nrtied_(0),
                                               nrwon_(0),
                                               size_(0)
{ uint i;

  onitama.getOptions ( options_, size_ );
  children_ = new MCTreeNode * [size_];

  for ( i = 0; i < size_; ++i )
    children_[i] = NULL;

  /* when created do a single random playout */
  randomPlayout ( onitama );
}


// MCTreeNode::MCTreeNode ( const MCTreeNode & node )
// {
//   /* TODO */
// }


MCTreeNode::~MCTreeNode ( void )
{ uint i;

  for ( i = 0; i < size_; ++i )
    if ( children_[i] != NULL )
      delete children_[i];

  delete[] children_;
}


// MCTreeNode::MCTreeNode & operator= ( const MCTreeNode & )
// {
//   /* TODO */
// }


uint
MCTreeNode::getBestMove ( void )
{ uint i;

  uint bestmove = 0,
       mostvisits = 0;

  for ( i = 0; i < size_; ++i )
    if ( children_[i] != NULL )
      if ( children_[i]->nrplayed_ >= mostvisits )
      { mostvisits = children_[i]->nrplayed_;
        bestmove = i;
      }

  return bestmove;
}


bool
MCTreeNode::nodeCycle ( Onitama & onitama )
{ uint childnr = 0;

  if ( onitama.wayOfTheStone ( ) || onitama.wayOfTheStream ( ) )
  { nrplayed_++;

    if (nrwon_ == 0)
      return false;

    nrwon_++;
    return true;
  }

  /* selection */
  selectChild ( childnr );

  if ( options_[childnr].pawnnr_ != N_PAWNS )
    onitama.movePawn      ( options_[childnr] );

  onitama.exchangeCards ( options_[childnr].cardnr_ );
  onitama.changeTurn    ( );

  if ( children_[childnr] == NULL )
  { /* Expansion (+ Simulation) */
    children_[childnr] = new MCTreeNode( onitama );

    nrplayed_++;

    if (children_[childnr]->nrwon_ == 0)
    { nrwon_++;
      return true;
    }
    return false;
  }
  else
  { nrplayed_++;

    if (children_[childnr]->nodeCycle( onitama ))
      return false;

    nrwon_++;
    return true;
  }
}


void
MCTreeNode::printTree ( std::string pre /* = "" */) const
{ uint i;

  std::cout << pre << " (" << nrwon_ << "/" << nrplayed_ << ") " << std::endl;

  for ( i = 0; i < size_; ++i )
  { if ( children_[i] != NULL )
    {
      children_[i]->printTree ( "\t" + pre );
    }
  }
}


void
MCTreeNode::selectChild ( uint & childnr )
{ uint  i;
  float currvalue = 0.f,
        highvalue = 0.f;

  MCTreeNode * child;

  for ( i = 0; i < size_; ++i )
  { child = children_[i];
    if ( child == NULL )
    { childnr = i;
      return;
    }

    else
      currvalue = ( (float) child->nrwon_ / (float) child->nrplayed_ )
                  + gExploration * sqrt( (float) log( (float) nrplayed_ )  / (float) child->nrplayed_ );

    if ( currvalue >= highvalue )
    { highvalue = currvalue;
      childnr = i;
    }
  }
}


void
MCTreeNode::randomPlayout ( Onitama & onitama )
{ uint turns = 0;

  Onitama onicopy = Onitama ( onitama );

  while( !onitama.wayOfTheStone ( ) && !onitama.wayOfTheStream ( ) && turns < (gMaxTurns / 10) )
  { onitama.randomMove ( );
    ++turns;
  }

  ++nrplayed_;

  if ( turns >= gMaxTurns )
    ++nrtied_;

  else if ( turns % N_PLAYERS == 0 )
  {
    // std::cout << onicopy.getTurn ( ) << std::endl;
    // onitama.printBoard ( );
    // std::cin >> turns;
    ++nrwon_;
  }

  onitama = onicopy;
}
