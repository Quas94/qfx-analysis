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
	SimpleDate entry_date;
	double entry_price;
	double stop_loss;
	double take_profit;

public:
	Trade(Signal signal, const SimpleDate &entry_date, double entry_price, int stop_loss_pips, int take_profit_pips);

	Position get_position() const;
	SimpleDate get_entry_date() const;
	double get_entry_price() const;

	bool stopped_out(double at_price) const;
	bool taken_profit(double at_price) const;
};
