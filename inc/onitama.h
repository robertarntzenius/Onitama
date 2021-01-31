#include <math.h>
#include <string>


/* Declarations */

/* Non-modular */
#define N_PLAYERS 2     /* Number of players */

#define CARDS_EXTRA 1  /* Number of neutral cards */

/* Modular */
#define BOARDSIZE 5       /* Board size */

#define N_PAWNS BOARDSIZE /* Number of pawns per player (equal to board size) */
#define N_MASTERS 1       /* Number of masters per player */


#define CARDS_PLAYER  2  /* Number of cards per player */

/* Total number of cards per game */
#define N_CARDS N_PLAYERS * CARDS_PLAYER + CARDS_EXTRA

#define MAX_MOVES 4     /* Maximum number of moves on a single card */
#define MAX_RANGE 5     /* Maximum range of a single move */

#define MAX_OPTIONS 40  /* Maximum number of movement options per turn */ /* TODO find the actual maximum */

/* Makes code more readable */
#define BLUE 0
#define RED  1

#define EXPLORATION_PARAMETER 2

typedef unsigned int uint;

/* Flag for printing boards */
extern bool gPrintFlag;

extern uint gPlayouts;
extern uint gMaxTurns;


enum ePawnType
{ Dead = 0,
  Student,
  Master
};


struct Move
{ int x_,
      y_;
};


struct Pawn
{ ePawnType type_;
  uint x_,
       y_;

  Pawn () : type_(ePawnType::Dead),
            x_(0),
            y_(0)
  {}
};


struct Card
{ Move moves_[MAX_MOVES];
  uint size_;

  Card () : size_(0)
  {}

  Card ( const bool cardMap[MAX_RANGE][MAX_RANGE], const bool stamp ) : size_(0)
  { uint i,j;

    for ( i = 0; i < MAX_RANGE; ++i )
      for ( j = 0; j < MAX_RANGE; ++j )
        if ( cardMap[i][j] == true )
        { this->moves_[size_].x_ = j - ( MAX_RANGE / 2 );
          this->moves_[size_].y_ = ( MAX_RANGE / 2 ) - i;
          ++size_;
        }
  }
};


struct Option
{ Pawn *  pawn_;
  Card ** card_;
  Move *  move_;
};

class MCTreeNode;

class Onitama
{ public:
    Onitama ( const char * );
    Onitama ( const Onitama & );

    /* remove default constructor */
    Onitama ( void ) = delete;
   ~Onitama ( void );

    /* operator copies card arrangement and pawn data */
    Onitama & operator= ( const Onitama & );

    void    randomMove ( void );
    void    MCMove     ( void );
    void    MCTSMove   ( void );

    uint    getTurn    ( void ) const;

    bool    wayOfTheStone  ( void ) const;
    bool    wayOfTheStream ( void ) const;

  private:
    void    initCards ( const char * );
    void    initPawns ( void );

    void    randomPlayouts ( Onitama &, uint &, uint & );

    void    getOptions  ( Option (&)[MAX_OPTIONS], uint & );

    void    movePawn      ( const Option & );
    void    exchangeCards ( Card ** );
    void    changeTurn    ( void );

    void    divideCards  ( void );
    void    refreshBoard ( void );

    void    printBoard ( void ); /* Not const, since it first updates the board */
    void    printPlayerCards ( uint ) const;

    char board_ [BOARDSIZE][BOARDSIZE];
    Pawn pawns_ [N_PLAYERS][N_PAWNS];

    Card cards_ [N_CARDS];

    Card * hands_ [N_PLAYERS][CARDS_PLAYER];
    Card * extra_;

    uint turn_;

    friend class MCTreeNode;
};


class MCTreeNode
{ public:
    MCTreeNode ( Onitama & );
    MCTreeNode ( const MCTreeNode & );

    /* remove default constructor */
    MCTreeNode ( void ) = delete;
   ~MCTreeNode ( void );

    MCTreeNode & operator= ( const MCTreeNode & );

    MCTreeNode * doBestMove    ( void );
    Onitama      getOnitama    ( void );

    bool         nodeCycle     ( void );

    void         printTree     ( std::string pre = "" ) const;

  private:
    void    deleteChildren     ( void );
    void    selectChild        ( uint & );
    void    randomPlayout      ( void );

    MCTreeNode ** children_;
    Onitama       onitama_;

    Option options_[MAX_OPTIONS];

    uint nrplayed_,
         nrtied_,
         nrwon_;
    uint size_;
};
