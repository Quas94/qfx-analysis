#pragma once

#include <vector>

#include "AbstractIndicator.hpp"

class Strategy {
private:

	// pointer to the parser object
	Parser *parser;

	// strategy config
	const int stop_loss_pips;
	const int take_profit_pips;
	vector<AbstractIndicator*> indicators;
	
	// outcome related fields
	int net_pips;
	int num_trades_closed;
	int num_trades_lost;
	int num_trades_won;
	Position current_position;

	void print_indicators();

public:

	Strategy(Parser *parser, int stop_loss_pips, int take_profit_pips);

	~Strategy();

	void run();

	static bool stopped_out(Position pos, double stop_loss, double price);
	static bool taken_profit(Position pos, double take_profit, double price);
};
