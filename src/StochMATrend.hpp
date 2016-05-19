#pragma once

#include "IndicatorGroup.hpp"
#include "Parser.hpp"

class StochMATrend : public IndicatorGroup {
private:
	vector<AbstractIndicator*> mas;
	vector<AbstractIndicator*> stochs;

public:
	StochMATrend(Parser *parser);

	~StochMATrend();
};
