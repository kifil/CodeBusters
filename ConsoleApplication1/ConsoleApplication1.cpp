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
	search, approach, capture, goBack, release
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

	Entity();
	Entity(int, int, int, int, int, int);
	void UpdateEntity(int, int, int, int);
	
	string GetAction();

};

vector<Entity> visibleEntities;
vector<Entity> myBusters;
vector<Entity> ghosts;

Entity::Entity()
{}

Entity::Entity(int a, int b, int c, int d, int e,  int g)
{
	entityID = a;
	position = Point(b, c);
	entityType = EntityType(d);
	entityState = EntityState(e);
	value = g;

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

	//update search if nothing close
	//TODO improve search algorithim
	if (GetDistance(position, searchTarget) < 200) {
		SetNewSearchTarget();
	}
}

void Entity::SetNewSearchTarget() {
	int x = rand() % XMAX;
	int y = rand() % YMAX;
	searchTarget = Point(x, y);
}

bool Entity::IsNearGhost() {

	for (Entity ghost : ghosts) {
		//TODO make it move away when near a ghost instead of calling it not near
		if (GetDistance(ghost.position, position) < 1650 && GetDistance(ghost.position, position) > 900) {
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
	if (entityState == EntityState::carryingGhost) {
		score -= 200;
	}
	return score;
}

int Entity::ScoreGoBack() {
	int score = 0;
	if (entityState == EntityState::carryingGhost) {
		score += 250;
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


string Entity::GetAction() {
	vector<tuple<int, EntityAction>> scores;
	tuple<int,EntityAction>	searchScore(ScoreSearch(), EntityAction::search);
	tuple<int, EntityAction> captureScore(ScoreCapture(), EntityAction::capture);
	tuple<int, EntityAction> goBackScore(ScoreGoBack(), EntityAction::goBack);
	tuple<int, EntityAction> releaseScore(ScoreRelease(), EntityAction::release);

	scores.push_back(searchScore);
	scores.push_back(captureScore);
	scores.push_back(goBackScore);
	scores.push_back(releaseScore);
	
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

			for (Entity ghost : ghosts) {
				if (GetDistance(ghost.position, position) < closestGhostDistance) {
					closestGhostDistance = GetDistance(ghost.position, position);
					closestGhostId = ghost.entityID;
				}
			}
			actionText = "BUST " + to_string(closestGhostId);
			break;
		case EntityAction::goBack:
			actionText = "MOVE " + to_string(baseLocation.x) + " " + to_string(baseLocation.y);
			break;
		case EntityAction::release:
			actionText = "RELEASE";
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
	EntityType myTeam = EntityType(myTeamId);

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
	for (Entity buster : myBusters) {
		cout << buster.GetAction() << endl;
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


