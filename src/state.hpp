#pragma once

#include <string>
#include "wininput/wininput.hpp"

#define STATE_KEYSEQ_NONE 0
#define STATE_KEYSEQ_UNLOCKED 1
#define STATE_KEYSEQ_LIMITED 2
#define STATE_KEYSEQ_LOCKED 3
#define STATE_STATUS_SHOWALWAYS 0
#define STATE_STATUS_HIDEWHENUNLOCKED 1
#define STATE_STATUS_HIDEALWAYS 2
#define STATE_STATUS_MAXVALUE 2

namespace state {

	enum class InputState { UNLOCKED, LIMITED, LOCKED };
	enum class EditState { NONE, UNLOCKSEQ, LIMITSEQ, LOCKSEQ };

	class Options {
	public:
		static const int MAX_SEQ_LEN = 11;
		input::KeyData unlockSeq[MAX_SEQ_LEN];
		input::KeyData limitSeq[MAX_SEQ_LEN];
		input::KeyData lockSeq[MAX_SEQ_LEN];
		int autoLock = 0; // In minutes, where 0 = disabled.
		int statusMode = STATE_STATUS_SHOWALWAYS;

		Options() {}
		Options(const Options&) = delete;
		Options& operator=(const Options&) = delete;
	};

	// Used by main.cpp; sets up input handling, and loads user settings.
	void setup();

	// Get the time, in minutes, of inactivity before automatically switching
	// to Locked mode. If this value is 0, autolock is disabled.
	std::string getAutoLock();

	// Get the current setting regarding when the status box should be shown.
	int getStatusMode();

	// Sets the autolock value and returns the updated value.
	std::string setAutoLock(std::string val);

	// Sets the status box setting and returns the updated value.
	int setStatusMode(int mode);

	// Returns true if in Unlocked mode (all inputs allowed).
	bool isUnlocked();

	// Get the current mode as an InputState enum.
	InputState getInputState();

	// Specify the sequence to be updated in following calls to updateSequence
	// and resets the internal index that tracks which KeyData in the sequence
	// to update next.
	// Type should be one of STATE_KEYSEQ_[X].
	void notifyInputUpdate(int type);

	// Return a string representation of the sequence specified by type.
	// Type should be one of STATE_KEYSEQ_[X].
	std::string getSequence(int type);

	// Update the next KeyData in the sequence specified by type.
	// Only works if the previous call to notifyInputUpdate specified the same type.
	// Type should be one of STATE_KEYSEQ_[X].
	std::string updateSequence(int type, unsigned vkCode);
}
