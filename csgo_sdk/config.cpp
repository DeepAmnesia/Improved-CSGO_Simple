#include "config.hpp"
#include "configurations.hpp"
#include "../csgo_sdk/helpers/keybinds.hpp"
#include "../csgo_sdk/render/render.hpp"
#include "../csgo_sdk/functions/aimbot.hpp"
#include "../csgo_sdk/functions/weapon_groups.hpp"
#include "../csgo_sdk/functions/visuals.hpp"
//#include "globals.hpp"

inline bool file_exists(const std::string& name) {
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

// SETUPS
void Config::PushItem(bool* pointer, std::string category, std::string name, bool def_value) {
	this->booleans.push_back({ pointer, category, name, def_value });
}
void Config::PushItem(int* pointer, std::string category, std::string name, int def_value) {
	this->ints.push_back({ pointer, category, name, def_value });
}
void Config::PushItem(float* pointer, std::string category, std::string name, float def_value) {
	this->floats.push_back({ pointer, category, name, def_value });
}
void Config::PushItem(Color* pointer, std::string category, std::string name, Color def_value) {
	this->colors.push_back({ pointer, category, name, def_value });
}
void Config::PushItem(CKeyBind* pointer, std::string category, std::string name, CKeyBind def_value) {
	this->keybinds.push_back({ pointer, category, name, def_value });
	g_KeyBinds->DeclareKeybind(pointer);
}
void Config::SetupValues() {
	Visuals::Get().SetupValues();
//	g_Visuals->SetupValues();
//	g_Misc->SetupValues();
	g_Aimbot.SetupValues();
//	g_SpecList->SetupValues();
//	g_SkinChanger->SetupValues();
}
// SETUPS

using nlohmann::json;
void Config::read(std::string path) {
	std::filesystem::create_directory("c:\\bnh_beta\\");
	path = "c:\\bnh_beta\\" + path + ".cfg";
	if (!file_exists(path)) return;

	json retData;
	std::ifstream reader(path);

	json data;
	reader >> data;

	reader.close();

	auto sanityChecks = [](json& d, std::string& c, std::string& n) -> bool {
		return d.find(c) != d.end() && d[c].find(n) != d[c].end();
	};

	for (auto& b : booleans)
		if (sanityChecks(data, b.category, b.name))
			*b.pointer = data[b.category][b.name].get<bool>();
	for (auto& i : ints)
		if (sanityChecks(data, i.category, i.name))
			*i.pointer = data[i.category][i.name].get<int>();
	for (auto& f : floats)
		if (sanityChecks(data, f.category, f.name))
			*f.pointer = data[f.category][f.name].get<float>();
	for (auto& c : colors) {
		if (sanityChecks(data, c.category, c.name)) {
			auto elem = data[c.category][c.name];
			c.pointer->SetColor(elem["r"].get<int>(), elem["g"].get<int>(), elem["b"].get<int>(), elem["a"].get<int>());
			
		}
	}
	for (auto& k : keybinds)
		if (sanityChecks(data, k.category, k.name))
			k.pointer->Deserialize(data[k.category][k.name]);

	g_CustomWeaponGroups->LoadSettings(data);

	
}

void Config::save(std::string path) {
	std::filesystem::create_directory("c:\\bnh_beta\\");
	path = "c:\\bnh_beta\\" + path + ".cfg";

	json retData;

	auto sanityChecks = [&](std::string category) -> void {
		if (retData.find(category) == retData.end())
			retData[category] = json();
	};

	for (auto& b : booleans) {
		sanityChecks(b.category);
		retData[b.category][b.name] = *b.pointer;
	}
	for (auto& i : ints) {
		sanityChecks(i.category);
		retData[i.category][i.name] = *i.pointer;
	}
	for (auto& f : floats) {
		sanityChecks(f.category);
		retData[f.category][f.name] = *f.pointer;
	}
	for (auto& c : colors) {
		sanityChecks(c.category);
		json d;
		Color clr = *c.pointer;
		d["r"] = clr.r();
		d["g"] = clr.g();
		d["b"] = clr.b();
		d["a"] = clr.a();
		retData[c.category][c.name] = d;
	}
	for (auto& k : keybinds) {
		sanityChecks(k.category);
		retData[k.category][k.name] = k.pointer->Serialize();
	}



	std::ofstream reader(path);
	reader.clear();
	reader << std::setw(4) << retData << std::endl;
	reader.close();
}

void Config::refresh() {
	config_files.clear();
	std::filesystem::create_directory("c:\\bnh_beta\\");

	for (const auto& p : std::filesystem::recursive_directory_iterator("c:\\bnh_beta\\")) {
		if (!std::filesystem::is_directory(p) &&
			p.path().extension().string() == ".cfg") {
			auto file = p.path().filename().string();
			config_files.push_back(file.substr(0, file.size() - 4));
		}
	}
}

void Config::remove(std::string path) {
	std::filesystem::create_directory("c:\\bnh_beta\\");
	path = "c:\\bnh_beta\\" + path + ".cfg";

	std::filesystem::remove(path);
	refresh();
}

void Config::Menu() {
	static std::string selected_cfg = "";
	static char cfg_name[32];

	ImGui::Columns(2, nullptr, false);

	ImGui::Text("Selector:");
	ImGui::BeginChild("##list", ImVec2(-1, -1), true); {
		if (ImGui::InputText("File", cfg_name, 32)) selected_cfg = std::string(cfg_name);

		ImGui::ListBoxHeader("##cfglist", ImVec2(-1, -1));

		for (auto cfg : config_files)
			if (ImGui::Selectable(cfg.c_str(), cfg == selected_cfg))
				selected_cfg = cfg;

		ImGui::ListBoxFooter();
	};
	ImGui::EndChild();
	ImGui::NextColumn();

	ImGui::Text("Controls:");
	ImGui::BeginChild("##second", ImVec2(-1, -1), true); {
		/*
		if (ImGui::Button("Load", ImVec2(-1, 19))) {
			if (selected_cfg.empty())
				g_Notification.Push("Error", "Please fill config name");
			else {
				g_ShowPrompt = true;
				g_PromptTitle = "Are you sure, you want to LOAD " + selected_cfg + " config?";
				g_PromptCallback = [&]() -> void {
					read(selected_cfg);
					g_Notification.Push("Success", ("Loaded config " + selected_cfg).c_str());
				};
			}
			refresh();
		}
		if (ImGui::Button("Save", ImVec2(-1, 19))) {
			if (selected_cfg.empty())
				g_Notification.Push("Error", "Please fill config name");
			else {
				g_ShowPrompt = true;
				g_PromptTitle = "Are you sure, you want to SAVE " + selected_cfg + " config?";
				g_PromptCallback = [&]() -> void {
					save(selected_cfg);
					g_Notification.Push("Success", ("Saved config as " + selected_cfg).c_str());
				};
			}
			refresh();
		}
		if (ImGui::Button("Delete", ImVec2(-1, 19))) {
			if (selected_cfg.empty())
				g_Notification.Push("Error", "Please fill config name");
			else {
				g_ShowPrompt = true;
				g_PromptTitle = "Are you sure, you want to DELETE " + selected_cfg + " config?";
				g_PromptCallback = [&]() -> void {
					remove(selected_cfg);
					g_Notification.Push("Success", ("Removed config " + selected_cfg).c_str());
				};
			}
			refresh();
		}

		ImGui::SetCursorPosY(ImGui::GetCurrentWindow()->Size.y - 58);
		if (ImGui::Button("Refresh list", ImVec2(-1, 19)))
			refresh();
		if (ImGui::Button("Reset settings", ImVec2(-1, 19))) {
			g_ShowPrompt = true;
			g_PromptTitle = "Are you sure, you want to RESET ALL SETTINGS?";
			g_PromptCallback = [&]() -> void {
				reset();
				g_Notification.Push("Success", "Successfully reset all settings to default");
			};
		}
		*/
	};
	ImGui::EndChild();
}

void Config::reset() {
	for (auto& b : booleans)
		*b.pointer = b.default_value;
	for (auto& i : ints)
		*i.pointer = i.default_value;
	for (auto& c : colors)
		*c.pointer = c.default_value;
	for (auto& f : floats)
		*f.pointer = f.default_value;
	for (auto& k : keybinds)
		k.pointer->Deserialize(k.default_value.Serialize()); // s - smart

	g_CustomWeaponGroups->FillDefaults();
}

void Config::ApplyRainbow() {
	
}

Config* g_Config = new Config();