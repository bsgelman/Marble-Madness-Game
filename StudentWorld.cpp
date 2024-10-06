#include "StudentWorld.h"
#include "GameConstants.h"
#include "Level.h" 
#include "Actor.h"
#include <iostream>
#include <sstream>
#include <iomanip>
using namespace std;

GameWorld* createStudentWorld(string assetPath)
{
	return new StudentWorld(assetPath);
}

// Students:  Add code to this file, StudentWorld.h, Actor.h, and Actor.cpp

StudentWorld::StudentWorld(string assetPath)
: GameWorld(assetPath), m_player(nullptr), m_bonus(1000), m_crystals(0)
{
}

StudentWorld::~StudentWorld() {
    cleanUp();
}

int StudentWorld::init()
{
    m_bonus = 1000;
    switch (loadLevel()) {
        case -1:
            return GWSTATUS_LEVEL_ERROR;
        case 1:
            return GWSTATUS_PLAYER_WON;
        case 0:
            return GWSTATUS_CONTINUE_GAME;            
    }

    return GWSTATUS_LEVEL_ERROR; // should never reach here
}

int StudentWorld::move()
{   
    setDisplayText();

    if (m_player != nullptr) { // is it ok to have this after all actors doSomething?
        m_player->doSomething();
    }

    for (size_t k = 0; k < actors.size(); k++) {
        actors[k]->doSomething();
    }

    if (!isPlayerAlive()) {
        decLives();
        playSound(SOUND_PLAYER_DIE);
        return GWSTATUS_PLAYER_DIED;
    }

    if (collectedCrystals()) { // if 0 crystals are left, check if the player is on the coordinates of the exit
        int x = m_player->getX();
        int y = m_player->getY();
        for (vector<Actor*>::iterator p = actors.begin(); p != actors.end(); p++) {
            if ((*p)->isInvisibleAtFirst()) {
                if ((*p)->getX() == x && (*p)->getY() == y) {
                    return GWSTATUS_FINISHED_LEVEL;
                }
            }
        }
    }

    removeDeadGameObjects();
    
    reduceLevelBonusByOne();

	return GWSTATUS_CONTINUE_GAME;
}

void StudentWorld::cleanUp()
{   
    std::vector<Actor*>::iterator p = actors.begin();
    while (p != actors.end()) {
        delete* p;
        p = actors.erase(p);
    }

    delete m_player;
    m_player = nullptr;
}

int StudentWorld::loadLevel() {
    if (getLevel() > 99) {
        return 1; // GAME WON
    }

    ostringstream oss;

    oss << "level" << setw(2) << setfill('0') << getLevel() << ".txt";

    string curLevel = oss.str();
    Level lev(assetPath());
    Level::LoadResult result = lev.loadLevel(curLevel);
    if (result == Level::load_fail_file_not_found || result == Level::load_fail_bad_format)
        return -1; // something bad happened!
    
    for (int x = 0; x < VIEW_WIDTH; x++) {
        for (int y = 0; y < VIEW_HEIGHT; y++) {
            Level::MazeEntry item = lev.getContentsOf(x, y);
            switch (item) {
            case Level::player:
                m_player = new Player(this, x, y);
                break;
            case Level::exit:
                actors.push_back(new Exit(this, x, y));
                break;
            case Level::crystal:
                actors.push_back(new Crystal(this, x, y));
                m_crystals++;
                break;
            case Level::horiz_ragebot:
                actors.push_back(new RageBot(this, x, y, GraphObject::right));
                break;
            case Level::vert_ragebot:
                actors.push_back(new RageBot(this, x, y, GraphObject::down));
                break;
            case Level::thiefbot_factory:
                actors.push_back(new ThiefBotFactory(this, x, y, ThiefBotFactory::REGULAR));
                break;
            case Level::mean_thiefbot_factory:
                actors.push_back(new ThiefBotFactory(this, x, y, ThiefBotFactory::MEAN));
                break;
            case Level::wall:
                actors.push_back(new Wall(this, x, y));
                break;
            case Level::marble:
                actors.push_back(new Marble(this, x, y));
                break;
            case Level::pit:
                actors.push_back(new Pit(this, x, y));
                break;
            case Level::extra_life:
                actors.push_back(new ExtraLifeGoodie(this, x, y));
                break;
            case Level::restore_health:
                actors.push_back(new RestoreHealthGoodie(this, x, y));
                break;
            case Level::ammo:
                actors.push_back(new AmmoGoodie(this, x, y));
                break;
            default:
                // Empty should be here
                break;
            }
        }
    }
    return 0;
}

Player* StudentWorld::getPlayer() const {
    return m_player;
}

int StudentWorld::getBonus() const{
    return m_bonus;
}

int StudentWorld::getPlayerHealth() const {
    if (m_player != nullptr) {
        return m_player->getHealthPct();
    }
    return 0; // player isnt created
}

int StudentWorld::getPlayerAmmo() const {
    if (m_player != nullptr) {
        return m_player->getAmmo();
    }
    return 0; // player isnt created
}

bool StudentWorld::isPlayerAlive() const {
    if (m_player != nullptr && m_player->isAlive()) {
        return true;
    }
    return false; // player isnt created
}

bool StudentWorld::canMarbleMoveTo(int x, int y) const {
    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        if ((*p)->getX() == x && (*p)->getY() == y) {
            if ((*p)->allowsMarble() == false) {
                if ((*p)->isStealable() && (*p)->isVisible() == false)
                    continue;
                return false;
            }
        }
    }
    return true;
}

bool StudentWorld::isActorAt(int x, int y) const {
    if (m_player->getX() == x && m_player->getY() == y)
        return true;
    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        if ((*p)->getX() == x && (*p)->getY() == y) {
            if ((*p)->allowsAgentColocation() == false) {
                return true;
            }
        }
    }
    return false;
}

bool StudentWorld::isObstacleAt(int x, int y) const {
    if (m_player->getX() == x && m_player->getY() == y)
        return true;
    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        if ((*p)->getX() == x && (*p)->getY() == y) {
            if ((*p)->allowsAgentColocation() == false && (*p)->allowsMarble() == false) { // obstacles that are non-pits
                return true;
            }
        }
    }
    return false;
}

bool StudentWorld::isThiefBotAt(int x, int y) const {
    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        Actor* actor = *p;
        if (actor->getX() == x && actor->getY() == y) {
            if (actor->countsInFactoryCensus()) {
                return true;
            }
        }
    }
    return false;
}

Actor* StudentWorld::getActorAt(int x, int y) const {
    if (m_player->getX() == x && m_player->getY() == y)
        return m_player;
    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        if ((*p)->getX() == x && (*p)->getY() == y) {
            if ((*p)->allowsAgentColocation() == false) {
                return (*p);
            }
        }
    }
    return nullptr;
}

Actor* StudentWorld::getMarbleAt(int x, int y) const {
    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        if ((*p)->getX() == x && (*p)->getY() == y) {
            if ((*p)->isSwallowable()) {
                return (*p);
            }
        }
    }
    return nullptr;
}

Actor* StudentWorld::getGoodieAt(int x, int y) const {
    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        if ((*p)->getX() == x && (*p)->getY() == y) {
            if ((*p)->isStealable()) {
                return (*p);
            }
        }
    }
    return nullptr;
}

Actor* StudentWorld::getDestroyableActorAt(int x, int y) const {
    if (m_player->getX() == x && m_player->getY() == y)
        return m_player;
    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        Actor* actor = *p;
        if (actor->getX() == x && actor->getY() == y) {
            if (actor->isDestroyable()) {
                return actor;
            }
        }
    }
    return nullptr;
}

int StudentWorld::countThiefBotsSurroundingFactory(int x, int y) const {
    int count = 0;
    int minX = x - 3;
    int maxX = x + 3;
    int minY = y - 3;
    int maxY = y + 3;

    for (vector<Actor*>::const_iterator p = actors.begin(); p != actors.end(); p++) {
        Actor* actor = *p;
        if (actor->countsInFactoryCensus() && actor->getX() >= minX && actor->getX() <= maxX && actor->getY() >= minY && actor->getY() <= maxY) {
            count++;
        }
    }
    return count;
}

void StudentWorld::setDisplayText() {
    ostringstream oss;

    oss << "Score: " << setw(7) << setfill('0') << getScore() << "  Level: " << setw(2) << setfill('0') << getLevel() << "  Lives: " << setw(2) << setfill(' ') << getLives() << "  Health: " << setw(3) << setfill(' ') << m_player->getHealthPct() << "%" << "  Ammo: " << setw(3) << setfill(' ') << m_player->getAmmo() << "  Bonus: " << setw(4) << setfill(' ') << m_bonus;

    string display = oss.str();

    setGameStatText(display);
}

void  StudentWorld::removeDeadGameObjects() {
    for (vector<Actor*>::iterator p = actors.begin(); p != actors.end();) {
        if ((*p)->isAlive() == false) {
            delete *p;
            p = actors.erase(p);
        }
        else {
            p++;
        }
    }
}

void StudentWorld::reduceLevelBonusByOne() {
    if (m_bonus > 0)
        m_bonus--;
}

void StudentWorld::reduceCrystalsByOne() {
    m_crystals--;
}

bool StudentWorld::collectedCrystals() {
    if (m_crystals <= 0) {
        for (vector<Actor*>::iterator p = actors.begin(); p != actors.end(); p++) {
            if ((*p)->isInvisibleAtFirst()) {
                if ((*p)->isVisible() == false) {
                    playSound(SOUND_REVEAL_EXIT);
                }
                (*p)->setVisible(true);
                return true;
            }
        }
    }
    return false;
}

void StudentWorld::createNewPea(int x, int y, int direction) {
    actors.push_back(new Pea(this, x, y, direction));
}

void StudentWorld::createNewThiefBot(int x, int y, int type) {
    // Create a new ThiefBot based on the type
    if (type == 1) {
        actors.push_back(new RegularThiefBot(this, x, y));
    }
    else if (type == 2) {
        actors.push_back(new MeanThiefBot(this, x, y));
    }
}