#include <iostream>
#include <iomanip>
#include <limits>

#include "Constants.hpp"
#include "Strategy.hpp"
#include "Stochastic.hpp"
#include "Trade.hpp"
#include "ReverseIndicator.hpp"
#include "Account.hpp"

/**
 * A strategy contains one or more Indicators which will be used collectively in order to
 * decide entry and exit points.
 */
using namespace std;

Strategy::Strategy(Parser *parser, double risk_percent_per_trade, int stop_loss_pips, int take_profit_pips, int cooldown,
	const vector<AbstractIndicator*> *ind_ptrs) : parser(parser), risk_percent_per_trade(risk_percent_per_trade),
	stop_loss_pips(stop_loss_pips), take_profit_pips(take_profit_pips), cooldown(cooldown) {

	// add the indicators to the list
	for (auto it = ind_ptrs->begin(); it != ind_ptrs->end(); it++) {
		indicators.push_back(*it);
	}

	// iterate through all candlesticks and run them by every TA tool
	for (auto it = indicators.begin(); it != indicators.end(); it++) {
		(*it)->process();
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
	int info_index = -1;
	vector<int> info_pips_gained;
	vector<string> info_period_names;
	vector<double> info_account_sizes;

	Account account(1000.0); // start off with 1k funds
	int cooldown_remaining = 0;
	double year_start_equity = account.get_balance(); // balance == equity at beginning
	double month_start_equity = account.get_balance(); // balance == equity at beginning
	double month_best_percent = 0;
	double month_worst_percent = numeric_limits<double>::max();
	int num_winning_months = 0;
	int num_losing_months = 0;

	int num_candles = parser->get_num_candles();
	const double *prices_high = parser->get_high_prices();
	const double *prices_low = parser->get_low_prices();
	const double *prices_close = parser->get_close_prices();
	const SimpleDate *prices_dates = parser->get_date_tracker();

	int prev_year = 0,
		prev_month = 0;

	int total_trades_won = 0;
	int total_trades_lost = 0;
	// can calc total pips from the above 2 variables at the end

	double updated_equity;

	for (int i = 0; i < num_candles; i++) {
		const double current_high = *(prices_high + i);
		const double current_low = *(prices_low + i);
		const double current_close = *(prices_close + i);
		const SimpleDate *current_date = prices_dates + i;

		// inform the account object that the price reached the specified high and low, in this candlebar
		pair<int, int> won_and_lost = account.update_price(current_high, current_low);
		int won = won_and_lost.first;
		int lost = won_and_lost.second;
		int pip_change = (won * take_profit_pips) - (lost * stop_loss_pips);
		total_trades_won += won;
		total_trades_lost += lost;

		updated_equity = account.calc_equity(current_close);

		int current_year = current_date->get_year();
		int current_month = current_date->get_month();
		// update year stats
		if (current_year != prev_year) {
			// new year
			info_index++;
			info_pips_gained.push_back(pip_change);
			info_period_names.push_back(to_string(current_year));
			double year_percent_change = (updated_equity / year_start_equity) * 100;
			// if (prev_year == 0) cout << "first year_percent_change = " << year_percent_change << endl;
			info_account_sizes.push_back(year_percent_change);
			// update prev_year
			prev_year = current_year;
			// cout << "year_start_equity being updated from " << year_start_equity << " -> " << updated_equity << endl;
			year_start_equity = updated_equity;
		} else {
			// same year, just add
			info_pips_gained[info_index] += pip_change;
			info_account_sizes[info_index] = (updated_equity / year_start_equity) * 100;
		}
		// update month stats
		if (current_month != prev_month) {
			if (prev_month != 0) {
				// this isn't the very first month
				double month_percent_change = (updated_equity / month_start_equity) * 100;
				if (month_percent_change < month_worst_percent)
					month_worst_percent = month_percent_change;
				if (month_percent_change > month_best_percent)
					month_best_percent = month_percent_change;
				if (month_percent_change > 100)
					num_winning_months++;
				else
					num_losing_months++;
				// set month_start_equity to current equity
				// cout << "cur_month = " << current_month << ", prev_month = " << prev_month << ", ";
				// cout << "month_start_equity being updated from " << month_start_equity << " -> " << updated_equity << endl;
				month_start_equity = updated_equity;
			}
			prev_month = current_month;
		}

		// start looking for new trades if we are no longer on cooldown
		if (cooldown_remaining) {
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
				// go ahead and make trade because all indicators showed same signal
				account.make_trade(signal, updated_equity, risk_percent_per_trade, *current_date, current_close,
					stop_loss_pips, take_profit_pips);

				// refresh the cooldown
				cooldown_remaining = cooldown;
			}
		}
		cooldown_remaining--;
	}

	// update last month
	// cout << "[LAST] prev_month = " << prev_month << ", ";
	// cout << "month_start_equity being updated from " << month_start_equity << " -> " << updated_equity << endl;
	double month_percent_change = (updated_equity / month_start_equity) * 100;
	if (month_percent_change < month_worst_percent)
		month_worst_percent = month_percent_change;
	if (month_percent_change > month_best_percent)
		month_best_percent = month_percent_change;
	if (month_percent_change > 100)
		num_winning_months++;
	else
		num_losing_months++;

	// notify account that the strategy is finished
	account.clear_trades();

	// summation calculations
	int net_pips = (total_trades_won * take_profit_pips) - (total_trades_lost * stop_loss_pips);
	int total_trades = total_trades_won + total_trades_lost;

	// print to csv output
	print_indicators(out);
	out << cooldown << "," << risk_percent_per_trade << "%," << stop_loss_pips << "," << take_profit_pips << ",";
	out << total_trades_won << "," << total_trades_lost << "," << total_trades << "," <<
		((int)((total_trades_won / (double)total_trades) * 100)) << "%," << net_pips;

	// lastly, the extra info breakdown
	// pips profited from each particular year
	for (unsigned int y = 0; y < info_period_names.size(); y++) {
		out << "," << info_pips_gained[y];
	}
	// account size at the end of each year
	double total = 0;
	bool noteworthy = true;
	bool semi = true;
	for (unsigned int y = 0; y < info_account_sizes.size(); y++) {
		/*
		int acc_sz = (int)round(info_account_sizes[y] * 100);
		if (acc_sz < 80 || (acc_sz < 100 && !noteworthy)) semi = false;
		if (acc_sz < 100) noteworthy = false;
		out << "," << acc_sz << "%";
		total += info_account_sizes[y];
		*/
		total += info_account_sizes[y];
		out << "," << info_account_sizes[y];
	}
	total /= info_account_sizes.size();
	out << "," << total;
	out << "," << (noteworthy ? "x" : (semi ? "." : ""));
	// worst month and best month
	out << "," << (month_best_percent) << "%";
	out << "," << (month_worst_percent) << "%";
	// winning months and losing months
	int total_months = num_winning_months + num_losing_months;
	out << "," << num_winning_months;
	out << ",\"" << num_losing_months << " out of " << total_months << "\"";
	// newline
	out << endl;
}
