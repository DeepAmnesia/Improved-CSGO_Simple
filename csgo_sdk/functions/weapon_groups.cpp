#include "weapon_groups.hpp"
#include <optional>


CCustomWeaponGroups::CCustomWeaponGroups() {
	this->FillDefaults();
}

void CCustomWeaponGroups::FillDefaults() {
	setts.clear();
	setts["Pistols"] = {
		{32, 4, 63, 3, 1, 64, 2, 30, 61, 36},
		legitbot_settings(),
		false
	};
	setts["SubMachineGuns"] = {
		{17, 34, 33, 24, 26, 19, 23},
		legitbot_settings(),
		false
	};
	setts["Rifles"] = {
		{13, 10, 16, 60, 39, 8, 7},
		legitbot_settings(),
		false
	};
	setts["Shotguns"] = {
		{35, 25, 29, 27, 14, 28},
		legitbot_settings(),
		false
	};
	setts["Snipers"] = {
		{40, 9, 11, 38},
		legitbot_settings(),
		false
	};
}

void CCustomWeaponGroups::RenderSelector(std::string& selected, float customSpasing, std::function<void(std::string)> callback) {
	if (ImGui::ListBoxHeader("##wgroup_groupselector", ImVec2(-1, ImGui::GetWindowSize().y - customSpasing))) {
		for (auto& wgroup : setts) {
			if (ImGui::Selectable(wgroup.first.c_str(), selected == wgroup.first))
				selected = wgroup.first;
			callback(wgroup.first);
		}

		ImGui::ListBoxFooter();
	}
}

void CCustomWeaponGroups::Menu(std::string& selected_group) { // TODO: Add breaks to delete functions
	ImGui::Columns(3, nullptr, false);

	//// DNDROP
	static DNDropTarget_ dndrop_target = DNDropTarget_New;
	static std::string dndrop_extra_payload = "";
	static bool dndrop_released = false;
	static short dndrop_payload;

	auto create_dndrop_target = [&](DNDropTarget_ target, std::string extra = "") -> void {
		if (!dndrop_released) {
			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payloadD = ImGui::AcceptDragDropPayload("DeliveryWGroupData")) {
					dndrop_released = true;
					dndrop_payload = *(const short*)payloadD->Data;
					dndrop_target = target;
					dndrop_extra_payload = extra;
				}
				ImGui::EndDragDropTarget();
			}
		}
	};
	auto create_dndrop = [&](std::string n, short w_idx, DNDropTarget_ trgt, std::optional<bool> selected, std::optional<std::function<void()>> callback) -> void {
		if (ImGui::Selectable(n.c_str(), selected.value_or(false)) && callback.has_value())
			callback.value()();

		if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover | ImGuiDragDropFlags_SourceNoHoldToOpenOthers | ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
			ImGui::SetDragDropPayload("DeliveryWGroupData", &w_idx, sizeof(short));
			ImGui::EndDragDropSource();
		}

		create_dndrop_target(trgt);
	};
	//// DNDROP


	// OTHER UTILS
	auto delete_group = [&](std::string name) -> void {
		setts.erase(name);
	};
	//


	static char ngroup_input[32];

	ImGui::Text("Select Weapon Group:");
	RenderSelector(selected_group, 93.f, [&](std::string nd) -> void {
		create_dndrop_target(DNDropTarget_MoveToAnotherGroup, nd);
		});
	create_dndrop_target(DNDropTarget_CreateGroup);

	weapon_setts_t* current_wgroup = new weapon_setts_t();
	if (setts.count(selected_group) <= 0)
		selected_group = "";
	else
		current_wgroup = &setts[selected_group];

	ImGui::InputText("##wsgruopcreatormenu", ngroup_input, 32);
	ImGui::SameLine();
	if (ImGui::Button("Create", ImVec2(-1, 19))) {
		std::string d = std::string(ngroup_input);
		setts[d] = weapon_setts_t();
	}
	if (ImGui::Button("Delete", ImVec2(-1, 19)))
		delete_group(selected_group);
	ImGui::PopStyleColor();

	ImGui::NextColumn();

	ImGui::Text("Weapons in wgroup:");
	if (ImGui::ListBoxHeader("##wgroup_currweapons", ImVec2(-1, -1))) {

		ImGui::Selectable("You can drag here weapons", false);
		create_dndrop_target(DNDropTarget_New);

		for (auto& w_idx : current_wgroup->def_indexes)
			create_dndrop(values::WNames[w_idx], w_idx, DNDropTarget_New, {}, {});
		ImGui::ListBoxFooter();
	}
	create_dndrop_target(DNDropTarget_New);
	ImGui::NextColumn();

	auto isWeaponFree = [&](short idx) -> bool {
		for (auto& collection : setts)
			for (auto& weapon : collection.second.def_indexes)
				if (weapon == idx)
					return false;
		return true;
	};

	ImGui::Text("Selector:");
	if (ImGui::ListBoxHeader("##wselector", ImVec2(-1, -1))) {
		ImGui::Selectable("From here", false);
		create_dndrop_target(DNDropTarget_Delete);

		for (auto& [weapon_index, weapon_name] : values::WNames)
			if (isWeaponFree(weapon_index))
				create_dndrop(weapon_name, weapon_index, DNDropTarget_Delete, {}, {});

		ImGui::ListBoxFooter();
	}
	create_dndrop_target(DNDropTarget_Delete);

	if (dndrop_released) {

		if (dndrop_target == DNDropTarget_New && current_wgroup->lock) {}

		if (dndrop_target == DNDropTarget_New && !current_wgroup->lock) {
			current_wgroup->def_indexes.push_back(dndrop_payload);
		}
		else if (dndrop_target == DNDropTarget_Delete) {
			int iter = 0;
			for (auto& data : current_wgroup->def_indexes) {
				if (data == dndrop_payload) {
					current_wgroup->def_indexes.erase(current_wgroup->def_indexes.begin() + iter);
					if (current_wgroup->def_indexes.size() <= 0)
						delete_group(selected_group);
					break;
				}
				iter++;
			}
		}
		else if (dndrop_target == DNDropTarget_CreateGroup) {
			for (auto& collection : setts) {
				int iter = 0;
				for (auto& weapon : collection.second.def_indexes) {
					if (weapon == dndrop_payload)
						collection.second.def_indexes.erase(collection.second.def_indexes.begin() + iter);
					iter++;
				}
			}
			auto t = weapon_setts_t();
			t.def_indexes.push_back(dndrop_payload);
			t.lock = true;
			setts[values::WNames[dndrop_payload]] = t;
		}
		else if (dndrop_target == DNDropTarget_MoveToAnotherGroup) {
			int iter = 0;
			for (auto& def_index : current_wgroup->def_indexes) {
				if (def_index == dndrop_payload) {
					current_wgroup->def_indexes.erase(current_wgroup->def_indexes.begin() + iter);
					if (current_wgroup->def_indexes.size() <= 0)
						delete_group(selected_group);
					break;
				}
				iter++;
			}
			for (auto& [wgroup_name, wgroup] : setts) {
				if (wgroup_name == dndrop_extra_payload) {
					wgroup.def_indexes.push_back(dndrop_payload);
					break;
				}
			}
		}

		dndrop_released = false;
	}
}

template<typename T>
T parseData(nlohmann::json& d, const char* c, T def_value) {
	if (d.find(c) != d.end())
		return d[c].get<T>();
	return def_value;
};

void CCustomWeaponGroups::LoadSettings(nlohmann::json& data) {
	setts.clear();

	if (data.find("aimbot") != data.end()) {
		for (nlohmann::json element : data["aimbot"]) {
			weapon_setts_t plocsetts;

			plocsetts.def_indexes = element["weapons"].get<std::vector<short>>();
			plocsetts.lock = element["lock"].get<bool>();

			nlohmann::json gSettings = element["settings"];

			plocsetts.settings.autopistol = parseData(gSettings, "autopistol", plocsetts.settings.autopistol);
			plocsetts.settings.smoke_check = parseData(gSettings, "smoke_check", plocsetts.settings.smoke_check);
			plocsetts.settings.flash_check = parseData(gSettings, "flash_check", plocsetts.settings.flash_check);
			plocsetts.settings.jump_check = parseData(gSettings, "jump_check", plocsetts.settings.jump_check);
			plocsetts.settings.autowall = parseData(gSettings, "autowall", plocsetts.settings.autowall);
			plocsetts.settings.autowall_int = parseData(gSettings, "autowall_int", plocsetts.settings.autowall_int);
			plocsetts.settings.silent = parseData(gSettings, "silent", plocsetts.settings.silent);
			plocsetts.settings.rcs = parseData(gSettings, "rcs", plocsetts.settings.rcs);
			plocsetts.settings.rcs_fov_enabled = parseData(gSettings, "rcs_fov_enabled", plocsetts.settings.rcs_fov_enabled);
			plocsetts.settings.rcs_smooth_enabled = parseData(gSettings, "rcs_smooth_enabled", plocsetts.settings.rcs_smooth_enabled);
			plocsetts.settings.autostop = parseData(gSettings, "autostop", plocsetts.settings.autostop);
			plocsetts.settings.only_in_zoom = parseData(gSettings, "only_in_zoom", plocsetts.settings.only_in_zoom);
			plocsetts.settings.aim_type = parseData(gSettings, "aim_type", plocsetts.settings.aim_type);
			plocsetts.settings.priority = parseData(gSettings, "priority", plocsetts.settings.priority);
			plocsetts.settings.fov_type = parseData(gSettings, "fov_type", plocsetts.settings.fov_type);
			plocsetts.settings.smooth_type = parseData(gSettings, "smooth_type", plocsetts.settings.smooth_type);
			plocsetts.settings.curve = parseData(gSettings, "curve", plocsetts.settings.curve);
			plocsetts.settings.humanize = parseData(gSettings, "humanize", plocsetts.settings.humanize);
			plocsetts.settings.rcs_type = parseData(gSettings, "rcs_type", plocsetts.settings.rcs_type);
			plocsetts.settings.hitbox = parseData(gSettings, "hitbox", plocsetts.settings.hitbox);
			plocsetts.settings.fov = parseData(gSettings, "fov", plocsetts.settings.fov);
			plocsetts.settings.trigger_enable = parseData(gSettings, "trigger_enable", plocsetts.settings.trigger_enable);
			plocsetts.settings.trigger_check_smoke = parseData(gSettings, "trigger_check_smoke", plocsetts.settings.trigger_check_smoke);
			plocsetts.settings.trigger_check_flash = parseData(gSettings, "trigger_check_flash", plocsetts.settings.trigger_check_flash);
			plocsetts.settings.trigger_hit_chance = parseData(gSettings, "trigger_hit_chance", plocsetts.settings.trigger_hit_chance);
			plocsetts.settings.trigger_delay = parseData(gSettings, "trigger_delay", plocsetts.settings.trigger_delay);
			plocsetts.settings.trigger_hitgroup_head = parseData(gSettings, "trigger_hitgroup_head", plocsetts.settings.trigger_hitgroup_head);
			plocsetts.settings.trigger_hitgroup_chest = parseData(gSettings, "trigger_hitgroup_chest", plocsetts.settings.trigger_hitgroup_chest);
			plocsetts.settings.trigger_hitgroup_stomach = parseData(gSettings, "trigger_hitgroup_stomach", plocsetts.settings.trigger_hitgroup_stomach);
			plocsetts.settings.trigger_hitgroup_left_arm = parseData(gSettings, "trigger_hitgroup_left_arm", plocsetts.settings.trigger_hitgroup_left_arm);
			plocsetts.settings.trigger_hitgroup_right_arm = parseData(gSettings, "trigger_hitgroup_right_arm", plocsetts.settings.trigger_hitgroup_right_arm);
			plocsetts.settings.trigger_hitgroup_left_leg = parseData(gSettings, "trigger_hitgroup_left_leg", plocsetts.settings.trigger_hitgroup_left_leg);
			plocsetts.settings.trigger_hitgroup_right_leg = parseData(gSettings, "trigger_hitgroup_right_leg", plocsetts.settings.trigger_hitgroup_right_leg);
			plocsetts.settings.hitchance = parseData(gSettings, "hitchance", plocsetts.settings.hitchance);
			plocsetts.settings.hitchance_amount = parseData(gSettings, "hitchance_amount", plocsetts.settings.hitchance_amount);
			plocsetts.settings.silent_fov = parseData(gSettings, "silent_fov", plocsetts.settings.silent_fov);
			plocsetts.settings.rcs_fov = parseData(gSettings, "rcs_fov", plocsetts.settings.rcs_fov);
			plocsetts.settings.smooth = parseData(gSettings, "smooth", plocsetts.settings.smooth);
			plocsetts.settings.rcs_smooth = parseData(gSettings, "rcs_smooth", plocsetts.settings.rcs_smooth);
			plocsetts.settings.shot_delay = parseData(gSettings, "shot_delay", plocsetts.settings.shot_delay);
			plocsetts.settings.kill_delay = parseData(gSettings, "kill_delay", plocsetts.settings.kill_delay);
			plocsetts.settings.rcs_x = parseData(gSettings, "rcs_x", plocsetts.settings.rcs_x);
			plocsetts.settings.rcs_y = parseData(gSettings, "rcs_y", plocsetts.settings.rcs_y);
			plocsetts.settings.rcs_start = parseData(gSettings, "rcs_start", plocsetts.settings.rcs_start);
			plocsetts.settings.min_damage = parseData(gSettings, "min_damage", plocsetts.settings.min_damage);
			plocsetts.settings.psilent = parseData(gSettings, "psilent", plocsetts.settings.psilent);
			plocsetts.settings.enable_backtrack = parseData(gSettings, "enable_backtrack", plocsetts.settings.enable_backtrack);
			plocsetts.settings.backtrack_ticks = parseData(gSettings, "backtrack_ticks", plocsetts.settings.backtrack_ticks);

			setts[element["name"].get<std::string>()] = plocsetts;
		}
	}
}

void CCustomWeaponGroups::SaveSettings(nlohmann::json& data) {
	nlohmann::json retData;
	for (auto& [key, val] : setts) {
		auto& option = val.settings;
		nlohmann::json locElem;

		locElem["name"] = key;
		locElem["weapons"] = val.def_indexes;
		locElem["lock"] = val.lock;

		locElem["settings"]["autopistol"] = option.autopistol;
		locElem["settings"]["smoke_check"] = option.smoke_check;
		locElem["settings"]["flash_check"] = option.flash_check;
		locElem["settings"]["jump_check"] = option.jump_check;
		locElem["settings"]["autowall"] = option.autowall;
		locElem["settings"]["autowall_int"] = option.autowall_int;
		locElem["settings"]["silent"] = option.silent;
		locElem["settings"]["rcs"] = option.rcs;
		locElem["settings"]["rcs_fov_enabled"] = option.rcs_fov_enabled;
		locElem["settings"]["rcs_smooth_enabled"] = option.rcs_smooth_enabled;
		locElem["settings"]["autostop"] = option.autostop;
		locElem["settings"]["only_in_zoom"] = option.only_in_zoom;
		locElem["settings"]["aim_type"] = option.aim_type;
		locElem["settings"]["priority"] = option.priority;
		locElem["settings"]["fov_type"] = option.fov_type;
		locElem["settings"]["smooth_type"] = option.smooth_type;
		locElem["settings"]["curve"] = option.curve;
		locElem["settings"]["humanize"] = option.humanize;
		locElem["settings"]["rcs_type"] = option.rcs_type;
		locElem["settings"]["hitbox"] = option.hitbox;
		locElem["settings"]["fov"] = option.fov;
		locElem["settings"]["trigger_enable"] = option.trigger_enable;
		locElem["settings"]["trigger_check_smoke"] = option.trigger_check_smoke;
		locElem["settings"]["trigger_check_flash"] = option.trigger_check_flash;
		locElem["settings"]["trigger_hit_chance"] = option.trigger_hit_chance;
		locElem["settings"]["trigger_delay"] = option.trigger_delay;
		locElem["settings"]["trigger_hitgroup_head"] = option.trigger_hitgroup_head;
		locElem["settings"]["trigger_hitgroup_chest"] = option.trigger_hitgroup_chest;
		locElem["settings"]["trigger_hitgroup_stomach"] = option.trigger_hitgroup_stomach;
		locElem["settings"]["trigger_hitgroup_left_arm"] = option.trigger_hitgroup_left_arm;
		locElem["settings"]["trigger_hitgroup_right_arm"] = option.trigger_hitgroup_right_arm;
		locElem["settings"]["trigger_hitgroup_left_leg"] = option.trigger_hitgroup_left_leg;
		locElem["settings"]["trigger_hitgroup_right_leg"] = option.trigger_hitgroup_right_leg;
		locElem["settings"]["hitchance"] = option.hitchance;
		locElem["settings"]["hitchance_amount"] = option.hitchance_amount;
		locElem["settings"]["silent_fov"] = option.silent_fov;
		locElem["settings"]["rcs_fov"] = option.rcs_fov;
		locElem["settings"]["smooth"] = option.smooth;
		locElem["settings"]["rcs_smooth"] = option.rcs_smooth;
		locElem["settings"]["shot_delay"] = option.shot_delay;
		locElem["settings"]["kill_delay"] = option.kill_delay;
		locElem["settings"]["rcs_x"] = option.rcs_x;
		locElem["settings"]["rcs_y"] = option.rcs_y;
		locElem["settings"]["rcs_start"] = option.rcs_start;
		locElem["settings"]["min_damage"] = option.min_damage;
		locElem["settings"]["psilent"] = option.psilent;
		locElem["settings"]["enable_backtrack"] = option.enable_backtrack;
		locElem["settings"]["backtrack_ticks"] = option.backtrack_ticks;

		retData.push_back(locElem);
	}
	data["aimbot"] = retData;
}

legitbot_settings* CCustomWeaponGroups::GetSettings(short idx) {
	for (auto& wgroup : setts) {
		auto idx_vec = wgroup.second.def_indexes;
		if (std::find(idx_vec.begin(), idx_vec.end(), idx) != idx_vec.end())
			return &wgroup.second.settings;
	}
	return new legitbot_settings();
}

legitbot_settings* CCustomWeaponGroups::GetSettings(std::string name) {
	if (setts.count(name) != 0)
		return &setts[name].settings;
	return new legitbot_settings();
}

weapon_setts_t CCustomWeaponGroups::GetGroupSettings(short idx) {
	for (auto& wgroup : setts) {
		auto idx_vec = wgroup.second.def_indexes;
		if (std::find(idx_vec.begin(), idx_vec.end(), idx) != idx_vec.end())
			return wgroup.second;
	}
	return weapon_setts_t();
}

weapon_setts_t CCustomWeaponGroups::GetGroupSettings(std::string name) {
	if (setts.count(name) != 0)
		return setts[name];
	return weapon_setts_t();
}

std::string CCustomWeaponGroups::GetGroupName(short idx) {
	for (auto& wgroup : setts) {
		auto idx_vec = wgroup.second.def_indexes;
		if (std::find(idx_vec.begin(), idx_vec.end(), idx) != idx_vec.end())
			return wgroup.first;
	}
	return "";
}


CCustomWeaponGroups* g_CustomWeaponGroups = new CCustomWeaponGroups();