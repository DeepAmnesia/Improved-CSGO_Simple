#pragma once

#include "../hooks.hpp"
#include "../configurations.hpp"
#include "../functions/damage_indicator.h"
#define MAX_FLOATING_TEXTS 50

class C_HookedEvents : public IGameEventListener2
{
public:
	void FireGameEvent(IGameEvent* event);
	void RegisterSelf();
	void RemoveSelf();
	int GetEventDebugID(void);
};

extern C_HookedEvents HookedEvents;
