#pragma once
#include "configurations.hpp"
#include "../csgo_sdk/sdk/utils/singleton.hpp"
#include "../csgo_sdk/helpers/nlohmann/json.hpp"
#include "helpers/keybinds.hpp"
#include <filesystem>
#include <vector>
#include <string>

template <typename T>
struct item_t {
	T* pointer;
	std::string category;
	std::string name;
	T default_value;
};


class Config {
private:
	std::vector<item_t<bool>> booleans;
	std::vector<item_t<int>> ints;
	std::vector<item_t<float>> floats;
	std::vector<item_t<Color>> colors;
	std::vector<item_t<CKeyBind>> keybinds;
public:
	void PushItem(bool* pointer, std::string category, std::string name, bool def_value);
	void PushItem(int* pointer, std::string category, std::string name, int def_value);
	void PushItem(float* pointer, std::string category, std::string name, float def_value);
	void PushItem(Color* pointer, std::string category, std::string name, Color def_value);
	void PushItem(CKeyBind* pointer, std::string category, std::string name, CKeyBind def_value);
public:
	void SetupValues();
public:
	std::vector<std::string> config_files;
	void read(std::string path);
	void save(std::string path);
	void remove(std::string path);
	void refresh();
	void reset();
public:
	void Menu();
public:
	void ApplyRainbow();
};

extern Config* g_Config;