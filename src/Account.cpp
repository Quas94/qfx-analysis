#include <iostream>

#include "Account.hpp"

using namespace std;

Account::Account(double starting_balance) : balance(starting_balance), num_warnings(0) {
	// nothing to do
}

void Account::make_trade(Signal signal, double balance, double risk, const SimpleDate &entry_date,
	double entry, int stop_loss, int take_profit, bool jpy_pair, bool move_stop) {

	Trade *trade = new Trade(signal, balance, risk, entry_date, entry, stop_loss, take_profit, jpy_pair, move_stop);
	trades.push_back(trade);
}

// notifies the account that the current price has reached the given high and low
// this method will then stop-loss or take-profit any trades that are applicable
pair<pair<int, int>, int> Account::update_price(double high, double low) {
	auto it = trades.begin();
	int won = 0,
		neutral = 0,
		lost = 0;
	while (it != trades.end()) {
		Trade *trade = *it;
		TradeResult result = trade->check_high_low(high, low);
		if (result == ACTIVE) {
			// do nothing, let the trade continue running its course
			it++;
		} else {
			// unless trade is still active, we remove it
			it = trades.erase(it);
			if (result == TAKE_PROFIT) {
				// winning trade
				balance += trade->get_winnings();
				won++;
			} else if (result == BREAK_EVEN) {
				// no change to balance
				neutral++;
			} else { // result == STOP_LOSS || result == ERROR
				// losing trade
				balance -= trade->get_losses();
				lost++;
				// inc warning if error
				if (result == ERROR)
					num_warnings++;
			}
			// free memory
			delete trade;
		}
	}
	return make_pair(make_pair(won, lost), neutral);
}

double Account::get_balance() {
	return balance;
}

// supplied with the close price of the latest bar, returns the equity of the account
// equity = balance + impacts of currently open trades
double Account::calc_equity(double close) {
	double active_trades_effect = 0;
	for (auto it = trades.begin(); it != trades.end(); it++) {
		active_trades_effect += (*it)->get_balance_change(close);
	}
	return balance + active_trades_effect;
}

void Account::clear_trades() {
	for (auto it = trades.begin(); it != trades.end(); it++) {
		delete *it;
	}
	trades.clear();
}

void Account::set_failed() {
	clear_trades();
	balance = 0;
}

int Account::get_num_warnings() {
	return num_warnings;
}
