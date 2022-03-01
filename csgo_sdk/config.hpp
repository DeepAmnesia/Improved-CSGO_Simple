#pragma once

#include "configurations.hpp"
#include "sdk/utils/singleton.hpp"

#include <fstream>

class Config : public Singleton<Config> {
public:
	void Save() {
		std::ofstream fout("csgo_sdk.cfg", std::ios::binary);
		const auto sz = sizeof(Configurations);
		const auto var_sz = sizeof(Var<bool>);
		const auto cnt = sz / var_sz;
		for (auto i = 0; i < cnt; i++) {
			const auto el = &(*(Var<int>*)(&g_Configurations)) + i;
			auto name = el->name;
			auto val = el->value;
			auto sizeof_val = el->size;
			fout << name << "\t" << Utils::BytesToString((unsigned char*)*(int*)&val, sizeof_val) << std::endl;
		}
		fout.close();
	}


	void Load() {
		std::ifstream fin("csgo_sdk.cfg", std::ios::binary);
		std::stringstream ss;
		ss << fin.rdbuf();


		auto lines = Utils::Split(ss.str(), "\n");

		for (auto line : lines) {
			auto data = Utils::Split(line, "\t");
			const auto sz = sizeof(Configurations);
			const auto var_sz = sizeof(Var<bool>);
			const auto cnt = sz / var_sz;
			for (auto i = 0; i < cnt; i++) {
				const auto &el = &(*(Var<bool>*)(&g_Configurations)) + i;
				if (data[0] == el->name) {
					auto bytes = Utils::HexToBytes(data[1]);
					memcpy(*(void**)&el->value, bytes.data(), el->size);
				}
			}
		}
		fin.close();
	}
};