#include "Trade.hpp"
#include "AbstractIndicator.hpp"
#include "Constants.hpp"

Trade::Trade(Signal signal, double entry_price, int stop_loss_pips,
	int take_profit_pips) : position(signal == BUY ? LONG : SHORT), entry_price(entry_price) {

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

bool Trade::stopped_out(double at_price) {
	if (position == LONG)
		return at_price <= stop_loss;
	else // position == SHORT
		return at_price >= stop_loss;
}

bool Trade::taken_profit(double at_price) {
	if (position == LONG)
		return at_price >= take_profit;
	else // position == SHORT
		return at_price <= take_profit;
}
