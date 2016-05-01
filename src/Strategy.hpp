#pragma once

#include <vector>
#include <fstream>

#include "AbstractIndicator.hpp"

class Strategy {
private:

	// pointer to the parser object
	Parser *parser;

	// strategy config
	const int stop_loss_pips;
	const int take_profit_pips;
	vector<AbstractIndicator*> indicators;
	const int cooldown; // number of blocks (Parser Timeframes) to wait before considering another trade
	
	// outcome related fields
	int net_pips;
	int num_trades_closed;
	int num_trades_lost;
	int num_trades_won;

	void print_indicators(ofstream &out);

public:

	Strategy(Parser *parser, int stop_loss_pips, int take_profit_pips, int cooldown);

	~Strategy();

	void run(ofstream &out);
};
