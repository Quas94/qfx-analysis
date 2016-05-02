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

Strategy::Strategy(Parser *parser, double risk_percent_per_trade, int stop_loss_pips, int take_profit_pips, int cooldown, const vector<AbstractIndicator*> *ind_ptrs) :
	parser(parser), risk_percent_per_trade(risk_percent_per_trade), stop_loss_pips(stop_loss_pips), take_profit_pips(take_profit_pips), cooldown(cooldown) {

	// add the indicators to the list
	for (auto it = ind_ptrs->begin(); it != ind_ptrs->end(); it++) {
		indicators.push_back(*it);
	}

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
	// each index of this vector will represent one unit of time being tracked (currently, just 1 month) for the extra info columns
	vector<int> extra_info_pips_gained;
	vector<string> extra_info_blocknames;
	vector<double> extra_info_account_sizes; // account sizes with respect to starting account size of 1.0, at the end of each block

	// account sizing
	double account_size = 1.0; // 1.0 = 100%
	const double percent_loss = risk_percent_per_trade / 100.0;
	const double percent_win = (((double) take_profit_pips) / stop_loss_pips) * percent_loss;
	const double account_size_multiply_loss = 1 - percent_loss; // multiply account_size by this variable when we encounter a trade loss
	const double account_size_multiply_win = 1 + percent_win; // multiply account_size by this variable when we encounter a trade win

	// fetch candles and dates
	int num_candles = parser->get_num_candles();
	const double *close_prices = parser->get_close_prices();
	const SimpleDate *date_tracker = parser->get_date_tracker();
	// keep track of last line's month and year
	int prev_month = 0;
	int prev_year = 0;
	int extra_info_index = -1;
	// iterate through all candlesticks and apply strategy
	for (int i = 0; i < num_candles; i++) {
		double current_price = *(close_prices + i);
		const SimpleDate *current_date = date_tracker + i;
		// if we're in any positions, check if we hit stop loss or take profit
		auto it = open_trades.begin();
		while (it != open_trades.end()) {
			Trade *trade = *it;
			bool stopped = trade->stopped_out(current_price);
			bool profited = trade->taken_profit(current_price);
			if (stopped || profited) {
				// close out the position
				num_trades_closed++;
				int pip_change;
				if (stopped) {
					pip_change = -stop_loss_pips;
					account_size *= account_size_multiply_loss;
					num_trades_lost++;
				} else { // profited
					pip_change = take_profit_pips;
					account_size *= account_size_multiply_win;
					num_trades_won++;
				}
				net_pips += pip_change;
				// remove from vector and delete
				it = open_trades.erase(it);
				delete trade;
				// add to extra info
				int cur_year = current_date->get_year();
				// int cur_month = current_date->get_month();
				if (prev_year != cur_year) { // new year
					prev_year = cur_year;
					// prev_month = cur_month;
					extra_info_index++;
					extra_info_pips_gained.push_back(pip_change);
					extra_info_blocknames.push_back(to_string(cur_year));
					extra_info_account_sizes.push_back(account_size);
					// reset account_size to 1.0
					account_size = 1.0;
				} else {
					// same year, just add on the net_pips
					extra_info_pips_gained[extra_info_index] += pip_change;
					extra_info_account_sizes[extra_info_index] = account_size;
				}
			} else {
				it++;
			}
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
	print_indicators(out);
	out << cooldown << "," << risk_percent_per_trade << "%," << stop_loss_pips << "," << take_profit_pips << ",";
	out << num_trades_won << "," << num_trades_lost << "," << num_trades_closed << "," <<
		((int)((num_trades_won / (double)num_trades_closed) * 100)) << "%," << net_pips;
	// lastly, the extra info breakdown if there was more than 1 year
	if (extra_info_blocknames.size() > 1) {
		// pips profited from each particular year
		for (unsigned int y = 0; y < extra_info_blocknames.size(); y++) {
			out << "," << extra_info_pips_gained[y];
		}
		// account size at the end of each year
		for (unsigned int y = 0; y < extra_info_account_sizes.size(); y++) {
			out << "," << ((int) round(extra_info_account_sizes[y] * 100)) << "%";
		}
	}
	// newline
	out << endl;
}
