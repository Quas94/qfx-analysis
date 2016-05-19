#pragma once

#include "IndicatorGroup.hpp"
#include "Parser.hpp"

class StochMATrendGroup : public IndicatorGroup {
private:
	vector<AbstractIndicator*> mas;
	vector<AbstractIndicator*> stochs;

public:
	StochMATrendGroup(Parser *parser);

	~StochMATrendGroup();
};
