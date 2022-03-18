#include "keybinds.hpp"
#include "../imgui/imgui.h"
#include "../configurations.hpp"


CKeyBinds* g_KeyBinds = new CKeyBinds();


void CKeyBinds::DeclareKeybind(CKeyBind* kbind) {
	this->KeyBinds.push_back(kbind);
}

std::vector<CKeyBind> CKeyBinds::ExportActiveKeybinds() {
	std::vector<CKeyBind> result;
	for (CKeyBind* kbind : this->KeyBinds) {
		if (kbind->state != KeyBindState_None)
			result.push_back(*kbind);
	}
	return result;
}

void CKeyBinds::ExecuteKeyBinds() {
	for (CKeyBind* kbind : this->KeyBinds) {
		if (kbind->mfuncpointer && !*kbind->mfuncpointer) {
			*kbind->gvar = false;
			kbind->state = KeyBindState_None;
			continue;
		}

		switch (kbind->mode) {
		case KeyBindMode_Always:
			*kbind->gvar = true;
			kbind->state = KeyBindState_Always;
			break;
		case KeyBindMode_HoldOff:
			if (InputSys::Get().IsKeyDown(kbind->key)) {
				*kbind->gvar = false;
				kbind->state = KeyBindState_HoldingOff;
			}
			else {
				*kbind->gvar = true;
				kbind->state = KeyBindState_None;
			}
			break;
		case KeyBindMode_HoldOn:
			if (InputSys::Get().IsKeyDown(kbind->key)) {
				*kbind->gvar = true;
				kbind->state = KeyBindState_HoldingOn;
			}
			else {
				*kbind->gvar = false;
				kbind->state = KeyBindState_None;
			}
			break;
		case KeyBindMode_Toggle:
			if (InputSys::Get().WasKeyPressed(kbind->key))
				*kbind->gvar = !*kbind->gvar;
			kbind->state = *kbind->gvar ? KeyBindState_Toggled : KeyBindState_None;
			break;
		default:
			kbind->state = KeyBindState_None;
			break;
		}
	}
}

void CKeyBinds::Draw() {
	if (!g_Configurations.draw_binds)
		return;

	ImGui::Begin("Key binds", nullptr, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
	{

		std::string names = "";
		std::string modes = "";

		for (auto kbind : g_KeyBinds->ExportActiveKeybinds()) {
			std::string mode = "";
			switch (kbind.state) {
			case KeyBindState_Always:
				mode = "Always";
				break;
			case KeyBindState_HoldingOff:
				mode = "Holding OFF";
				break;
			case KeyBindState_HoldingOn:
				mode = "Holding ON";
				break;
			case KeyBindState_Toggled:
				mode = "Toggled";
				break;
			};
			names += kbind.name + "\n";
			modes += mode + "\n";
		}
		ImVec2 pWsize = ImGui::CalcTextSize(names.c_str());
		ImGui::SetWindowSize(ImVec2(200, 39 + pWsize.y));
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 2.f);
	

		ImGui::BeginGroup();
		ImGui::Columns(2, nullptr, false);

		ImGui::Text(names.c_str());
		ImGui::NextColumn();
		ImGui::Text(modes.c_str());

		ImGui::EndGroup();



	}
	ImGui::End();
}