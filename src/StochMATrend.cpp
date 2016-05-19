#include "StochMATrend.hpp"
#include "Stochastic.hpp"
#include "MovingAverageTrend.hpp"

StochMATrend::StochMATrend(Parser *parser) {
	mas = {
		new MovingAverageTrend(parser, false, 10, 25, 50),
		new MovingAverageTrend(parser, false, 25, 50, 100),
		new MovingAverageTrend(parser, false, 50, 100, 200),
		new MovingAverageTrend(parser, false, 100, 200, 400),
	};
	stochs = {
		new Stochastic(parser, 5, 95, 14),
		new Stochastic(parser, 10, 90, 14),
		new Stochastic(parser, 15, 85, 14),
		new Stochastic(parser, 5, 95, 9),
		new Stochastic(parser, 10, 90, 9),
		new Stochastic(parser, 15, 85, 9),
	};
	for (auto ma = mas.begin(); ma != mas.end(); ma++) {
		for (unsigned int i = 0; i < stochs.size(); i++) {
			indicator_groups.push_back(new vector<AbstractIndicator*>{ *ma, stochs[i] });
		}
	}
	for (auto stoch = stochs.begin(); stoch != stochs.end(); stoch++) {
		indicator_groups.push_back(new vector<AbstractIndicator*>{ *stoch });
	}
}

StochMATrend::~StochMATrend() {
	// delete indicator groups
	for (auto it = indicator_groups.begin(); it != indicator_groups.end(); it++) {
		delete *it;
	}
	// delete indicators
	for (auto it = mas.begin(); it != mas.end(); it++) delete *it;
	for (auto it = stochs.begin(); it != stochs.end(); it++) delete *it;
}
