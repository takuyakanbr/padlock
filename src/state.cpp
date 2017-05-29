#include "stdafx.h"

#include <atomic>
#include "state.hpp"
#include "ui.hpp"
#include "settings.hpp"
#include "wininput\keymap.hpp"


namespace {
	using namespace state;

	std::atomic<InputState> inputState(InputState::UNLOCKED);
	std::atomic<EditState> editState(EditState::NONE);
	Options opts;

	std::atomic<int> updating(0);
	int updateIndex = 0;

	bool keyHandler(input::KeyData& data) {
		switch (inputState.load()) {
		case InputState::UNLOCKED:
			return false;
		case InputState::LIMITED:
			if (!data.ctrl && !data.alt) {
				// allow shift, 0-9, A-Z, various navigation keys
				if (data.code == VK_LSHIFT || data.code == VK_RSHIFT) return false;
				if (data.code >= 0x30 && data.code <= 0x5A) return false;
				if (data.code >= VK_SPACE && data.code <= VK_DOWN) return false;
			}
		case InputState::LOCKED:
			// allow ctrl, shift, alt keyup
			if (data.type == INPUT_TYPE_KEYUP 
				&& data.code >= VK_LSHIFT && data.code <= VK_RMENU) return false;
		}
		return true;
	}

	// if Limited/Locked -> block all mouse input
	bool mouseHandler(input::MouseData& data) {
		return inputState.load() != InputState::UNLOCKED;
	}

	// if Limited/Locked -> set to Unlocked
	bool unlockSeqHandler() {
		_Dc("state: Unlock sequence" << std::endl);
		if (inputState.load() != InputState::UNLOCKED) {
			inputState.store(InputState::UNLOCKED);
			input::trackModifierState(false);
			ui::redrawStatusWindow();
			return true;
		}
		return false;
	}

	// if Unlocked -> set to Limited
	bool limitSeqHandler() {
		_Dc("state: Limit sequence" << std::endl);
		if (inputState.load() == InputState::UNLOCKED && updating.load() == 0) {
			inputState.store(InputState::LIMITED);
			input::trackModifierState(true);
			ui::redrawStatusWindow();
			return true;
		}
		return false;
	}

	// if Unlocked/Limited -> set to Locked
	bool lockSeqHandler() {
		_Dc("state: Lock sequence" << std::endl);
		if (inputState.load() != InputState::LOCKED && updating.load() == 0) {
			inputState.store(InputState::LOCKED);
			input::trackModifierState(true);
			ui::redrawStatusWindow();
			return true;
		}
		return false;
	}

	std::string getSequenceText(const input::KeyData *seq) {
		std::string str;
		for (int i = 0; i < Options::MAX_SEQ_LEN; i++) {
			if (seq[i].code == 0) break;
			str += input::keyToString(seq[i]);
		}
		return str;
	}

	void updateKeyData(input::KeyData *seq, unsigned vkCode) {
		if (updateIndex >= Options::MAX_SEQ_LEN - 1) return;

		SHORT ctrl = GetAsyncKeyState(VK_CONTROL) >> (sizeof(SHORT) - 1);
		SHORT shift = GetAsyncKeyState(VK_SHIFT) >> (sizeof(SHORT) - 1);
		SHORT alt = GetAsyncKeyState(VK_MENU) >> (sizeof(SHORT) - 1);
		seq[updateIndex] = { vkCode, ctrl != 0, shift != 0, alt != 0, 3 };

		// reset the whole sequence
		if (updateIndex == 0) {
			for (int i = 1; i < Options::MAX_SEQ_LEN; i++) {
				seq[i] = { 0, false, false, false, 3 };
			}
		}

		++updateIndex;
	}
}

namespace state {

	void setup() {
		// defaults
		opts.unlockSeq[0] = { 0x41, false, false, false, 3 }; // asdf
		opts.unlockSeq[1] = { 0x53, false, false, false, 3 };
		opts.unlockSeq[2] = { 0x44, false, false, false, 3 };
		opts.unlockSeq[3] = { 0x46, false, false, false, 3 };
		opts.limitSeq[0] = { 0x52, false, false, true, 3 }; // Alt+R
		opts.lockSeq[0] = { 0x4C, false, false, true, 3 }; // Alt+L

		// add input handlers
		input::addKeyHandler(keyHandler);
		input::addMouseHandler(mouseHandler);
		input::addKeySequence(opts.unlockSeq, true, unlockSeqHandler, nullptr);
		input::addKeySequence(opts.limitSeq, true, limitSeqHandler, nullptr);
		input::addKeySequence(opts.lockSeq, true, lockSeqHandler, nullptr);

		input::setupCodemap();
		settings::loadOptions(opts);
	}

	bool isUnlocked() {
		return inputState.load() == InputState::UNLOCKED;
	}

	InputState getInputState() {
		return inputState.load();
	}

	void notifyUpdate(int type) {
		updating.store(type);
		updateIndex = 0;

		if (type == 0)
			settings::saveOptions(opts);
	}

	std::string getSequence(int type) {
		switch (type) {
		case STATE_KEYSEQ_UNLOCKED:
			return getSequenceText(opts.unlockSeq);
		case STATE_KEYSEQ_LIMITED:
			return getSequenceText(opts.limitSeq);
		case STATE_KEYSEQ_LOCKED:
			return getSequenceText(opts.lockSeq);
		}
		return std::string();
	}

	std::string updateSequence(int type, unsigned vkCode) {
		if (type == updating && vkCode != VK_CONTROL 
			&& vkCode != VK_SHIFT && vkCode != VK_MENU) {
			switch (type) {
			case STATE_KEYSEQ_UNLOCKED:
				updateKeyData(opts.unlockSeq, vkCode);
				break;
			case STATE_KEYSEQ_LIMITED:
				updateKeyData(opts.limitSeq, vkCode);
				break;
			case STATE_KEYSEQ_LOCKED:
				updateKeyData(opts.lockSeq, vkCode);
				break;
			}
		}
		return getSequence(type);
	}
}
