#pragma once

#include <vector>

#include "Trade.hpp"

class Account {
private:
	double balance; // only factors in closed trades

	vector<Trade*> trades;

	int num_warnings;

public:

	Account(double starting_balance);

	void make_trade(Signal signal, double balance, double risk, const SimpleDate &entry_date,
		double entry, int stop_loss, int take_profit);

	pair<int, int> update_price(double high, double low);

	double get_balance();
	double calc_equity(double close);

	void clear_trades();

	void set_failed();

	int get_num_warnings();
};
