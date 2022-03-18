#include "SpecList.hpp"
#include "../imgui/imgui.h"
#include "../imgui/imgui_internal.h"
#include "../render/render.hpp"

SpecList* g_SpecList = new SpecList();

struct spec_t {
	std::string name;
	std::string mode;
};

void SpecList::Draw() {
	if (!g_Configurations.spectator_list) return;

	std::string specs = "";
	std::string modes = "";

	if (g_EngineClient->IsInGame() && g_EngineClient->IsConnected()) {
		C_BasePlayer* pLocalEntity = g_LocalPlayer;

		if (g_LocalPlayer && !g_LocalPlayer->IsAlive())
			pLocalEntity = g_LocalPlayer->m_hObserverTarget();

		if (pLocalEntity) {
			for (int i = 0; i < g_EngineClient->GetMaxClients(); i++) {
				C_BasePlayer* pBaseEntity = C_BasePlayer::GetPlayerByIndex(i);
				if (!pBaseEntity)										     continue;
				if (pBaseEntity->m_iHealth() > 0)							 continue;
				if (pBaseEntity == pLocalEntity)							 continue;
				if (pBaseEntity->IsDormant())								 continue;
				if (pBaseEntity->m_hObserverTarget() != pLocalEntity)		 continue;
				player_info_t pInfo;
				g_EngineClient->GetPlayerInfo(pBaseEntity->EntIndex(), &pInfo);
				if (pInfo.ishltv) continue;

				std::string mode = "unknown";
				switch (pBaseEntity->m_iObserverMode())
				{
				case OBS_MODE_CHASE:
					mode = ("3rd Person");
					break;
				case OBS_MODE_DEATHCAM:
					mode = ("DeathCams");
					break;
				case OBS_MODE_FIXED:
					mode = ("Fixed");
					break;
				case OBS_MODE_FREEZECAM:
					mode = ("FreezeCam");
					break;
				case OBS_MODE_IN_EYE:
					mode = ("1st Person");
					break;
				case OBS_MODE_NONE:
					mode = ("None");
					break;
				case OBS_MODE_ROAMING:
					mode = ("Roaming");
					break;
				}

				specs += pInfo.szName;
				specs += "\n";

				modes += mode;
				modes += "\n";
			}
		}
	}
	if (specs.size() > 0) specs += "\n";

	if (ImGui::Begin("Spectator List", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize)) {

		ImVec2 pWsize = ImGui::CalcTextSize(specs.c_str());
		ImGui::SetWindowSize(ImVec2(200, 35 + pWsize.y));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.f);

		ImGui::BeginGroup();
		ImGui::Columns(2, nullptr, false);

		ImGui::Text(specs.c_str());
		ImGui::NextColumn();
		ImGui::Text(modes.c_str());

		ImGui::EndGroup();
	}
	ImGui::End();
}
