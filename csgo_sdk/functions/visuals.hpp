#pragma once

#include "../sdk/utils/singleton.hpp"
#include "../render/render.hpp"
#include "../sdk/utils/math.hpp"
#include "../sdk/csgostructs.hpp";
#include "../helpers/keybinds.hpp"
#include "../config.hpp"
#include "../render/menu.hpp"
#include "../functions/weapon_groups.hpp"

class Visuals : public Singleton<Visuals>
{
	friend class Singleton<Visuals>;

	CRITICAL_SECTION cs;

	Visuals();
	~Visuals();
private:
	bool third_person_pressed = false;;
	CKeyBind thirdperson_key = CKeyBind(&third_person_pressed, &g_Configurations.misc_thirdperson, "Third person");
public:
	
	class Player
	{
	public:
		struct
		{
			C_BasePlayer* pl;
			bool          is_enemy;
			bool          is_visible;
			Color         clr;
			Vector        head_pos;
			Vector        head_pos_def;
			Vector        feet_pos;
			RECT          bbox;
		} ctx;
		bool Begin(C_BasePlayer * pl);
		void RenderBox();
		void RenderName();
		void RenderWeaponName();
		void RenderHealth();
		void RenderArmour();
		void RenderSnapline();
		void OutOfScreenArrow();
		void DrawHeadDot();
	};
	void RenderWeapon(C_BaseCombatWeapon* ent);
	void RenderDefuseKit(C_BaseEntity* ent);
	void RenderPlantedC4(C_BaseEntity* ent);
	void RenderItemEsp(C_BaseEntity* ent);
	void ThirdPerson();
	void ShowFov();
public:
	void AddToDrawList();
	void Render();
	void SetupValues();
	void MenuVisuals();
};
