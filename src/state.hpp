#pragma once

#include <string>
#include "wininput/wininput.hpp"

#define STATE_KEYSEQ_NONE 0
#define STATE_KEYSEQ_UNLOCKED 1
#define STATE_KEYSEQ_LIMITED 2
#define STATE_KEYSEQ_LOCKED 3

namespace state {

	enum class InputState { UNLOCKED, LIMITED, LOCKED };
	enum class EditState { NONE, UNLOCKSEQ, LIMITSEQ, LOCKSEQ };

	class Options {
	public:
		static const int MAX_SEQ_LEN = 11;
		input::KeyData unlockSeq[MAX_SEQ_LEN];
		input::KeyData limitSeq[MAX_SEQ_LEN];
		input::KeyData lockSeq[MAX_SEQ_LEN];

		Options() {}
		Options(const Options&) = delete;
		Options& operator=(const Options&) = delete;
	};

	void setup();

	bool isUnlocked();
	InputState getInputState();

	void notifyUpdate(int type);
	std::string getSequence(int type);
	std::string updateSequence(int type, unsigned vkCode);
}
