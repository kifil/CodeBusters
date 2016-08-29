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
	carryingGhost = 1,
	stunned = 2,
	trapping = 3,
};
enum class EntityType {
	teamOrange = 0,
	teamPurple = 1,
	ghost = -1
};
enum class EntityAction {
	search, approach, capture, goBack, release, stun, assist
};

int GetDistance(Point point1, Point point2)
{
	int xResult = (point2.x - point1.x) ^ 2;
	int yResult = (point2.y - point1.y) ^ 2;
	return sqrt((pow((point2.x - point1.x),2) + pow((point2.y - point1.y),2)));
}

class Entity {
	int ScoreSearch();
	int ScoreCapture();
	int ScoreGoBack();
	int ScoreRelease();
	int ScoreStun();
	int ScoreApproach();
	int ScoreAssist();
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
	vector<Entity> GetEntitiesInRangeByType(vector<Entity>* entities, EntityType type, int range);
	
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
		if (GetDistance(entity.position, position) < closestEntityDistance
			&& entity.entityType == type
			&& entity.entityID != entityID) {
			closestEntityDistance = GetDistance(entity.position, position);
			closestEntity = entity;
		}
	}
	return closestEntity;
}

vector<Entity> Entity::GetEntitiesInRangeByType(vector<Entity>* entities, EntityType type, int range) {
	vector<Entity> entitiesInRange;

	for (Entity entity : *entities) {
		if (GetDistance(entity.position, position) < range
			&& entity.entityType == type
			&& entity.entityID != entityID) {
				entitiesInRange.push_back(entity);
		}
	}
	return entitiesInRange;
}

bool Entity::IsNearGhost() {

	for (Entity entity : visibleEntities) {
		//TODO make it move away when near a ghost instead of calling it not near
		if (entity.entityType == EntityType::ghost
			&& GetDistance(entity.position, position) < 1750
			&& GetDistance(entity.position, position) > 900) {
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
		score += 125;
	}
	if (entityState != EntityState::carryingGhost) {
		score -= 125;
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

//todo:add approach enemy, use stun score + enemy visible distance and see if they are carrying

//TODO: make get nearby entities within range function
//then use that in the stun behavior to stun enemy in range with ghsot
int Entity::ScoreStun() {
	int score = 0;

	Entity colsestEnemy = GetClosestEntityByType(&visibleEntities, enemyTeam);
	//todo update stun score if enemy no near enemy carring ghsot?
	if (GetDistance(colsestEnemy.position, position) < 1749 && timeSinceLastStun >= 20) {
		score += 250;
	}
	else {
		score -= 250;
	}

	return score;
}

int Entity::ScoreApproach() {
	int score = 0;

	Entity colsestGhost = GetClosestEntityByType(&visibleEntities, EntityType::ghost);
	int colsestGhostDistance = GetDistance(colsestGhost.position, position);

	if (colsestGhostDistance < 2200 && colsestGhostDistance >= 1749) {
		score += 100;
	}
	else {
		score -= 100;
	}
	if (entityState == EntityState::carryingGhost || entityState == EntityState::trapping) {
		score -= 100;
	}
	else {
		score += 100;
	}

	return score;
}

int Entity::ScoreAssist() {
	int score = 0;
	int closestAllyDistance = 999999;
	Entity closestAlly;
	vector<Entity> nearbyFriendlyBusters;

	for (Entity visibleEntity : visibleEntities) {
		int allyDistance = GetDistance(visibleEntity.position, position);
		if (visibleEntity.entityType == myTeam
			&& visibleEntity.entityID != entityID
			&& allyDistance < 7000) {
			//get list of nearby in case we want it later
				nearbyFriendlyBusters.push_back(visibleEntity);
				//set closest ally data
				if (allyDistance < closestAllyDistance) {
					closestAllyDistance = allyDistance;
					closestAlly =  visibleEntity;
				}
		}
	}
	if (entityState == EntityState::trapping || entityState == EntityState::carryingGhost) {
		score -= 150;
	}

	if (closestAlly.entityState == EntityState::trapping) {
		score += 150;
	}
	else {
		score -= 150;
	}

	//let capture and stun take priority
	if (ScoreCapture() > 0) {
		score -= ScoreCapture();
	}
	if (ScoreStun() > 0) {
		score -= ScoreStun();
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
	tuple<int, EntityAction> approachScore(ScoreApproach(), EntityAction::approach);
	tuple<int, EntityAction> assistScore(ScoreAssist(), EntityAction::assist);

	scores.push_back(searchScore);
	scores.push_back(captureScore);
	scores.push_back(goBackScore);
	scores.push_back(releaseScore);
	scores.push_back(stunScore);
	scores.push_back(approachScore);
	scores.push_back(assistScore);
	
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
	Point closestAllyPosition;
	vector<Entity> enemies;
	Entity enemyToStun;

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
			enemies = GetEntitiesInRangeByType(&visibleEntities, enemyTeam, 1749);
			enemyToStun = enemies[0];
			for (Entity enemy : enemies) {
				if (enemy.entityState == EntityState::carryingGhost) {
					enemyToStun = enemy;
				}
			}
			actionText = "STUN " + to_string(enemyToStun.entityID);
			break;
		case EntityAction::approach:
			searchTarget = GetClosestEntityByType(&visibleEntities, EntityType::ghost).position;
			actionText = "MOVE " + to_string(searchTarget.x) + " " + to_string(searchTarget.y);
			break;
		case EntityAction::assist:
			closestAllyPosition = GetClosestEntityByType(&visibleEntities, myTeam).position;
			actionText = "MOVE " + to_string(closestAllyPosition.x) + " " + to_string(closestAllyPosition.y);
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
			if (myBusters[i].entityID == visibleEntity.entityID && myBusters[i].entityType == visibleEntity.entityType) {
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


