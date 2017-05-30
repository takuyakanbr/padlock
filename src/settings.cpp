#include "stdafx.h"

#include <ShlObj.h>
#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include "settings.hpp"

#define APP_FOLDER_NAME "\\Padlock"
#define APP_CONFIG_FILE "\\conf.ini"

namespace {
	std::map<std::string, std::string> iniData;

	// intentionally naive conversion, returns 0 if no conversion can be made
	int nstoi(const char *p) {
		int x = 0;
		while (*p >= '0' && *p <= '9') {
			x = (x * 10) + (*p - '0');
			++p;
		}
		return x;
	}

	void loadSeq(const char *name, input::KeyData *seq) {
		if (iniData.find(name) == iniData.end()) return;

		_Dc("Loading seq: " << name << std::endl);
		std::stringstream in(iniData[name]);
		for (int i = 0; i < state::Options::MAX_SEQ_LEN; i++) {
			if (!in.good()) {
				seq[i].code = 0;
				continue;
			}
			seq[i].ctrl = in.get() == '1';
			seq[i].shift = in.get() == '1';
			seq[i].alt = in.get() == '1';
			in >> seq[i].code;
			in.ignore(1);
			_Dc((int)seq[i].ctrl << (int)seq[i].shift << (int)seq[i].alt << 
				seq[i].code << std::endl);
		}
	}

	void saveSeq(const char *name, const input::KeyData *seq) {
		std::stringstream out;

		_Dc("Saving seq: " << name << std::endl);
		for (int i = 0; i < state::Options::MAX_SEQ_LEN; i++) {
			out << (int)seq[i].ctrl << (int)seq[i].shift << (int)seq[i].alt;
			out << seq[i].code << ",";
			_Dc((int)seq[i].ctrl << (int)seq[i].shift << (int)seq[i].alt << 
				seq[i].code << std::endl);
		}
		iniData[name] = out.str();
	}

	// load config data from our config file into iniData
	bool loadData() {
		if (iniData.size() > 0) return true;

		CHAR path[MAX_PATH];
		SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);
		if (std::strlen(path) > 200) return false;
		strcat(path, APP_FOLDER_NAME);
		CreateDirectoryA(path, NULL);
		strcat(path, APP_CONFIG_FILE);

		std::ifstream in(path);
		if (!in.good()) return false;

		while (!in.eof()) {
			std::string key;
			std::string value;
			std::getline(in, key, '=');
			std::getline(in, value);
			if (key.size() > 0 && value.size() > 0)
				iniData[key] = value;
		}
		in.close();

		return true;
	}

	// save config data in iniData to our config file
	bool saveData() {
		CHAR path[MAX_PATH];
		SHGetFolderPathA(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, path);
		if (std::strlen(path) > 200) return false;
		strcat(path, APP_FOLDER_NAME);
		CreateDirectoryA(path, NULL);
		strcat(path, APP_CONFIG_FILE);

		std::ofstream out(path, std::fstream::trunc);
		for (auto& e : iniData) {
			out << e.first << "=" << e.second << std::endl;
		}
		out.close();

		return true;
	}
}

namespace settings {

	bool loadOptions(state::Options& opts) {
		if (!loadData()) return false;

		loadSeq("useq", opts.unlockSeq);
		loadSeq("rseq", opts.limitSeq);
		loadSeq("lseq", opts.lockSeq);
		opts.autoLock = nstoi(iniData["alock"].c_str());
		opts.statusMode = nstoi(iniData["smode"].c_str());
		if (opts.statusMode > STATE_STATUS_MAXVALUE)
			opts.statusMode = STATE_STATUS_MAXVALUE;

		return true;
	}

	bool saveOptions(const state::Options& opts) {
		saveSeq("useq", opts.unlockSeq);
		saveSeq("rseq", opts.limitSeq);
		saveSeq("lseq", opts.lockSeq);
		iniData["alock"] = std::to_string(opts.autoLock);
		iniData["smode"] = std::to_string(opts.statusMode);
		return saveData();
	}
}
