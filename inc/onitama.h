#include <math.h>
#include <string>


/* Declarations */

/* Non-modular */
#define N_PLAYERS 2     /* Number of players */

#define CARDS_EXTRA 1  /* Number of neutral cards */

/* Modular */
#define BOARDWIDTH  5       /* Board width */
#define BOARDHEIGHT 5       /* Board height */

#define N_PAWNS BOARDWIDTH  /* Number of pawns per player (equal to board width) */
#define N_MASTERS 1         /* Number of masters per player */


#define CARDS_PLAYER  2  /* Number of cards per player */

/* Total number of cards per game */
#define N_CARDS N_PLAYERS * CARDS_PLAYER + CARDS_EXTRA

#define MAX_MOVES 4     /* Maximum number of moves on a single card */
#define MAX_RANGE 5     /* Maximum range of a single move */

#define MAX_OPTIONS 40  /* Maximum number of movement options per turn */

/* Makes code more readable */
#define BLUE 0
#define RED  1

// #define EXPLORATION_PARAMETER 1.41421356237

typedef unsigned int uint;

extern float gExploration;
extern uint gPlayouts;
extern uint gMaxTurns;

extern uint gCycleCount;

extern uint studentHeatMaps[N_PLAYERS + 1][BOARDHEIGHT][BOARDWIDTH];
extern uint masterHeatMaps[N_PLAYERS + 1][BOARDHEIGHT][BOARDWIDTH];

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
};


struct Card
{ Move moves_[MAX_MOVES];
  uint size_;
};


struct Option
{ uint pawnnr_;
  uint cardnr_;
  uint movenr_;
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
    void    MCTSMove   ( bool, uint );

    uint    getTurn    ( void ) const;

    bool    wayOfTheStone  ( void ) const;
    bool    wayOfTheStream ( void ) const;

    void    printBoard ( void ) const;

    void    updateStats   ( void );

  private:
    void    initCards ( const char * );
    void    initPawns ( void );

    void    randomPlayouts ( Onitama &, uint &, uint &, uint );

    void    getOptions  ( Option (&)[MAX_OPTIONS], uint & );

    void    movePawn      ( const Option & );
    void    exchangeCards ( uint );
    void    changeTurn    ( void );

    void    divideCards   ( void );
    void    refreshBoard  ( char [BOARDHEIGHT][BOARDWIDTH] ) const;

    void    printPlayerCards ( uint ) const;

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
    MCTreeNode ( const MCTreeNode & ) = delete;

    /* remove default constructor */
    MCTreeNode ( void ) = delete;
   ~MCTreeNode ( void );

    MCTreeNode & operator= ( const MCTreeNode & );

    Option getBestMove   ( void );
    uint   nodeCycle     ( Onitama & );
    bool   pruneNodes    ( uint );

    void   printTree     ( std::string pre = "" ) const;

  private:
    void   deleteChildren     ( void );
    void   selectChild        ( uint & );
    void   randomPlayout      ( Onitama & );

    MCTreeNode ** children_;

    Option options_[MAX_OPTIONS];

    uint nrplayed_,
         nrtied_,
         nrwon_;
    uint size_;
};
