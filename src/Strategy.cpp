#include <iostream>

#include "Constants.hpp"
#include "Strategy.hpp"
#include "Stochastic.hpp"
#include "Trade.hpp"
#include "ReverseIndicator.hpp"

/**
 * A strategy contains one or more Indicators which will be used collectively in order to
 * decide entry and exit points.
 */
using namespace std;

Strategy::Strategy(Parser *parser, int stop_loss_pips, int take_profit_pips, int cooldown) : parser(parser),
	stop_loss_pips(stop_loss_pips), take_profit_pips(take_profit_pips), cooldown(cooldown) {

	// initialise and insert a bunch of AbstractIndicator implementations here
	indicators.push_back(new Stochastic(parser, 10, 90));
	// indicators.push_back(new ReverseIndicator(new Stochastic(parser, 15, 85)));
	// indicators.push_back(new ReverseIndicator(new Stochastic(parser, 10, 90)));
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
}

Strategy::~Strategy() {
	for (auto it = indicators.begin(); it != indicators.end(); it++) {
		delete (*it);
	}
}

void Strategy::print_indicators(ofstream &out) {
	out << "\"";
	for (auto it = indicators.begin(); it != indicators.end(); it++) {
		if (it != indicators.begin()) out << " & ";
		out << (*it)->get_desc();
	}
	out << "\",";
}

void Strategy::run(ofstream &out) {
	// stores all currently open trades
	vector<Trade*> open_trades;

	// this value must be 0 or less before we can start looking for trades
	// will be reset to be equal to 'cooldown' when a trade is made
	int cooldown_remaining = 0;

	// iterate through all candlesticks and apply strategy
	int num_candles = parser->get_num_candles();
	const double *close_prices = parser->get_close_prices();
	for (int i = 0; i < num_candles; i++) {
		double current_price = *(close_prices + i);
		// if we're in any positions, check if we hit stop loss or take profit
		auto it = open_trades.begin();
		while (it != open_trades.end()) {
			Trade *trade = *it;
			bool stopped = trade->stopped_out(current_price);
			bool profited = trade->taken_profit(current_price);
			if (stopped || profited) {
				// close out the position
				num_trades_closed++;
				if (stopped) {
					net_pips -= stop_loss_pips;
					num_trades_lost++;
				} else { // profited
					net_pips += take_profit_pips;
					num_trades_won++;
				}
				// remove from vector and delete
				it = open_trades.erase(it);
				delete trade;
			} else {
				it++;
			}
			// else if (i == num_candles - 1) {
				// we're on the last candle of this dataset
				// out << "Held " << (current_position == LONG ? "long" : "short") << " position at end. Entry = " <<
				//	to_string(entry_price) << ", price at end = " << to_string(current_price) << endl;
			// }
		}

		// if cooldown_remaining is 0 or less, we can check for new trades
		if (cooldown_remaining <= 0) {
			// make a trade if all indicators in ta_list come to the same conclusion
			// regardless of whether or not we already have trade(s) open
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
				if (new_signal != signal) {
					signal = NOT_APPLICABLE;
					break; // is opposite to what we had, or neutral, or not applicable
				}
			}
			// check what signal we ended up with
			if (signal == BUY || signal == SELL) {
				// actually do something here because all indicators showed same signal
				double entry_price = current_price;
				Trade *new_trade = new Trade(signal, current_price, stop_loss_pips, take_profit_pips);
				open_trades.push_back(new_trade);
				// refresh the cooldown
				cooldown_remaining = cooldown;
			}
		}
		cooldown_remaining--;
	}

	// if any trades still open, delete them
	for (auto it = open_trades.begin(); it != open_trades.end(); it++) {
		delete *it;
	}
	open_trades.clear();

	// print to csv output
	// out << "Indicator list and descriptions,Cooldown,Stop loss,Take profit,Winners,Losers,Total trades," <<
	// "Win %,Pips gained" << endl;
	print_indicators(out);
	out << cooldown << "," << stop_loss_pips << "," << take_profit_pips << ",";
	out << num_trades_won << "," << num_trades_lost << "," << num_trades_closed << "," <<
		((int)((num_trades_won / (double)num_trades_closed) * 100)) << "%," << net_pips << endl;
}
