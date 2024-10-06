#include "Actor.h"
#include "StudentWorld.h"

Actor::Actor(StudentWorld* world, int imageID, int startX, int startY, int startDirection = none)
	: GraphObject(imageID, startX, startY, startDirection), m_world(world), m_alive(true)
{}

bool Actor::isAlive() const{
	if (!m_alive) {
		return false;
	}
	return true;
}

bool Actor::isWithinBounds(int x, int y) const {
    if (x < 0 || x >= VIEW_WIDTH || y < 0 || y >= VIEW_HEIGHT) {
        return false;
    }
    return true;
}

StudentWorld* Actor::getWorld() const {
	return m_world;
}

void Actor::kill() {
    m_alive = false;
    setVisible(false);
}

// AGENT IMPLEMENTATIONS

Agent::Agent(StudentWorld* world, int imageID, int startX, int startY, int hitPoints, int startDirection)
    : Actor(world, imageID, startX, startY, startDirection), m_hitpoints(hitPoints) {
    setVisible(true);
}

int Agent::getHitPoints() const {
    return m_hitpoints;
}

void Agent::decreaseHitPoints(int damageAmt) {
    m_hitpoints -= damageAmt;
}

void Agent::restorePlayerHitPoints() {
    m_hitpoints = 20;
}

bool Agent::firePea() {
    int x = getX();
    int y = getY();
    switch (getDirection()) {
        case up:
            y++;
            break;
        case down:
            y--;
            break;
        case left:
            x--;
            break;
        case right:
            x++;
            break;
    }

    if (needsClearShot()) { // robot
        if (shotIsClear()) {
            getWorld()->createNewPea(x, y, getDirection());
            getWorld()->playSound(shootingSound());
            return true;
        }
    }
    else { // player
        getWorld()->createNewPea(x, y, getDirection());
        getWorld()->getPlayer()->decreaseAmmo(1);
        getWorld()->playSound(shootingSound());
        return true;
    }
    return false;
}

bool Agent::shotIsClear() {
    int playerX = getWorld()->getPlayer()->getX();
    int playerY = getWorld()->getPlayer()->getY();
    int x = getX();
    int y = getY();

    switch (getDirection()) {
    case up:
        y++;
        while (getWorld()->isObstacleAt(x, y) == false && isWithinBounds(x, y)) {
            y++;
        }
        break;
    case down:
        y--;
        while (getWorld()->isObstacleAt(x, y) == false && isWithinBounds(x, y)) {
            y--;
        }
        break;
    case left:
        x--;
        while (getWorld()->isObstacleAt(x, y) == false && isWithinBounds(x, y)) {
            x--;
        }
        break;
    case right:
        x++;
        while (getWorld()->isObstacleAt(x, y) == false && isWithinBounds(x, y)) {
            x++;
        }
        break;
    }

    if (x == playerX && y == playerY) {
        return true;
    }
    return false;

}

bool Agent::moveIfPossible() {
    int x = getX();
    int y = getY();
    switch (getDirection()) {
    case up:
        y++;
        break;
    case down:
        y--;
        break;
    case left:
        x--;
        break;
    case right:
        x++;
        break;
    }

    if (!isWithinBounds(x, y)) {
        return false;
    }

    bool result = getWorld()->isActorAt(x, y);

    if (!result) {
        moveTo(x, y);
        return true;
    }

    if (canPushMarbles()) {
        Actor* marble = getWorld()->getActorAt(x, y);
        if (marble->bePushedBy(this, marble->getX(), marble->getY())) {
            switch (getDirection()) {
            case up:
                marble->moveTo(x, y + 1);
                break;
            case down:
                marble->moveTo(x, y - 1);
                break;
            case left:
                marble->moveTo(x - 1, y);
                break;
            case right:
                marble->moveTo(x + 1, y);
                break;
            }
            moveTo(x, y);
        }

    }

    return false;
}

// PLAYER IMPLEMENTATIONS

Player::Player(StudentWorld* world, int startX, int startY)
	: Agent(world, IID_PLAYER, startX, startY, 20, right), m_ammo(20) {
}

int Player::getHealthPct() const {
	return getHitPoints() * 100 / 20;
}

int Player::getAmmo() const {
	return m_ammo;
}

void Player::decreaseAmmo(int amount) {
    m_ammo -= amount;
}

void Player::increaseAmmo(int amount) {
    m_ammo += amount;
}

void Player::damage(int damageAmt) {
    decreaseHitPoints(damageAmt);
    getWorld()->playSound(SOUND_PLAYER_IMPACT);
    if (getHitPoints() <= 0) {
        kill();
    }
}

void Player::doSomething() {
    if (!isAlive()) {
        return;
    }

    int ch;
    if (getWorld()->getKey(ch)) {
        // User pressed a key
        switch (ch) {
            case KEY_PRESS_ESCAPE:
                kill();
                break;
            case KEY_PRESS_SPACE:
                if (m_ammo > 0) {
                    firePea();
                }
                break;
            case KEY_PRESS_UP:
                setDirection(up);
                moveIfPossible();
                break;
            case KEY_PRESS_DOWN:
                setDirection(down);
                moveIfPossible();
                break;
            case KEY_PRESS_LEFT:
                setDirection(left);
                moveIfPossible();
                break;
            case KEY_PRESS_RIGHT:
                setDirection(right);
                moveIfPossible();
                break;
        }
    }
 }

// ROBOT IMPLEMENTATIONS

Robot::Robot(StudentWorld* world, int imageID, int startX, int startY, int hitPoints, int score, int startDir) 
    : Agent(world, imageID, startX, startY, hitPoints, startDir), m_scoreValue(score), m_ticks(0) {
    setVisible(true);
}

void Robot::doSomething() {
    if (!isAlive()) {
        return;
    }

    m_ticks++;

    int ticks = (28 - getWorld()->getLevel()) / 4;

    if (ticks < 3)
        ticks = 3;

    if (m_ticks / ticks == 1) {
        m_ticks = 0;

        if (isShootingRobot() && needsClearShot()) {
            if (firePea()) { // returns true if pea fired
                return;
            }
        }

        performAction();
    }
}

int Robot::getScoreValue() const {
    return m_scoreValue;
}

// RAGEBOT IMPLEMENTATIONS

RageBot::RageBot(StudentWorld* world, int startX, int startY, int startDir) 
    : Robot(world, IID_RAGEBOT, startX, startY, 10, 100, startDir) {
}

void RageBot::performAction() {
    if (!moveIfPossible()) {
        switch (getDirection()) {
        case up:
            setDirection(down);
            break;
        case down:
            setDirection(up);
            break;
        case left:
            setDirection(right);
            break;
        case right:
            setDirection(left);
            break;
        }
    }
}

void RageBot::damage(int damageAmt) {
    decreaseHitPoints(damageAmt);
    getWorld()->playSound(SOUND_ROBOT_IMPACT);
    if (getHitPoints() <= 0) {
        getWorld()->increaseScore(getScoreValue());
        kill();
        getWorld()->playSound(SOUND_ROBOT_DIE);
    }
}

// THIEFBOT IMPLEMENTATIONS

ThiefBot::ThiefBot(StudentWorld* world, int imageID, int startX, int startY, int hitPoints, int score) 
: Robot(world, imageID, startX, startY, hitPoints, score, right), m_distanceBeforeTurning(randInt(1, 6)), m_carryingGoodie(false), m_stolenGoodie(nullptr) {
}

void ThiefBot::performAction() {
    if (getWorld()->getGoodieAt(getX(), getY()) != nullptr && !hasPickedUpGoodie()) {
        int random = randInt(1, 10);
        if (random == 1) { // 1 in 10 chance
            pickUpGoodie();
            return;
        }
    }

    if (hasNotMovedDistanceBeforeTurning()) {
        if (moveIfPossible()) {
            m_distanceBeforeTurning--;
            return; // moved in it's direction
        }
    }

    m_distanceBeforeTurning = randInt(1, 6);
    std::vector<int> attemptedDirections;
    int randomDir = randInt(1, 4);
    int temp = randomDir;
    for (int i = 0; i < 4; i++) {
        int x = getX();
        int y = getY();
        switch (randomDir) {
        case 1: // up
            y++;
            break;
        case 2: // down
            y--;
            break;
        case 3: // left
            x--;
            break;
        case 4: // right
            x++;
            break;
        }
        if (getWorld()->isActorAt(x, y) == false && isWithinBounds(x, y)) {
            switch (randomDir) {
            case 1:
                setDirection(up);
                break;
            case 2:
                setDirection(down);
                break;
            case 3:
                setDirection(left);
                break;
            case 4:
                setDirection(right);
                break;
            }
            if (moveIfPossible()) {
                m_distanceBeforeTurning--;
                return;
            }
        }

        attemptedDirections.push_back(randomDir);

        // consider another direction
        if (i < 3) {
            bool isNewDir = false;
            while (!isNewDir) {
                randomDir = randInt(1, 4);
                bool found = false;
                for (size_t i = 0; i < attemptedDirections.size(); i++) {
                    if (attemptedDirections[i] == randomDir) {
                        found = true;
                        break;
                    }
                }
                isNewDir = !found;
            }
        }
    }

    switch (temp) {
    case 1:
        setDirection(up);
        break;
    case 2:
        setDirection(down);
        break;
    case 3:
        setDirection(left);
        break;
    case 4:
        setDirection(right);
        break;
    }
}

void ThiefBot::damage(int damageAmt) {
    decreaseHitPoints(damageAmt);
    getWorld()->playSound(SOUND_ROBOT_IMPACT);
    if (getHitPoints() <= 0) {
        if (hasPickedUpGoodie()) {
            m_stolenGoodie->moveTo(getX(), getY());
            m_stolenGoodie->setStolen(false);
        }
        kill();
        getWorld()->playSound(SOUND_ROBOT_DIE);
        getWorld()->increaseScore(getScoreValue());
    }
}

bool ThiefBot::hasPickedUpGoodie() const {
    return m_carryingGoodie;
}

bool ThiefBot::hasNotMovedDistanceBeforeTurning() const {
    return (m_distanceBeforeTurning > 0);
}

void ThiefBot::pickUpGoodie() {
    Actor* goodie = getWorld()->getGoodieAt(getX(), getY());

    if (goodie != nullptr) {
        m_stolenGoodie = goodie;
        goodie->setStolen(true); // update the goodie's status
        m_carryingGoodie = true;
        getWorld()->playSound(SOUND_ROBOT_MUNCH);
    }
}

// REGULAR THIEFBOT IMPLEMENTATIONS

RegularThiefBot::RegularThiefBot(StudentWorld* world, int startX, int startY) 
    : ThiefBot(world, IID_THIEFBOT, startX, startY, 5, 10) {
}

// MEAN THIEFBOT IMPLEMENTATIONS

MeanThiefBot::MeanThiefBot(StudentWorld* world, int startX, int startY) 
    : ThiefBot(world, IID_MEAN_THIEFBOT, startX, startY, 8, 20) {
}


// THIEFBOT FACTORY IMPLEMENTATIONS

ThiefBotFactory::ThiefBotFactory(StudentWorld* world, int startX, int startY, ProductType type)
    : Actor(world, IID_ROBOT_FACTORY, startX, startY, none), m_type(type) {
    setVisible(true); // Make the factory visible
}

void ThiefBotFactory::doSomething() {
    int count = getWorld()->countThiefBotsSurroundingFactory(getX(), getY());

    if (count < 3 && !getWorld()->isThiefBotAt(getX(), getY())) {
        int random = randInt(1, 50);

        if (random == 1) {
            if (m_type == REGULAR)
                getWorld()->createNewThiefBot(getX(), getY(), 1);
            else if (m_type == MEAN)
                getWorld()->createNewThiefBot(getX(), getY(), 2);
            getWorld()->playSound(SOUND_ROBOT_BORN);
        }
    }
}

// PEA IMPLEMENTATIONS

Pea::Pea(StudentWorld* world, int startX, int startY, int startDirection)
    : Actor(world, IID_PEA, startX, startY, startDirection), m_newPea(true) {
    setVisible(true);
}

void Pea::doSomething() {
    if (!isAlive()) {
        return;
    }
    
    if (m_newPea) {
        m_newPea = false;
        return;
    }

    if (checkForActors()) {
        return;
    }

    switch (getDirection()) {
        case up:
            moveTo(getX(), getY() + 1); 
            break;
        case down:
            moveTo(getX(), getY() - 1);
            break;
        case left:
            moveTo(getX() - 1, getY());
            break;
        case right:
            moveTo(getX() + 1, getY());
            break;
    }
    
    checkForActors();

}

bool Pea::checkForActors() {
    if (getWorld()->isObstacleAt(getX(), getY())) {
        Actor* actor = getWorld()->getDestroyableActorAt(getX(), getY());
        if (actor != nullptr) {
                actor->damage(2);
                // NOTE: If a pea finds itself on a square with both a robot and a factory, then the pea must damage the robot, can do this by creating a getDestroyableActor() function
                kill();
                return true;
        }
        else { // non-destroyable actor like a wall or factory
            kill();
            return true;;
        } 
    }
    return false;
}

// EXIT IMPLEMENTATIONS

Exit::Exit(StudentWorld* world, int startX, int startY)
    : Actor(world, IID_EXIT, startX, startY, none) {
    setVisible(false);
}

void Exit::doSomething() {
    if (!getWorld()->collectedCrystals())
        return;
    int x = getWorld()->getPlayer()->getX();
    int y = getWorld()->getPlayer()->getY();
    if (getX() == x && getY() == y) {
        getWorld()->playSound(SOUND_FINISHED_LEVEL);
        getWorld()->increaseScore(2000);
        getWorld()->increaseScore(getWorld()->getBonus());
    }
}

// WALL IMPLEMENTATIONS

Wall::Wall(StudentWorld* world, int startX, int startY)
    : Actor(world, IID_WALL, startX, startY, none) {
    setVisible(true);
}

// MARBLE IMPLEMENTATIONS

Marble::Marble(StudentWorld* world, int startX, int startY) 
    : Actor(world, IID_MARBLE, startX, startY, none), m_hitpoints(10) {
    setVisible(true);
}
void Marble::damage(int damageAmt) {
    m_hitpoints -= damageAmt;
    if (m_hitpoints <= 0) {
        kill();
    }
}

bool Marble::bePushedBy(Agent* a, int x, int y) {
    switch (a->getDirection()) {
        case up:
            if (getWorld()->canMarbleMoveTo(getX(), getY() + 1)) {
                return true;
            }
            break;
        case down:
            if (getWorld()->canMarbleMoveTo(getX(), getY() - 1)) {
                return true;
            }
            break;
        case left:
            if (getWorld()->canMarbleMoveTo(getX() - 1, getY())) {
                return true;
            }
            break;
        case right:
            if (getWorld()->canMarbleMoveTo(getX() + 1, getY())) {
                return true;
            }
            break;
    }
    return false;
}

// PIT IMPLEMENTATIONS

Pit::Pit(StudentWorld* world, int startX, int startY) 
    : Actor(world, IID_PIT, startX, startY, none) {
}

void Pit::doSomething() {
    if (!isAlive()) {
        return;
    }

    Actor* marble = getWorld()->getMarbleAt(getX(), getY());
    if ( marble != nullptr) {
        marble->kill();
        kill();
    }
}


// ITEM IMPLEMENTATIONS

Item::Item(StudentWorld* world, int imageID, int startX, int startY, int score)
    : Actor(world, imageID, startX, startY, none), m_scoreValue(score) {
    setVisible(true);
}

int Item::getScoreValue() const {
    return m_scoreValue;
}

// CRYSTAL IMPLEMENTATIONS

Crystal::Crystal(StudentWorld* world, int startX, int startY)
    : Item(world, IID_CRYSTAL, startX, startY, 50) {
}

void Crystal::doSomething() {
    if (!isAlive()) {
        return;
    }

    Player* player = getWorld()->getPlayer();
    if (player != nullptr && getX() == player->getX() && getY() == player->getY()) {
        getWorld()->increaseScore(getScoreValue());
        kill();
        getWorld()->reduceCrystalsByOne();
        getWorld()->playSound(SOUND_GOT_GOODIE);
    }
}

// GOODIE IMPLEMENTATION

Goodie::Goodie(StudentWorld* world, int imageID, int startX, int startY, int score)
    : Item(world, imageID, startX, startY, score), m_stolen(false) {
}

void Goodie::doSomething() {
    if (!isAlive()) {
        return;
    }

    Player* player = getWorld()->getPlayer();
    if (player != nullptr && getX() == player->getX() && getY() == player->getY() && !m_stolen) {
        getWorld()->increaseScore(getScoreValue());
        performAction();
        kill();
        getWorld()->playSound(SOUND_GOT_GOODIE);
    }
}

void Goodie::setStolen(bool status) {
    if (status)
        setVisible(false);
    else
        setVisible(true);
    m_stolen = status;
}

// EXTRA LIFE GOODIE IMPLEMENTATION

ExtraLifeGoodie::ExtraLifeGoodie(StudentWorld* world, int startX, int startY) 
    : Goodie(world, IID_EXTRA_LIFE, startX, startY, 1000) {
}

void ExtraLifeGoodie::performAction() {
    getWorld()->incLives();
}

// RESTORE HEALTH GOODIE IMPLEMENTATION

RestoreHealthGoodie::RestoreHealthGoodie(StudentWorld* world, int startX, int startY)
    : Goodie(world, IID_RESTORE_HEALTH, startX, startY, 500) {
}

void RestoreHealthGoodie::performAction() {
    getWorld()->getPlayer()->restorePlayerHitPoints();
}


// AMMO GOODIE IMPLEMENTATION

AmmoGoodie::AmmoGoodie(StudentWorld* world, int startX, int startY)
    : Goodie(world, IID_AMMO, startX, startY, 100) {
}

void AmmoGoodie::performAction() {
    getWorld()->getPlayer()->increaseAmmo(20);
}