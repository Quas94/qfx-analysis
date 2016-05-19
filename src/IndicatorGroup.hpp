#pragma once

#include <vector>
#include "AbstractIndicator.hpp"

using namespace std;

/**
 * Simple class to store groups of indicators as overarching strategies.
 */
class IndicatorGroup {
protected:
	vector<vector<AbstractIndicator*>*> indicator_groups;

public:
	virtual const vector<vector<AbstractIndicator*>*>& get_groups() final {
		return indicator_groups;
	}

	~IndicatorGroup() {
		// delete indicator groups
		for (auto it = indicator_groups.begin(); it != indicator_groups.end(); it++) {
			delete *it;
		}
	}
};
