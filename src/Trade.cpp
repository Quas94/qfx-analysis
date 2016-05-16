#include "Trade.hpp"
#include "AbstractIndicator.hpp"
#include "Constants.hpp"

Trade::Trade(Signal signal, double balance_on_entry, double risk_percentage, const SimpleDate &entry_date,
	double entry_price, int stop_loss_pips, int take_profit_pips, bool is_jpy, bool move_stop) :
	position(signal == BUY ? LONG : SHORT), entry_date(entry_date), entry_price(entry_price), jpy_currency(is_jpy),
	stop_loss_pips(stop_loss_pips), take_profit_pips(take_profit_pips), move_stop(move_stop), move_stop_done(false) {

	if (signal != BUY && signal != SELL)
		throw runtime_error("signal must be BUY or SELL when passed into Trade::Trade() constructor");

	// calculate balance_per_pip
	double balance_risk = balance_on_entry * (risk_percentage / 100);
	balance_per_pip = balance_risk / stop_loss_pips;

	double stop_loss_diff = stop_loss_pips / (jpy_currency ? PIP_PRICE_RATIO_JPY : PIP_PRICE_RATIO);
	double take_profit_diff = take_profit_pips / (jpy_currency ? PIP_PRICE_RATIO_JPY : PIP_PRICE_RATIO);
	if (signal == BUY) {
		stop_loss = entry_price - stop_loss_diff;
		take_profit = entry_price + take_profit_diff;
		move_stop_price = entry_price + stop_loss_diff;
	} else { // signal == SELL
		stop_loss = entry_price + stop_loss_diff;
		take_profit = entry_price - take_profit_diff;
		move_stop_price = entry_price - stop_loss_diff;
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

// calculate the current effect on balance of this trade
double Trade::get_balance_change(double at_price) const {
	double price_diff = at_price - entry_price;
	if (position == SHORT) // short means we gain profit if at_price < entry_price
		price_diff *= -1;

	double pips = price_diff / (jpy_currency ? PIP_PRICE_RATIO_JPY : PIP_PRICE_RATIO);
	return pips * balance_per_pip;
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

TradeResult Trade::check_high_low(double high, double low) {
	bool sl,
		 tp;
	if (position == LONG) {
		// move stop to break-even if flag is set
		if (move_stop && !move_stop_done && high >= move_stop_price) {
			stop_loss = entry_price;
			move_stop_done = true;
		}
		// then check sl and tp
		sl = stopped_out(low);
		tp = taken_profit(high);
	} else { // position == SHORT
		// move stop to break-even if flag is set
		if (move_stop && !move_stop_done && low <= move_stop_price) {
			stop_loss = entry_price;
			move_stop_done = true;
		}
		// then check sl and tp
		sl = stopped_out(high);
		tp = taken_profit(low);
	}
	if (sl && tp)
		return ERROR;
	if (sl)
		return move_stop_done ? BREAK_EVEN : STOP_LOSS;
	if (tp)
		return TAKE_PROFIT;

	return ACTIVE;
}

double Trade::get_winnings() const {
	return balance_per_pip * take_profit_pips;
}

double Trade::get_losses() const {
	return balance_per_pip * stop_loss_pips;
}
