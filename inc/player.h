
#include "onitama.h"

typedef Onitama Game;

enum ePlayerType
{
  RandomPlayer,
  MonteCarloPlayer
};

class Player
{

  public:
    Player ( const Game *game, const bool ID ) : game ( game ),
                                                 playerID (ID),
                                                 type ( ePlayerType::RandomPlayer )
    {

    };

   ~Player ()
   {

   };

   void setPlayerType (const int type);

   void printData () const;

  private:
    const Game *game;


    const bool playerID;
    ePlayerType type;
};
