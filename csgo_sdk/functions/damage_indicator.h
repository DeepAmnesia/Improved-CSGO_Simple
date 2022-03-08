#pragma once

#include "../hooks.hpp"
#include "../configurations.hpp"
#include "../sdk/utils/math.hpp"
#include "../render/render.hpp"
#define MAX_FLOATING_TEXTS 50


struct DamageIndicator_t {
	bool valid = false;
	float startTime = 1.f;
	int damage = 0;
	int hitgroup = 0;
	Vector hitPosition = Vector(0, 0, 0);
	int randomIdx = 0;
};

class DamageIndicators {
public:
	std::vector<DamageIndicator_t> floatingTexts;
	int floatingTextsIdx = 0;
	Vector c_impactpos = Vector(0, 0, 0);
	void paint();


};

extern DamageIndicators damage_indicators;