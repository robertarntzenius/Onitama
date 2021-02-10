#include "onitama.h"

#include <stdlib.h>

#include <iostream>
#include <fstream>


uint gPlayouts  = 1000;
uint gMaxTurns  = 100;


/* CLASS ONITAMA */

Onitama::Onitama ( const char * filename) : extra_(NULL),
                                            turn_(BLUE)
{
  // srand(123);
  srand(time(NULL));

  /* initialize cards from file with card data */
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

  if ( options[n].pawn_ != NULL )
    movePawn      ( options[n] );

  exchangeCards ( options[n].card_ );
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
    exchangeCards ( options[0].card_ );
    changeTurn    ( );
  }


  for ( i = 0; i < n; ++i )
  { wincount = 0;
    tiecount = 0;

    if ( options[i].pawn_ != NULL )
      movePawn      ( options[i] );

    exchangeCards ( options[i].card_ );
    changeTurn    ( );

    if ( i < gPlayouts % n )
      randomPlayouts ( onioption, wincount, tiecount, ( gPlayouts / n ) + 1 );
    else
      randomPlayouts ( onioption, wincount, tiecount, gPlayouts / n );

    if ( wincount + ( tievalue * tiecount ) >= maxwincount )
    { maxwincount = wincount + ( tievalue * tiecount );
      bestmove = i;
    }

    *this = onicopy;
  }

  if ( options[bestmove].pawn_ != NULL )
    movePawn      ( options[bestmove] );

  exchangeCards ( options[bestmove].card_ );
  changeTurn    ( );
}


void
Onitama::MCTSMove ( void )
{ MCTreeNode * root = new MCTreeNode ( *this );

  Option options [MAX_OPTIONS];
  uint n = 0;

  getOptions ( options, n );

  if ( n == 1 )
  { movePawn      ( options[0] );
    exchangeCards ( options[0].card_ );
    changeTurn    ( );
  }

  /* force first nodecycle (otherwise gPlayouts = 0 would cause segfault) */

  root->nodeCycle ( );
  for ( uint i = 1; i < gPlayouts; ++i )
  {
    root->nodeCycle ( );

    // std::cout << std::endl;
    //
    // root->printTree ( );
  }

  root = root->doBestMove ( );
  *this = root->getOnitama ( );

  delete root;
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
        { options[size].pawn_ = &pawns_[turn_][i];
          options[size].card_ = &hands_[turn_][j];
          options[size].move_ = &card->moves_[k];
          if ( turn_ == BLUE )
          { x = options[size].pawn_->x_ - options[size].move_->x_;
            y = options[size].pawn_->y_ + options[size].move_->y_;
          }
          else
          { x = options[size].pawn_->x_ + options[size].move_->x_;
            y = options[size].pawn_->y_ - options[size].move_->y_;
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
    { options[i].pawn_ = NULL;
      options[i].move_ = NULL;
      options[i].card_ = &hands_[turn_][i];
    }

    size = CARDS_PLAYER;
  }
}


void
Onitama::movePawn ( const Option & option )
{ uint i,j;

  if (turn_ == BLUE)
  { option.pawn_->x_ -= option.move_->x_;
    option.pawn_->y_ += option.move_->y_;
  }
  else
  { option.pawn_->x_ += option.move_->x_;
    option.pawn_->y_ -= option.move_->y_;
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
Onitama::exchangeCards ( Card ** card )
{ Card * helpcard;

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
    for ( uint j = 0; j < BOARDWIDTH; ++j )
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
                                               onitama_(onitama),
                                               nrplayed_(0),
                                               nrtied_(0),
                                               nrwon_(0),
                                               size_(0)
{ uint i;

  onitama_.getOptions ( options_, size_ );
  children_ = new MCTreeNode * [size_];

  for ( i = 0; i < size_; ++i )
    children_[i] = NULL;

  /* when created do a single random playout */
  randomPlayout ( );
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


MCTreeNode *
MCTreeNode::doBestMove ( void )
{ uint i;
  MCTreeNode * newroot;

  uint bestmove = 0,
       mostvisits = 0;

  for ( i = 0; i < size_; ++i )
    if ( children_[i] != NULL )
      if ( children_[i]->nrplayed_ >= mostvisits )
      { mostvisits = children_[i]->nrplayed_;
        bestmove = i;
      }

  // std::cout << bestmove << std::endl;

  newroot = children_[bestmove];

  children_[bestmove] = NULL;

  delete this;

  return newroot;
}


Onitama
MCTreeNode::getOnitama ( void )
{ return onitama_;
}


bool
MCTreeNode::nodeCycle ( void )
{ uint childnr = 0;

  Onitama onicopy = Onitama ( onitama_ );

  if ( onitama_.wayOfTheStone ( ) || onitama_.wayOfTheStream ( ) )
  { nrplayed_++;

    if (nrwon_ == 0)
      return false;

    nrwon_++;
    return true;
  }

  /* selection */
  selectChild ( childnr );

  if ( children_[childnr] == NULL )
  { if ( options_[childnr].pawn_ != NULL )
      onitama_.movePawn      ( options_[childnr] );

    onitama_.exchangeCards ( options_[childnr].card_ );
    onitama_.changeTurn    ( );

    /* Expansion (+ Simulation) */
    children_[childnr] = new MCTreeNode( onitama_ );

    /* Reset board positions */
    onitama_ = onicopy;

    nrplayed_++;

    if (children_[childnr]->nrwon_ == 0)
    { nrwon_++;
      return true;
    }
    return false;
  }
  else
  { nrplayed_++;

    if (children_[childnr]->nodeCycle())
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
                  + sqrt( (float) EXPLORATION_PARAMETER / (float) child->nrplayed_ );

    if ( currvalue >= highvalue )
    { highvalue = currvalue;
      childnr = i;
    }
  }
}


void
MCTreeNode::randomPlayout ( void )
{ uint turns = 0;

  Onitama onicopy = Onitama ( onitama_ );

  while( !onitama_.wayOfTheStone ( ) && !onitama_.wayOfTheStream ( ) && turns < gMaxTurns )
  { onitama_.randomMove ( );
    ++turns;
  }

  ++nrplayed_;

  if ( turns >= gMaxTurns )
    ++nrtied_;

  else if ( turns % N_PLAYERS == 0 )
  {
    // std::cout << onicopy.getTurn ( ) << std::endl;
    // onitama_.printBoard ( );
    // std::cin >> turns;
    ++nrwon_;
  }

  onitama_ = onicopy;
}
