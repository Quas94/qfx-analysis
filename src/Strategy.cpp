#include <iostream>

#include "Constants.hpp"
#include "Strategy.hpp"
#include "Stochastic.hpp"

/**
 * A strategy contains one or more Indicators which will be used collectively in order to
 * decide entry and exit points.
 */
using namespace std;

Strategy::Strategy(Parser *parser, int stop_loss_pips, int take_profit_pips) : parser(parser),
	stop_loss_pips(stop_loss_pips), take_profit_pips(take_profit_pips) {

	// initialise and insert a bunch of AbstractIndicator implementations here
	Stochastic *stochastic = new Stochastic(parser);
	indicators.push_back(stochastic);
	// @TODO add more

	// iterate through all candlesticks and run them by every TA tool
	for (auto it = indicators.begin(); it != indicators.end(); it++) {
		(*it)->process();
	}

	// clear outcome logs
	this->net_pips = 0;
	this->num_trades_closed = 0;
	this->num_trades_lost = 0;
	this->num_trades_won = 0;
	this->current_position = NONE;
}

Strategy::~Strategy() {
	for (auto it = indicators.begin(); it != indicators.end(); it++) {
		delete (*it);
	}
}

void Strategy::print_indicators() {
	cout << "[Indicators List]" << endl;
	for (auto it = indicators.begin(); it != indicators.end(); it++) {
		cout << (*it)->get_desc() << endl;
	}
}

void Strategy::run() {
	cout << "[Strategy] SL = " << to_string(stop_loss_pips) << " / TP = " <<
		to_string(take_profit_pips) << endl;
	print_indicators();

	// variables for storing the current trade
	double entry_price;
	double stop_loss;
	double take_profit;

	// calculate actual price diff of stop loss and take profit
	const double stop_loss_diff = stop_loss_pips / PIP_PRICE_RATIO;
	const double take_profit_diff = take_profit_pips / PIP_PRICE_RATIO;

	// iterate through all candlesticks and apply strategy
	int num_candles = parser->get_num_candles();
	const double *close_prices = parser->get_close_prices();
	for (int i = 0; i < num_candles; i++) {
		double current_price = *(close_prices + i);
		// cout << "Hour " << to_string(i + 1) << ": price = " << to_string(current_price) << endl;
		// if we're in a position, check if we hit stop loss or take profit
		if (current_position != NONE) {
			bool stopped = stopped_out(current_position, stop_loss, current_price);
			bool profited = taken_profit(current_position, take_profit, current_price);
			if (stopped || profited) {
				// close out the position
				current_position = NONE;
				num_trades_closed++;
				if (stopped) {
					net_pips -= (int) round(stop_loss_diff * PIP_PRICE_RATIO);
					num_trades_lost++;
					// cout << "[Trade] Stopped out at " << to_string(current_price) << ", lost 50 pips" << endl;
				} else { // profited
					net_pips += (int) round(take_profit_diff * PIP_PRICE_RATIO);
					num_trades_won++;
					// cout << "[Trade] Took profit at " << to_string(current_price) << ", gained 100 pips" << endl;
				}
			} else if (i == num_candles - 1) {
				// we're on the last candle of this dataset
				//cout << "Held " << (current_position == LONG ? "long" : "short") << " position at end. Entry = " <<
				//	to_string(entry_price) << ", price at end = " << to_string(current_price) << endl;
			}
		} else {
			// only make a decision if all indicators in ta_list come to the same conclusion
			Signal signal = _NULL;
			for (auto it = indicators.begin(); it != indicators.end(); it++) {
				AbstractIndicator *indicator = *it;
				if (signal == _NULL) {
					// very first iteration
					signal = indicator->get_signal(i);
					if (signal == NEUTRAL || signal == NOT_APPLICABLE)
						break;
					continue;
				}
				Signal new_signal = indicator->get_signal(i);
				if (new_signal != signal)
					break; // is opposite to what we had, or neutral, or not applicable
			}
			// check what signal we ended up with
			if (signal == BUY || signal == SELL) {
				// actually do something here because all indicators showed same signal
				entry_price = current_price;
				if (signal == BUY) {
					current_position = LONG;
					stop_loss = entry_price - stop_loss_diff;
					take_profit = entry_price + take_profit_diff;
				} else { // signal == SELL
					current_position = SHORT;
					stop_loss = entry_price + stop_loss_diff;
					take_profit = entry_price - take_profit_diff;
				}
				//cout << "[Trade] Opened a " << (current_position == LONG ? "long" : "short") << " position at " <<
				//	to_string(entry_price) << " (hour " << to_string(i + 1) << ")" << endl;
			}
		}
	}
	cout << "Winners: " << to_string(num_trades_won) << "/" << to_string(num_trades_closed) << "(" <<
		to_string((int)((num_trades_won / (double)num_trades_closed) * 100)) << "%)" << endl;
	cout << "Losers: " << to_string(num_trades_lost) << "/" << to_string(num_trades_closed) << endl;
	cout << "[PIPS GAINED: " << to_string(net_pips) << "]" << endl;
	cout << "=---------------------------------------------=" << endl;
}

bool Strategy::stopped_out(Position position, double stop_loss, double price) {
	if (position == LONG) {
		return price <= stop_loss;
	} else { // SHORT
		return price >= stop_loss;
	}
}

bool Strategy::taken_profit(Position position, double take_profit, double price) {
	if (position == LONG) {
		return price >= take_profit;
	} else { // SHORT
		return price <= take_profit;
	}
}
