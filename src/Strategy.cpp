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
	const vector<AbstractIndicator*> *ind_ptrs, bool move_stop) : parser(parser),
	risk_percent_per_trade(risk_percent_per_trade), stop_loss_pips(stop_loss_pips), take_profit_pips(take_profit_pips),
	cooldown(cooldown), move_stop(move_stop) {

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
	out << ",desc:[";
	for (auto it = indicators.begin(); it != indicators.end(); it++) {
		if (it != indicators.begin()) out << ",";
		out << "\"" << (*it)->get_desc() << "\"";
	}
	out << "],";
}

void Strategy::run(ofstream &out) {
	// determine if jpy pair
	bool is_jpy_pair = (parser->get_currency_pair().find(JPY) != string::npos);
	string currency_pair = parser->get_currency_pair();

	int info_index = -1;
	vector<string> info_period_names;
	vector<double> info_account_sizes;

	const double initial_balance = 1000;
	Account account(initial_balance); // start off with 1k funds
	int cooldown_remaining = 0;
	double year_start_equity = account.get_balance(); // balance == equity at beginning
	double month_start_equity = account.get_balance(); // balance == equity at beginning
	double month_best_percent = 0;
	double month_worst_percent = numeric_limits<double>::max();
	int num_winning_months = 0;
	int num_losing_months = 0;
	vector<double> month_percentages;

	int num_candles = parser->get_num_candles();
	const double *prices_high = parser->get_high_prices();
	const double *prices_low = parser->get_low_prices();
	const double *prices_close = parser->get_close_prices();
	const SimpleDate *prices_dates = parser->get_date_tracker();

	int prev_year = 0,
		prev_month = 0;

	int total_trades_won = 0;
	int total_trades_lost = 0;
	int total_trades_neutral = 0;
	// can calc total pips from the above 2 variables at the end

	double updated_equity;

	for (int i = 0; i < num_candles; i++) {
		const double current_high = *(prices_high + i);
		const double current_low = *(prices_low + i);
		const double current_close = *(prices_close + i);
		const SimpleDate *current_date = prices_dates + i;
		const int current_year = current_date->get_year();
		const int current_month = current_date->get_month();

		// inform the account object that the price reached the specified high and low, in this candlebar
		pair<pair<int, int>, int> won_neutral_lost = account.update_price(current_high, current_low);
		int won = won_neutral_lost.first.first;
		int lost = won_neutral_lost.first.second;
		int neutral = won_neutral_lost.second;
		int pip_change = (won * take_profit_pips) - (lost * stop_loss_pips);
		total_trades_won += won;
		total_trades_lost += lost;
		total_trades_neutral += neutral;

		updated_equity = account.calc_equity(current_close);

		// if at any point, equity falls below 10% of starting equity, set it to zero
		if (updated_equity < (initial_balance / 10)) {
			account.set_failed();
			updated_equity = 0;
		}

		// update year stats
		double year_percent_change = (year_start_equity == 0) ? 0 : ((updated_equity / year_start_equity) * 100);
		if (current_year != prev_year) {
			// new year
			info_index++;
			info_period_names.push_back(to_string(current_year));
			// if (prev_year == 0) cout << "first year_percent_change = " << year_percent_change << endl;
			info_account_sizes.push_back(year_percent_change);
			// update prev_year
			//cout << "in " << prev_year << ", equity went from from " << year_start_equity << " -> " <<
			//	updated_equity << ", year_percent_change = " << year_percent_change << endl;
			prev_year = current_year;
			year_start_equity = updated_equity;
		} else {
			// same year, just add
			info_account_sizes[info_index] = year_percent_change;
		}
		// update month stats
		if (current_month != prev_month) {
			if (prev_month != 0) {
				// this isn't the very first month
				double month_percent_change = (month_start_equity == 0) ? 0 : ((updated_equity / month_start_equity) * 100);
				month_percentages.push_back(month_percent_change);
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
		if (cooldown_remaining <= 0) {
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
					stop_loss_pips, take_profit_pips, is_jpy_pair, move_stop);

				// refresh the cooldown
				cooldown_remaining = cooldown;
			}
		}
		cooldown_remaining--;
	}

	// update last month
	// cout << "[LAST] prev_month = " << prev_month << ", ";
	// cout << "month_start_equity being updated from " << month_start_equity << " -> " << updated_equity << endl;
	double month_percent_change = (month_start_equity == 0) ? 0 : ((updated_equity / month_start_equity) * 100);
	month_percentages.push_back(month_percent_change);
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
	//int net_pips = (total_trades_won * take_profit_pips) - (total_trades_lost * stop_loss_pips);
	int total_trades = total_trades_won + total_trades_lost + total_trades_neutral;

	// print to csv output
	out << "{"; // beginning of javascript object
	out << "pair:\"" << currency_pair << "\"";
	print_indicators(out);
	out << "cd:" << cooldown << "," << "risk:" << risk_percent_per_trade << ",";
	out << "sl:" << stop_loss_pips << ",tp:" << take_profit_pips << ",";
	out << "winners:" << total_trades_won << ",losers:" << total_trades_lost;
	out << ",neutral:" << total_trades_neutral;
	out << ",trades:" << total_trades;
	// out << "," << net_pips;

	// lastly, the extra info breakdown
	// account size at the end of each year
	double final_multiplier = 1.0;
	bool noteworthy = true;
	bool semi = true;
	bool bust = false;
	double worst = numeric_limits<double>::max();
	out << ",years:[";
	for (unsigned int y = 0; y < info_account_sizes.size(); y++) {
		double acc_sz = info_account_sizes[y];
		if (acc_sz == 0) bust = true;
		if (acc_sz < 80 || (acc_sz < 100 && !noteworthy)) semi = false;
		if (acc_sz < 100) noteworthy = false;
		out << acc_sz << ",";
		if (acc_sz < worst) worst = acc_sz;
		final_multiplier *= (acc_sz / 100);
	}
	out << "]";
	out << ",final:" << final_multiplier;
	out << ",worst:" << worst;
	// months
	out << ",months:[";
	for (unsigned int m = 0; m < month_percentages.size(); m++) {
		out << month_percentages[m] << ",";
		// if (m == month_percentages.size() - 1) cout << "last month = " << month_percentages[m] << endl;
	}
	out << "]";
	// worst month and best month
	out << ",month_worst:" << (month_worst_percent);
	out << ",month_best:" << (month_best_percent);
	// winning months and losing months
	int total_months = num_winning_months + num_losing_months;
	out << ",month_winners:" << num_winning_months;
	out << ",month_losers:" << num_losing_months;
	out << ",warnings:" << account.get_num_warnings();
	// newline
	out << "},"; // end of javascript object
	out << endl;
}
