#ifndef ACTOR_H_
#define ACTOR_H_

#include "GraphObject.h"

class StudentWorld;
class Agent;

class Actor : public GraphObject
{
	public:
		Actor(StudentWorld* world, int imageID, int startX, int startY,  int startDirection);
		virtual void doSomething() = 0;
		// Can an agent occupy the same square as this actor?
		virtual bool allowsAgentColocation() const { return false; }
		virtual bool isDestroyable() const { return false; }
		virtual bool isSwallowable() const { return false; }
		virtual bool isInvisibleAtFirst() const { return false; }
		virtual bool bePushedBy(Agent* a, int x, int y) { return false; }
		virtual bool allowsMarble() const { return false; } // PIT SHOULD BE TRUE (when implemented)
		virtual bool countsInFactoryCensus() const { return false; }
		virtual bool isStealable() const { return false; }
		virtual void damage(int damageAmt) {}
		virtual void setStolen(bool status) {}
		bool isAlive() const;
		bool isWithinBounds(int x, int y) const;
		StudentWorld* getWorld() const;
		void kill();
	private:
		StudentWorld* m_world;
		bool m_alive;
};

class Agent : public Actor
{
public:
	Agent(StudentWorld* world, int imageID, int startX, int startY, int hitPoints, int startDirection);
	
	int getHitPoints() const;

	void decreaseHitPoints(int damageAmt);

	void restorePlayerHitPoints(); // should only be used by Player
	
	// Move to the adjacent square in the direction the agent is facing
	// if it is not blocked, and return true.  Return false if the agent
	// can't move.
	bool moveIfPossible();
	// Return true if this agent can push marbles (which means it's the
	// player).
	virtual bool canPushMarbles() const { return false; }
	
	// Return true if this agent doesn't shoot unless there's an unobstructed
	// path to the player.
	//virtual bool needsClearShot() const;

	virtual bool isDestroyable() const { return true; }

	bool firePea();

	bool shotIsClear();

	virtual bool needsClearShot() const = 0;

	// Return the sound effect ID for a shot from this agent.
	virtual int shootingSound() const = 0;

private:
	int m_hitpoints;
};

class Player : public Agent
{
	public:
		Player(StudentWorld* world, int startX, int startY);
		virtual bool canPushMarbles() const { return true; }
		int getHealthPct() const;
		int getAmmo() const;
		void decreaseAmmo(int amount);
		void increaseAmmo(int amount);
		virtual void damage(int damageAmt);
		virtual void doSomething();
		virtual bool needsClearShot() const { return false; }
		virtual int shootingSound() const { return SOUND_PLAYER_FIRE; }

	private:
		int m_ammo;
};

class Robot : public Agent
{
public:
	Robot(StudentWorld* world, int imageID, int startX, int startY, int hitPoints, int score, int startDir);
	virtual void doSomething();
	virtual void performAction() = 0;
	virtual bool needsClearShot() const { return true; }
	virtual int shootingSound() const { return SOUND_ENEMY_FIRE; }
	virtual bool isShootingRobot() const { return true; }
	int getScoreValue() const;
private:
	int m_scoreValue;
	int m_ticks;
};

class RageBot : public Robot
{
public:
	RageBot(StudentWorld* world, int startX, int startY, int startDir);
	virtual void performAction();
	virtual void damage(int damageAmt);
};	

class ThiefBot : public Robot
{
public:
	ThiefBot(StudentWorld* world, int imageID, int startX, int startY, int hitPoints, int score);
	virtual bool countsInFactoryCensus() const { return true; }
	virtual void performAction();
	virtual void damage(int damageAmt);
	bool hasPickedUpGoodie() const;
	bool hasNotMovedDistanceBeforeTurning() const;
	void pickUpGoodie();
private:
	int m_distanceBeforeTurning;
	bool m_carryingGoodie;
	Actor* m_stolenGoodie;
};

class RegularThiefBot : public ThiefBot
{
public:
	RegularThiefBot(StudentWorld* world, int startX, int startY);
	virtual bool isShootingRobot() const { return false; }
};

class MeanThiefBot : public ThiefBot
{
public:
	MeanThiefBot(StudentWorld* world, int startX, int startY);
};

class ThiefBotFactory : public Actor
{
public:
	enum ProductType { REGULAR, MEAN };

	ThiefBotFactory(StudentWorld* world, int startX, int startY, ProductType type);
	virtual void doSomething();
private:
	ProductType m_type;
};

class Pea : public Actor
{
	public:
		Pea(StudentWorld* world, int startX, int startY, int startDirection);
		virtual bool allowsAgentColocation() const { return true; }
		virtual void doSomething();
		bool checkForActors();
	private:
		bool m_newPea;
};

class Exit : public Actor
{
	public:
		Exit(StudentWorld* world, int startX, int startY);
		virtual bool allowsAgentColocation() const { return true; }
		virtual bool isInvisibleAtFirst() const { return true; }
		virtual void doSomething();
	private:
};

class Wall : public Actor 
{
	public:
		Wall(StudentWorld* world, int startX, int startY);
		virtual void doSomething() {}
	private:
};

class Marble : public Actor {
public:
	Marble(StudentWorld* world, int startX, int startY);
	virtual void doSomething() {}
	virtual bool isDestroyable() const { return true; }
	virtual bool isSwallowable() const { return true; }
	virtual void damage(int damageAmt);
	virtual bool bePushedBy(Agent* a, int x, int y);
private:
	int m_hitpoints;
};

class Pit : public Actor
{
public:
	Pit(StudentWorld* world, int startX, int startY);
	virtual bool allowsMarble() const { return true; }
	virtual void doSomething();
};

class Item : public Actor {
public:
	Item(StudentWorld* world, int startX, int startY, int imageID, int score);
	virtual bool allowsAgentColocation() const { return true; }
	int getScoreValue() const;
private:
	int m_scoreValue;
};

class Crystal : public Item
{
public:
	Crystal(StudentWorld* world, int startX, int startY);
	virtual void doSomething();
private:
};

class Goodie : public Item
{
public:
	Goodie(StudentWorld* world, int startX, int startY, int imageID, int score);
	virtual void doSomething();
	virtual bool isStealable() const { return true; }
	virtual void performAction() = 0;

	// Set whether this goodie is currently stolen.
	virtual void setStolen(bool status);
private:
	bool m_stolen;
};

class ExtraLifeGoodie : public Goodie
{
public:
	ExtraLifeGoodie(StudentWorld * world, int startX, int startY);
	virtual void performAction();
};

class RestoreHealthGoodie : public Goodie
{
public:
	RestoreHealthGoodie(StudentWorld* world, int startX, int startY);
	virtual void performAction();
};

class AmmoGoodie : public Goodie
{
public:
	AmmoGoodie(StudentWorld* world, int startX, int startY);
	virtual void performAction();
};

#endif // ACTOR_H_
