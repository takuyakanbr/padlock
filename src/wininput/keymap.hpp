#pragma once

#include <string>
#include <map>
#include "wininput.hpp"

// Definitions provided by WinInput are contained within the 'input' scope
namespace input {

	// Sets up the internal map used for mapping keys to strings.
	// This must be called before any calls to keyToString is made.
	void setupCodemap();

	// Returns a string representation of the given KeyData.
	std::string keyToString(const KeyData& key);
}