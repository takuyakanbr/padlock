
#include "keymap.hpp"

namespace {
	std::map<unsigned, std::string> map;
	std::map<unsigned, std::string> shifted;
}

namespace input {

	void setupCodemap() {
		map[0x10] = "Shift";
		map[0x11] = "Ctrl";
		map[0x12] = "Alt";
		map[0xA0] = "Shift";
		map[0xA1] = "Shift";
		map[0xA2] = "Ctrl";
		map[0xA3] = "Ctrl";
		map[0xA4] = "Alt";
		map[0xA5] = "Alt";
		map[0x14] = "CAPS";
		map[0x90] = "NUM";
		map[0x91] = "SCROLL";

		map[0x08] = "Back";
		map[0x09] = "Tab";
		map[0x0C] = "Clear";
		map[0x0D] = "Enter";
		map[0x13] = "Pause";
		map[0x1B] = "Esc";
		map[0x20] = "Space";
		map[0x21] = "PgeUp";
		map[0x22] = "PgeDwn";
		map[0x23] = "End";
		map[0x24] = "Home";
		map[0x25] = "Left";
		map[0x26] = "Up";
		map[0x27] = "Right";
		map[0x28] = "Down";
		map[0x29] = "Select";
		map[0x2A] = "Print";
		map[0x2D] = "Ins";
		map[0x2E] = "Del";
		map[0x2F] = "Help";
		map[0x5B] = "LWin";
		map[0x5C] = "RWin";
		map[0x5D] = "Apps";
		map[0x5F] = "Sleep";
		map[0x60] = "00";
		map[0x61] = "01";
		map[0x62] = "02";
		map[0x63] = "03";
		map[0x64] = "04";
		map[0x65] = "05";
		map[0x66] = "06";
		map[0x67] = "07";
		map[0x68] = "08";
		map[0x69] = "09";
		map[0x6A] = "*";
		map[0x6B] = "+";
		map[0x6C] = "Sep";
		map[0x6D] = "-";
		map[0x6E] = ".";
		map[0x6F] = "/";
		map[0x70] = "F1";
		map[0x71] = "F2";
		map[0x72] = "F3";
		map[0x73] = "F4";
		map[0x74] = "F5";
		map[0x75] = "F6";
		map[0x76] = "F7";
		map[0x77] = "F8";
		map[0x78] = "F9";
		map[0x79] = "F10";
		map[0x7A] = "F11";
		map[0x7B] = "F12";
		map[0x7C] = "F13";
		map[0x7D] = "F14";
		map[0x7E] = "F15";
		map[0x7F] = "F16";
		map[0x80] = "F17";
		map[0x81] = "F18";
		map[0x82] = "F19";
		map[0x83] = "F20";
		map[0x84] = "F21";
		map[0x85] = "F22";
		map[0x86] = "F23";
		map[0x87] = "F24";
		map[0xDF] = "OEM8";

		map[0xBA] = ";";
		map[0xBB] = "=";
		map[0xBC] = ",";
		map[0xBD] = "-";
		map[0xBE] = ".";
		map[0xBF] = "/";
		map[0xC0] = "`";
		map[0xDB] = "LB";
		map[0xDC] = "\\";
		map[0xDD] = "RB";
		map[0xDE] = "'";

		shifted[0xBA] = ":";
		shifted[0xBB] = "+";
		shifted[0xBC] = "<";
		shifted[0xBD] = "_";
		shifted[0xBE] = ">";
		shifted[0xBF] = "?";
		shifted[0xC0] = "~";
		shifted[0xDB] = "{";
		shifted[0xDC] = "|";
		shifted[0xDD] = "}";
		shifted[0xDE] = "\"";

		// number keys
		shifted[0x30] = ")";
		shifted[0x31] = "!";
		shifted[0x32] = "@";
		shifted[0x33] = "#";
		shifted[0x34] = "$";
		shifted[0x35] = "%";
		shifted[0x36] = "^";
		shifted[0x37] = "&";
		shifted[0x38] = "*";
		shifted[0x39] = "(";
	}

	std::string keyToString(const KeyData& key) {
		bool shift = key.shift;

		std::string base;
		// A - Z
		if (key.code >= 0x41 && key.code <= 0x5A) {
			char arr[2] = "A";
			if (key.ctrl || shift || key.alt)
				arr[0] = (char)(key.code - 0x41) + 'A'; // command / capital
			else
				arr[0] = (char)(key.code - 0x41) + 'a'; // small
			if (shift && !key.ctrl && !key.alt)
				shift = false; // hide shift if it's the only modifier
			base = arr;

		// keys with special handling when shifted
		} else if (shift && !key.ctrl && !key.alt && shifted.find(key.code) != shifted.end()) {
			shift = false;
			base = shifted[key.code];

		// 0 - 9
		} else if (key.code >= 0x30 && key.code <= 0x39) {
			char arr[2] = "0";
			arr[0] = (char)(key.code - 0x30) + '0';
			base = arr;

		// other special keys
		} else if (map.find(key.code) != map.end())
			base = map[key.code];
		else
			base = "Unk";

		if (base.size() == 1 && !key.ctrl && !shift && !key.alt) {
			return base;
		} else {
			std::string res("[");
			if (key.ctrl) res += "Ctrl+";
			if (shift) res += "Shift+";
			if (key.alt) res += "Alt+";
			res += base;
			res += "]";
			return res;
		}
	}

}