// ConsoleApplication1.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <vector>
#include <math.h>
#include <tuple>
#include <stdlib.h>
#include <time.h>
#include <string>

using namespace std;

int XMIN = 0;
int YMIN = 0;
int XMAX = 16001;
int YMAX = 9001;

struct Point {
	int x;
	int y;
	Point() {}
	Point(int x1, int y1) {
		x = x1;
		y = y1;
	}
};

enum class EntityState {
	idle = 0,
	carryingGhost = 1
};
enum class EntityType {
	teamOrange = 0,
	teamPurple = 1,
	ghost = -1
};
enum class EntityAction {
	search, approach, capture, goBack, release, stun
};

int GetDistance(Point point1, Point point2)
{
	int xResult = (point2.x - point1.x) ^ 2;
	int yResult = (point2.y - point1.y) ^ 2;
	return sqrt((pow((point2.x - point1.x),2) + pow((point2.y - point1.y),2)));
}

class Entity {
	int ScoreSearch();
	int ScoreApproach(); //tODO implement this
	int ScoreCapture();
	int ScoreGoBack();
	int ScoreRelease();
	int ScoreStun();
	bool IsNearGhost();
	void SetNewSearchTarget();
public:
	int entityID;
	Point position;
	Point searchTarget;
	Point baseLocation;
	EntityType entityType;
	EntityState entityState;
	EntityAction entityAction;
	int value;
	int timeSinceLastStun;

	Entity();
	Entity(int, int, int, int, int, int);
	void UpdateEntity(int, int, int, int);
	Entity GetClosestEntityByType(vector<Entity>*, EntityType);
	
	string GetAction();

};

vector<Entity> visibleEntities;
vector<Entity> myBusters;
vector<Entity> ghosts;
EntityType myTeam;
EntityType enemyTeam;

Entity::Entity()
{}

Entity::Entity(int a, int b, int c, int d, int e,  int g)
{
	entityID = a;
	position = Point(b, c);
	entityType = EntityType(d);
	entityState = EntityState(e);
	value = g;
	timeSinceLastStun = 20;

	SetNewSearchTarget();

	if (entityType == EntityType::teamOrange) {
		baseLocation = Point(XMIN, YMIN);
	}
	else if (entityType == EntityType::teamPurple) {
		baseLocation = Point(XMAX, YMAX);
	}
}

void Entity::UpdateEntity(int a, int b, int c, int d)
{
	//this isnt actually updating the class, or the vector isnt being updated
	position.x = a;
	position.y = b;
	entityState = EntityState(c);
	value = d;

	//may not want to autoincrement this in case functionis called elsewhere
	timeSinceLastStun++;

	//update search if nothing close

	if (GetDistance(position, searchTarget) < 200) {
		SetNewSearchTarget();
	}
}

//TODO improve search algorithim
void Entity::SetNewSearchTarget() {
	int x = rand() % XMAX;
	int y = rand() % YMAX;
	searchTarget = Point(x, y);
}

Entity Entity::GetClosestEntityByType(vector<Entity>* entities, EntityType type) {
	int closestEntityDistance = 999999;
	Entity closestEntity;

	for (Entity entity : *entities) {
		if (GetDistance(entity.position, position) < closestEntityDistance && entity.entityType == type) {
			closestEntityDistance = GetDistance(entity.position, position);
			closestEntity = entity;
		}
	}
	return closestEntity;
}

bool Entity::IsNearGhost() {

	for (Entity ghost : ghosts) {
		//TODO make it move away when near a ghost instead of calling it not near
		if (GetDistance(ghost.position, position) < 1750 && GetDistance(ghost.position, position) > 900) {
			return true;
		}
	}
	return false;
}

int Entity::ScoreSearch() {
	int score = 0;
	if (entityState != EntityState::carryingGhost) {
		score += 100;
	}
	if (IsNearGhost()) {
		score -= 100;
	}
	return score;
}

//int Entity::ScoreApproach();
int Entity::ScoreCapture() {
	int score = 0;

	if (IsNearGhost()) {
		score += 100;
	}
	else {
		score -= 100;
	}
	if (entityState == EntityState::carryingGhost) {
		score -= 100;
	}
	else {
		score += 100;
	}
	return score;
}

int Entity::ScoreGoBack() {
	int score = 0;
	if (entityState == EntityState::carryingGhost) {
		score += 100;
	}
	if (entityState != EntityState::carryingGhost) {
		score -= 100;
	}
	return score;
}

int Entity::ScoreRelease() {
	int score = 0;
	if (entityState == EntityState::carryingGhost) {
		score += 100;
	}
	if (GetDistance(position, baseLocation) < 1600) {
		score += 50;
	}
	if (entityState != EntityState::carryingGhost) {
		score -= 150;
	}
	return score;
}

int Entity::ScoreStun() {
	int score = 0;

	Entity colsestEnemy = GetClosestEntityByType(&visibleEntities, enemyTeam);
	if (GetDistance(colsestEnemy.position, position) < 1700 && timeSinceLastStun >= 20) {
		score += 250;
	}
	else {
		score -= 250;
	}

	return score;
}


string Entity::GetAction() {
	vector<tuple<int, EntityAction>> scores;
	tuple<int,EntityAction>	searchScore(ScoreSearch(), EntityAction::search);
	tuple<int, EntityAction> captureScore(ScoreCapture(), EntityAction::capture);
	tuple<int, EntityAction> goBackScore(ScoreGoBack(), EntityAction::goBack);
	tuple<int, EntityAction> releaseScore(ScoreRelease(), EntityAction::release);
	tuple<int, EntityAction> stunScore(ScoreStun(), EntityAction::stun);

	scores.push_back(searchScore);
	scores.push_back(captureScore);
	scores.push_back(goBackScore);
	scores.push_back(releaseScore);
	scores.push_back(stunScore);
	
	int highestScore = 0;
	EntityAction chosenAction;

	for (tuple<int, EntityAction>score : scores) {
		if (get<0>(score) > highestScore) {
			highestScore = get<0>(score);
			chosenAction = get<1>(score);
		}
	}

	string actionText = "";
	int closestGhostDistance = 9999999;
	int closestGhostId = 0;

	//todo: move logic into functions
	switch (chosenAction) {
		case EntityAction::search:
			actionText = "MOVE " + to_string(searchTarget.x) + " " + to_string(searchTarget.y);
			break;
		case EntityAction::capture:
			closestGhostId = GetClosestEntityByType(&visibleEntities, EntityType::ghost).entityID;
			actionText = "BUST " + to_string(closestGhostId);
			break;
		case EntityAction::goBack:
			actionText = "MOVE " + to_string(baseLocation.x) + " " + to_string(baseLocation.y);
			break;
		case EntityAction::release:
			actionText = "RELEASE";
			break;
		case EntityAction::stun:
			timeSinceLastStun = 0;
			actionText = "STUN " + to_string(GetClosestEntityByType(&visibleEntities, enemyTeam).entityID);
			//TODO, need to reset time since last stun
			break;
		default:
			actionText = "no action chosen";
			break;
	}

	return actionText;
}


int main()
{
	int myTeamId = 0;

	visibleEntities.clear();
	ghosts.clear();
	srand(time(NULL));
	myTeam = EntityType(myTeamId);

	if (myTeam == EntityType::teamOrange) {
		enemyTeam = EntityType::teamPurple;
	}
	else {
		enemyTeam = EntityType::teamOrange;
	}

	//game paramaters
	int entities = 2;

	//push in all entities each frame to visible entities
	Entity myEntity(1, 2, 3, 0, 1, 1);
	Entity myGhost(123, 900, 1000, -1, 2, 1);
	visibleEntities.push_back(myEntity);
	visibleEntities.push_back(myGhost);

	//add new entities
	for (Entity visibleEntity : visibleEntities) {
		if (visibleEntity.entityType == myTeam) {
			bool busterExists = false;
			for (Entity buster : myBusters) {
				if (buster.entityID == visibleEntity.entityID) {
					busterExists = true;
					break;
				}
			}
			if (!busterExists) {
				myBusters.push_back(visibleEntity);
			}
			
		}
		if (visibleEntity.entityType == EntityType::ghost) {
			//can just reset all the ghosts all the time
			ghosts.push_back(visibleEntity);
		}
	}

	//update busters
	for (Entity visibleEntity : visibleEntities) {
		for (int i = 0; i < myBusters.size(); i++){
			if (myBusters[i].entityID == visibleEntity.entityID) {
				myBusters[i].UpdateEntity(visibleEntity.position.x, visibleEntity.position.y, (int)visibleEntity.entityState, visibleEntity.value);
			}
		}
	}

	//get buster action
	for (int i = 0; i < myBusters.size(); i++) {
		cout << myBusters[i].GetAction() << endl;
	}


	return 0;
}

//seach, approach, capture, return, release
//if no ghost near and not carrying ghpst
//search out
//only search within lane
//set to search return at map edge
//if ghost near
// not in range, but in lane
//approachs
//in range, and in lane
//capture
//if have ghost
//not close enough to realse
//return
//clsoe enough to relese
//release


