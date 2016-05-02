#include "Trade.hpp"
#include "AbstractIndicator.hpp"
#include "Constants.hpp"

Trade::Trade(Signal signal, const SimpleDate &entry_date, double entry_price, int stop_loss_pips,
	int take_profit_pips) : position(signal == BUY ? LONG : SHORT), entry_date(entry_date), entry_price(entry_price) {

	if (signal != BUY && signal != SELL)
		throw runtime_error("signal must be BUY or SELL when passed into Trade::Trade() constructor");

	double stop_loss_diff = stop_loss_pips / PIP_PRICE_RATIO;
	double take_profit_diff = take_profit_pips / PIP_PRICE_RATIO;
	if (signal == BUY) {
		stop_loss = entry_price - stop_loss_diff;
		take_profit = entry_price + take_profit_diff;
	} else { // signal == SELL
		stop_loss = entry_price + stop_loss_diff;
		take_profit = entry_price - take_profit_diff;
	}
}

Position Trade::get_position() const {
	return position;
}

SimpleDate Trade::get_entry_date() const {
	return entry_date;
}

double Trade::get_entry_price() const {
	return entry_price;
}

bool Trade::stopped_out(double at_price) const {
	if (position == LONG)
		return at_price <= stop_loss;
	else // position == SHORT
		return at_price >= stop_loss;
}

bool Trade::taken_profit(double at_price) const {
	if (position == LONG)
		return at_price >= take_profit;
	else // position == SHORT
		return at_price <= take_profit;
}
