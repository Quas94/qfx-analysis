#pragma once

#include "AbstractIndicator.hpp"

enum Position {
	LONG,
	SHORT,
	NO_POSITION
};

class Trade {
private:
	Position position;
	double entry_price;
	double stop_loss;
	double take_profit;

public:
	Trade(Signal signal, double entry_price, int stop_loss_pips, int take_profit_pips);

	bool stopped_out(double at_price);
	bool taken_profit(double at_price);
};
