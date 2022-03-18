#pragma once

#include "../hooks.hpp"
#include "../configurations.hpp"
#include "../functions/damage_indicator.h"
#define MAX_FLOATING_TEXTS 50

class C_HookedEvents : public IGameEventListener2
{
public:
	void DrawBeams();
	void DrawBeam(bool local_tracer, const Vector& src, const Vector& end, Color color);
	struct ImpactData
	{
		C_BasePlayer* pEntity;
		Vector ImpactPosition;
		float Time;
	};

	std::vector<ImpactData> impacts;
	void FireGameEvent(IGameEvent* event);
	void RegisterSelf();
	void RemoveSelf();
	int GetEventDebugID(void);
};

extern C_HookedEvents HookedEvents;
