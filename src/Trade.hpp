#pragma once

#include "AbstractIndicator.hpp"

enum Position {
	LONG,
	SHORT,
	NO_POSITION
};

enum TradeResult {
	TAKE_PROFIT = 1,
	ACTIVE = 0,
	STOP_LOSS = -1,
	ERROR = -2
};

class Trade {
private:
	Position position;
	SimpleDate entry_date;
	double entry_price;
	const int stop_loss_pips;
	const int take_profit_pips;
	double stop_loss;
	double take_profit;
	double balance_per_pip;
	bool jpy_currency;

	bool stopped_out(double at_price) const;
	bool taken_profit(double at_price) const;

public:
	Trade(Signal signal, double balance_on_entry, double risk_percentage, const SimpleDate &entry_date,
		double entry_price, int stop_loss_pips, int take_profit_pips, bool is_jpy_pair);

	Position get_position() const;
	SimpleDate get_entry_date() const;
	double get_entry_price() const;

	double get_balance_change(double at_price) const;
	TradeResult check_high_low(double high, double low) const;
	double get_winnings() const;
	double get_losses() const;
};
