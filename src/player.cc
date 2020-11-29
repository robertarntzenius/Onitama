#include <iostream>

#include "player.h"


void
Player::setPlayerType ( const int type )
{
  switch (type) {
    default:
    case ePlayerType::RandomPlayer:
      this->type = ePlayerType::RandomPlayer;
      break;
    case ePlayerType::MonteCarloPlayer:
      this->type = ePlayerType::MonteCarloPlayer;
      break;
  }
}


void
Player::printData ( ) const
{
  std::cout << "Player type: " << type << std::endl;
}
