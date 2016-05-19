#include <iostream>
#include "ThreeDucks.hpp"

ThreeDucks::ThreeDucks(Parser *parser, bool default_settings, int crossing_strength_pips, int checking_width) :
	AbstractIndicator(parser), crossing_strength_pips(crossing_strength_pips), checking_width(checking_width) {

	if (checking_width % 3 != 0) {
		cout << "checking_width must be a multiple of 3, but was " << checking_width << endl;
		throw runtime_error("ThreeDucks checking_width error");
	}

	int max_candles = parser->get_max_candles();
	a_ma = new double[max_candles];
	b_ma = new double[max_candles];
	c_ma = new double[max_candles];

	if (default_settings) {
		cout << "Error: ThreeDucks default settings not implemented yet. Use custom." << endl;
		throw runtime_error("three ducks error");
	}

	simple_ma = true;
	a_period = 60; // 60 ma on 1-hour chart
	b_period = 240; // 60 ma on 4-hour chart
	c_period = 960; // 60 ma on daily chart

	// init internal tracker variables
	last_checked_index = -1;
}

ThreeDucks::~ThreeDucks() {
	delete a_ma;
	delete b_ma;
	delete c_ma;
}

void ThreeDucks::process() {
	int num_candles = parser->get_num_candles();
	const double *close_prices = parser->get_close_prices();

	TA_RetCode ret_code_a = TA_MA(0, num_candles - 1, close_prices, a_period,
		simple_ma ? TA_MAType_SMA : TA_MAType_EMA, &a_begin, &a_num, a_ma);
	TA_RetCode ret_code_b = TA_MA(0, num_candles - 1, close_prices, b_period,
		simple_ma ? TA_MAType_SMA : TA_MAType_EMA, &b_begin, &b_num, b_ma);
	TA_RetCode ret_code_c = TA_MA(0, num_candles - 1, close_prices, c_period,
		simple_ma ? TA_MAType_SMA : TA_MAType_EMA, &c_begin, &c_num, c_ma);
}

Signal ThreeDucks::get_signal(int index) {
	int a_index = index - a_begin;
	int b_index = index - b_begin;
	int c_index = index - c_begin;
	if (a_index < 0 || b_index < 0 || c_index < 0) {
		return NOT_APPLICABLE;
	}
	// a = 1-hour, b = 4-hour, c = daily
	if (a_index < checking_width) {
		cout << "a_index is only " << a_index << ", this shouldn't happen" << endl;
		throw runtime_error("error in ThreeDucks::get_signal()");
	}

	double a = a_ma[a_index];
	double b = b_ma[b_index];
	double c = c_ma[c_index];

	const double *close_prices = parser->get_close_prices();
	double price = *(close_prices + index);

	double pip_value = Util::get_pip_value(parser->get_currency_pair());
	double crossing_strength_price = crossing_strength_pips * pip_value;

	// if price is on the same side of all three moving averages, check for a recent cross of the 1-hour ma
	bool possible_sell = (price < a && price < b && price < c);
	bool possible_buy = (price > a && price > b && price > c);
	if (possible_sell || possible_buy) {
		int one_third = checking_width / 3;
		// check for sell: signal if price only crossed 1-hour 60-ma recently
		bool first_third_valid = true;
		// first 1/3rd loop
		for (int i = 0; i < one_third; i++) {
			double cprice = *(close_prices + index - checking_width + i + 1);
			double ma = a_ma[a_index - checking_width + i + 1];

			double price_above_ma_by = cprice - ma;
			double price_below_ma_by = ma - cprice;

			if ((possible_sell && price_above_ma_by < crossing_strength_price) || (possible_buy && price_below_ma_by < crossing_strength_price)) {
				first_third_valid = false;
				break;
			}
		}
		// last 1/3rd loop
		bool last_third_valid = true;
		for (int i = 0; i < one_third; i++) {
			double cprice = *(close_prices + index - i);
			double ma = a_ma[a_index - -i];

			double price_above_ma_by = cprice - ma;
			double price_below_ma_by = ma - cprice;

			if ((possible_sell && price_below_ma_by < crossing_strength_price) || (possible_buy && price_above_ma_by < crossing_strength_price)) {
				last_third_valid = false;
				break;
			}
		}
		// if the first third and last third are both valid, then we return the buy/sell signal
		if (first_third_valid && last_third_valid) {
			return possible_sell ? SELL : BUY;
		}
	}
	return NEUTRAL;
}

string ThreeDucks::get_desc() {
	return "ThreeDucks(1h,4h,1d,XS" + to_string(crossing_strength_pips) + ",CW" + to_string(checking_width) + ")";
}
