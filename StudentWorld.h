#ifndef STUDENTWORLD_H_
#define STUDENTWORLD_H_

#include "GameWorld.h"
#include "Level.h"
#include <string>
#include <vector>

class Actor;
class Player;
// Students:  Add code to this file, StudentWorld.cpp, Actor.h, and Actor.cpp

class StudentWorld : public GameWorld
{
public:
  StudentWorld(std::string assetPath);
  ~StudentWorld();
  virtual int init();
  virtual int move();
  virtual void cleanUp();
  int loadLevel();
  Player* getPlayer() const;
  int getBonus() const;
  int getPlayerHealth() const;
  int getPlayerAmmo() const;
  bool isPlayerAlive() const;
  bool canMarbleMoveTo(int x, int y) const;
  bool isActorAt(int x, int y) const;
  bool isObstacleAt(int x, int y) const; // for peas
  bool isThiefBotAt(int x, int y) const;
  Actor* getActorAt(int x, int y) const; // agent only
  Actor* getMarbleAt(int x, int y) const;
  Actor* getGoodieAt(int x, int y) const; // for thiefbots
  Actor* getDestroyableActorAt(int x, int y) const;
  int countThiefBotsSurroundingFactory(int x, int y) const;

  void setDisplayText();
  void removeDeadGameObjects();
  void reduceLevelBonusByOne();
  void reduceCrystalsByOne();
  bool collectedCrystals();
  void createNewPea(int x, int y, int direction);
  void createNewThiefBot(int x, int y, int type);

private:
	Player* m_player; // tracks player
	std::vector<Actor*> actors; // array of Actor pointers
	int m_bonus; // tracks bonus points
	int m_crystals; // tracks # of crystals left
};

#endif // STUDENTWORLD_H_
