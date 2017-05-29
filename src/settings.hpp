#pragma once

#include "state.hpp"

namespace settings {

	bool loadOptions(state::Options& opts);

	bool saveOptions(const state::Options& opts);
}