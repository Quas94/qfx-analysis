#pragma once

#include <vector>
#include <fstream>

#include "AbstractIndicator.hpp"

class Strategy {
private:

	// pointer to the parser object
	Parser *parser;

	// strategy config
	const double risk_percent_per_trade;
	const int stop_loss_pips;
	const int take_profit_pips;
	vector<AbstractIndicator*> indicators;
	const int cooldown; // number of blocks (Parser Timeframes) to wait before considering another trade

	void print_indicators(ofstream &out);

public:

	Strategy(Parser *parser, double risk_percent_per_trade, int stop_loss_pips, int take_profit_pips, int cooldown, const vector<AbstractIndicator*> *ind_ptrs);

	void run(ofstream &out);
};
