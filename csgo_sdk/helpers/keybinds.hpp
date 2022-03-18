#pragma once
#include <vector>
#include <string>
#include "../sdk/utils/input.hpp"
#include "../helpers/nlohmann/json.hpp"


enum KeyBindMode_ {
	KeyBindMode_Toggle = 0,
	KeyBindMode_HoldOn,
	KeyBindMode_HoldOff,
	KeyBindMode_Always
};

enum KeyBindState_ {
	KeyBindState_HoldingOn = 0,
	KeyBindState_HoldingOff,
	KeyBindState_Toggled,
	KeyBindState_Always,
	KeyBindState_None
};


class CKeyBind {
public:
	int key;
	int mode;
	int state = KeyBindState_None;
	bool* gvar;
	bool* mfuncpointer;
	bool need_modes;
	std::string name;

	//        is_active;   pointer funci dlya 4ekov;        name;       nujni li modes dlya keybinda v ui
	CKeyBind(bool* gvar, bool* mfuncpointer, std::string name, bool need_modes = true) {
		this->gvar = gvar;
		this->mfuncpointer = mfuncpointer;
		this->name = name;
		this->need_modes = need_modes;
	}

	nlohmann::json Serialize() {
		return nlohmann::json{
			{"key", this->key},
			{"mode", this->mode}
		};
	}

	void Deserialize(nlohmann::json data) {
		this->key = data["key"].get<int>();
		this->mode = data["mode"].get<int>();
	}
};


class CKeyBinds {
private:
	std::vector<CKeyBind*> KeyBinds = {};
public:
	void Draw();
	void ExecuteKeyBinds();
	void DeclareKeybind(CKeyBind* kbind);
	std::vector<CKeyBind> ExportActiveKeybinds();
};


extern CKeyBinds* g_KeyBinds;