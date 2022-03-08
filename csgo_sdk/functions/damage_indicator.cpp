#include "damage_indicator.h"
DamageIndicators damage_indicators;

void DamageIndicators::paint() 
{
	if (!g_EngineClient->IsInGame() || !g_LocalPlayer) 
	{
		floatingTexts.empty();
		floatingTexts.clear();
		return;
	}
	if (!floatingTexts.size())
		return;
	for (int i = 0; i < floatingTexts.size(); i++)
	{
		int w, h = 0;
		g_EngineClient->GetScreenSize(w, h);
		w = w / 2;
		h = h / 2;
		DamageIndicator_t* txt = &floatingTexts.at(i);

		if (!txt->valid)
			continue;

		float endTime = txt->startTime + 2.0f;

		if (endTime < g_GlobalVars->curtime)
		{
			txt->valid = false;
			continue;
		}

		Vector origin_screen;
		Vector MarkerOrigin;
		if (!Math::WorldToScreen(txt->hitPosition, MarkerOrigin))
			return;

		origin_screen = MarkerOrigin;

		float t = 1.0f - (endTime - g_GlobalVars->curtime) / (endTime - txt->startTime);

		origin_screen.y -= t * (50.0f);
		origin_screen.x -= (float)txt->randomIdx * t * 3.0f;
		if (g_Configurations.damage_indicator)
		{
			char msg[12];
			sprintf_s(msg, 12, "%d", txt->damage);
			Color damage_color;
			if (txt->damage >= 66 && txt->damage <= 800)
				damage_color = Color(255, 0, 0, 255);
			else if (txt->damage >= 33 && txt->damage <= 65)
				damage_color = Color(255, 160, 0, 255);
			else if (txt->damage >= 0 && txt->damage <= 32)
				damage_color = Color(0, 255, 0, 255);

			Render::Get().RenderText(msg, ImVec2(origin_screen.x, origin_screen.y), 22.f, damage_color, true);
		}
		if (g_Configurations.hit_marker == 1)
		{
			Render::Get().RenderLine(MarkerOrigin.x - 8, MarkerOrigin.y - 8, MarkerOrigin.x - 2, MarkerOrigin.y - 2, Color(255, 255, 255), 1);
			Render::Get().RenderLine(MarkerOrigin.x + 8, MarkerOrigin.y - 8, MarkerOrigin.x + 2, MarkerOrigin.y - 2, Color(255, 255, 255), 1);
			Render::Get().RenderLine(MarkerOrigin.x - 8, MarkerOrigin.y + 8, MarkerOrigin.x - 2, MarkerOrigin.y + 2, Color(255, 255, 255), 1);
			Render::Get().RenderLine(MarkerOrigin.x + 8, MarkerOrigin.y + 8, MarkerOrigin.x + 2, MarkerOrigin.y + 2, Color(255, 255, 255), 1);
		}
		else if (g_Configurations.hit_marker == 2)
		{
			Render::Get().RenderLine(w - 10, h - 10, w - 3, h - 3, Color(255, 255, 255), 1);
			Render::Get().RenderLine(w + 10, h - 10, w + 3, h - 3, Color(255, 255, 255), 1);
			Render::Get().RenderLine(w - 10, h + 10, w - 3, h + 3, Color(255, 255, 255), 1);
			Render::Get().RenderLine(w + 10, h + 10, w + 3, h + 3, Color(255, 255, 255), 1);
		}
	}
}

