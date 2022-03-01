#pragma once

#include "../sdk/csgostructs.hpp"
#include "../sdk/utils/singleton.hpp"

enum NadeThrowAct
{
	ACT_NONE,
	ACT_THROW,
	ACT_LOB,
	ACT_DROP
};

class Grenade_Pred : public Singleton<Grenade_Pred>
{
public:
	void Predict(CUserCmd* ucmd);
	bool Detonated(C_BaseCombatWeapon* weapon, float time, trace_t& trace);
	void Trace(CUserCmd* ucmd);
	void DrawPrediction();
};

class NadePoint 
{
public:
	NadePoint() 
	{
		m_valid = false;
	}

	NadePoint(Vector start, Vector end, bool plane, bool valid, Vector normal, bool detonate) 
	{
		m_start = start;
		m_end = end;
		m_plane = plane;
		m_valid = valid;
		m_normal = normal;
		m_detonate = detonate;
	}

	Vector m_start, m_end, m_normal;
	bool m_valid, m_plane, m_detonate;
};