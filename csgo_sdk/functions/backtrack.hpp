#pragma once
#include "../sdk/csgostructs.hpp"
#include <map>
#include <deque>

struct backtrack_data {
	C_BasePlayer* player;
	mstudiohitboxset_t* hitboxset;
	float simTime;
	Vector hitboxPos;
	matrix3x4_t boneMatrix[128];
};

class CBacktrack {
public:
	void CMove(CUserCmd* pCmd);
private:
	float correct_time = 0.0f;
	float latency = 0.0f;
	float lerp_time = 0.0f;
public:
	std::map<int, std::deque<backtrack_data>> data;
};

extern CBacktrack* g_Backtrack;
