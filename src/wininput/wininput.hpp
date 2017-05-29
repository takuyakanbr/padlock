#pragma once

// The value of KeyEvent.type that represents null.
#define INPUT_TYPE_KEYNONE 0
// The value of KeyEvent.type that represents a key-up input.
#define INPUT_TYPE_KEYUP 2
// The value of KeyEvent.type that represents a key-down input.
#define INPUT_TYPE_KEYDOWN 3

// Definitions provided by WinInput are contained within the 'input' scope
namespace input {

	struct KeyData {
		unsigned long code = 0UL;
		bool ctrl = false;
		bool shift = false;
		bool alt = false;
		short type = INPUT_TYPE_KEYNONE;
	};

	struct MouseData {
		unsigned code = 0;
		long x = 0;
		long y = 0;
		unsigned long param = 0;
	};

	// Defines the type of function to be passed into addKeyHandler.
	// The function receives a KeyData containing data about the key input.
	// The function should return true if further processing of the input
	// should be halted, and false if otherwise.
	typedef bool(*key_handler_fn)(KeyData& data);

	// Defines the type of function to be passed into addMouseHandler.
	// The function receives a MouseData containing data about the mouse input.
	// The function should return true if further processing of the input
	// should be halted, and false if otherwise.
	typedef bool(*mouse_handler_fn)(MouseData& data);

	// Defines the type of function to be passed into onKeyEvent and
	// onMouseEvent.
	// The function should return true if further processing of the input
	// should be halted, and false if otherwise.
	typedef bool(*event_handler_fn)();


	// Register a key_handler_fn for handling keyboard events.
	// Returns true if successful, and false if otherwise.
	bool addKeyHandler(key_handler_fn fn);

	// Register a mouse_handler_fn for handling mouse events.
	// Returns true if successful, and false if otherwise.
	bool addMouseHandler(mouse_handler_fn fn);

	// Remove the previously registered key_handler_fn.
	// Returns true if successful, and false if otherwise.
	bool removeKeyHandler(key_handler_fn fn);

	// Remove the previously registered mouse_handler_fn.
	// Returns true if successful, and false if otherwise.
	bool removeMouseHandler(mouse_handler_fn fn);

	// Register an event_handler_fn that is called when the given sequence
	// of key event(s) is observed. Set strict to true if ctrl, shift, alt
	// should also be matched, or false if otherwise.
	// The list of KeyData should be terminated by a 'null' KeyData with vkCode of 0.
	// Returns true if successful, and false if otherwise.
	// The ID of the sequence will be written to sequenceId.
	bool addKeySequence(KeyData *data, bool strict, event_handler_fn fn, int *sequenceId);

	// Register an event_handler_fn that is called when the given sequence
	// of mouse event(s) is observed. Tolerance determines the allowed
	// deviation of the x and y coordinate from the values specified in data.
	// The list of MouseData should be terminated by a 'null' MouseData with code of 0.
	// Returns true if successful, and false if otherwise.
	// The ID of the sequence will be written to sequenceId.
	bool addMouseSequence(MouseData *data, unsigned tolerance, event_handler_fn fn, int *sequenceId);

	// Remove the previously registered sequence that matches the given sequenceId.
	// Returns true if successful, and false if otherwise.
	bool removeKeySequence(int sequenceId);

	// Remove the previously registered sequence that matches the given sequenceId.
	// Returns true if successful, and false if otherwise.
	bool removeMouseSequence(int sequenceId);

	// Sets whether the state of ctrl, shift, and alt should be internally tracked.
	// This should be enabled if you are going to block those keys from reaching
	// the OS inside your key handler function.
	void trackModifierState(bool track);

	// Remove the keyboard and mouse hooks, and stops the internal message handling thread.
	void shutdown();

}
