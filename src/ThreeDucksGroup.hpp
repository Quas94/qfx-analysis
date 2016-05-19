#pragma once

#include "IndicatorGroup.hpp"
#include "Parser.hpp"

class ThreeDucksGroup : public IndicatorGroup {
private:
	vector<AbstractIndicator*> ducks;

public:
	ThreeDucksGroup(Parser *parser);

	~ThreeDucksGroup();
};
