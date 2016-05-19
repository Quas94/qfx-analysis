#include <iostream>
#include <algorithm>

#include "Stochastic.hpp"
#include "Constants.hpp"

using namespace std;

Stochastic::Stochastic(Parser *parser, int buy_threshold, int sell_threshold,
	int fastk, int slowk, int slowd, bool multiple_timeframes) : AbstractIndicator(parser), multiple_timeframes(multiple_timeframes) {

	// @TODO be more memory efficient with array sizing
	int max_candles = parser->get_max_candles();
	this->out_slowk = new double[max_candles];
	this->out_slowd = new double[max_candles];
	if (multiple_timeframes) {
		this->fhour_out_slowd = new double[max_candles];
		this->fhour_out_slowk = new double[max_candles];
	}

	// @TODO allow for customisation of stochastic interpretation settings
	this->threshold_buy = buy_threshold; // typically 20
	this->threshold_sell = sell_threshold; // typically 80
	// @TODO allow for customisation of stochastic technical settings
	this->fastk_period = fastk;
	this->slowk_period = slowk;
	this->slowk_ma_type = TA_MAType_SMA;
	this->slowd_period = slowd;
	this->slowd_ma_type = TA_MAType_SMA;
}

Stochastic::~Stochastic() {
	delete out_slowk;
	delete out_slowd;
	if (multiple_timeframes) {
		delete fhour_out_slowd;
		delete fhour_out_slowk;
	}
}

// returns a pair of <slowd, slowk> for stoch on the four-hour chart
pair<double, double> Stochastic::calc_fhour_stoch(int index) {
	if (!multiple_timeframes) {
		cout << "Stochastic::calc_fhour_stoch called when multiple_timeframes is not enabled" << endl;
		throw runtime_error("multiple_timeframes error");
	}

	// calculate if the index is high enough to calculate a stoch for the 4-hour chart
	int lookback = TA_STOCH_Lookback(fastk_period, slowk_period, slowk_ma_type, slowd_period, slowd_ma_type) + 1;
	int num_fhour_bars = (int) ceil((index + 1) / 4.0); // number of 4-hour bars from the very beginning, this value is only used to check
	if (num_fhour_bars < lookback)
		return make_pair(50, 50); // return a completely neutral signal because not applicable for 4-hour chart

	const double *high_prices = parser->get_high_prices();
	const double *low_prices = parser->get_low_prices();
	const double *close_prices = parser->get_close_prices();

	// construct arrays for 4-hour candles
	// they only need to be of size 'lookback' since we're only trying to find the output of one 4-hour bar
	double *fhour_high_prices = new double[lookback];
	double *fhour_low_prices = new double[lookback];
	double *fhour_close_prices = new double[lookback];
	double temp_out_slowk[1];
	double temp_out_slowd[1];

	// check if the num of 1-hour bars is divisible by 4
	int remainder = (index + 1) % 4;
	if (remainder == 0)
		remainder = 4;
	int f = index - (((lookback - 1) * 4) + remainder) + 1;
	//cout << "index = " << index << ", lookback - 1 = " << (lookback - 1) << ", remainder = " << remainder << endl;
	//cout << "called 4hour_stoch(" << index << "), ceil'd " << ((index + 1) / 4.0) << ", f begins = " << f << endl;
	int fh_candle_count = 0;
	while (f <= index) { // f = index of the start of the 4-hour candle, on the 1-hour arrays
		// figure out how many candles we should use 
		int num_hour_candles_to_combine = min(index - f + 1, 4); // 4 x 1-hour candles, unless we have less than 4 remaining
		//cout << "num hour candles to combine = " << num_hour_candles_to_combine << ", f = " << f << ", index = " << index << endl;

		// 4-hour high, low and close
		double fh_high = 0;
		double fh_low = numeric_limits<double>::max();
		double fh_close;
		for (int h = 0; h < num_hour_candles_to_combine; h++) { // the 1-hour candles that comprise this 4-hour candle will be f+h
			int i = f + h;
			double high = *(high_prices + i);
			double low = *(low_prices + i);
			if (high > fh_high)
				fh_high = high;
			if (low < fh_low)
				fh_low = low;
			fh_close = *(close_prices + i);
		}
		fhour_high_prices[fh_candle_count] = fh_high;
		fhour_low_prices[fh_candle_count] = fh_low;
		fhour_close_prices[fh_candle_count] = fh_close;
		// increment four hour candle count
		fh_candle_count++;
		f += 4;
	}

	if (fh_candle_count != lookback) {
		cout << "fh_candle_count = " << fh_candle_count << ", lookback = " << lookback << endl;
		throw runtime_error("error with four hour stochastic");
	}

	// throw the fhour_arrays into the talib stochastic function
	int fh_out_begin;
	int fh_num_elements;
	//cout << "calling TA_STOCH(0, " << (fh_candle_count - 1) << ", fhp, flp, fcp, " << fastk_period << ", " << slowk_period << ", matype, " <<
	//	slowd_period << ", matype" << endl;

	TA_RetCode ret_code = TA_STOCH(0, fh_candle_count - 1, fhour_high_prices, fhour_low_prices, fhour_close_prices, fastk_period, slowk_period,
		slowk_ma_type, slowd_period, slowd_ma_type, &fh_out_begin, &fh_num_elements, temp_out_slowk, temp_out_slowd);

	//cout << "TA_RetCode = " << ret_code << endl;

	if (fh_num_elements != 1 || fh_out_begin != lookback - 1) {
		cout << "fh_num_elements supposed to be 1, is " << fh_num_elements << endl;
		cout << "fh_out_begin supposed to be " << (lookback - 1) << ", is " << fh_out_begin << endl;
		throw runtime_error("error with four hour stochastic (2)");
	}

	delete fhour_high_prices;
	delete fhour_low_prices;
	delete fhour_close_prices;

	return make_pair(temp_out_slowd[0], temp_out_slowk[0]);
}

void Stochastic::process() {
	// we are up to bar 'index'
	const double *high_prices = parser->get_high_prices();
	const double *low_prices = parser->get_low_prices();
	const double *close_prices = parser->get_close_prices();
	int num_candles = parser->get_num_candles();
	// cout << "Called Stochastic::process()" << endl;

	// calculate stochastic for entire block
	TA_RetCode ret_code = TA_STOCH(0, num_candles - 1, high_prices, low_prices, close_prices,
		fastk_period, slowk_period, slowk_ma_type, slowd_period, slowd_ma_type, &out_begin,
		&out_num, out_slowk, out_slowd);

	// calculate 4-hour stochastic
	if (multiple_timeframes) {
		for (int i = out_begin; i < num_candles; i++) {
			// cout << "calling calc_fhour_stoch(" << i << ")" << endl;
			pair<double, double> fhour_results = calc_fhour_stoch(i);
			if (fhour_results.first < 0 || fhour_results.second < 0 || fhour_results.first > 100 || fhour_results.second > 100) {
				cout << "ERROR: fhour results out of bounds... first = " << fhour_results.first << ", second = " << fhour_results.second << endl;
				throw runtime_error("blah");
			}
			// cout << "returned (" << fhour_results.first << ", " << fhour_results.second << "), out_begin = " << out_begin << endl;
			fhour_out_slowd[i - out_begin] = fhour_results.first;
			fhour_out_slowk[i - out_begin] = fhour_results.second;
		}
	}
}

Signal Stochastic::get_signal(int index) {
	// account for the starting point of this indicator
	index -= out_begin;
	if (index < 0) {
		return NOT_APPLICABLE;
	}
	// otherwise we can proceed
	double d = *(out_slowd + index);
	double k = *(out_slowk + index);
	// @TODO implement more complex stochastic analysis: use k, k/d crossovers, enter+exit overbought/oversold before deciding, etc.
	// @TODO do more research on why flipping the < and > appears to be so effective?!
	if (multiple_timeframes) {
		double fhd = *(fhour_out_slowd + index);
		double fhk = *(fhour_out_slowk + index);
		if (d <= threshold_buy && fhd <= threshold_buy)
			return BUY;
		if (d >= threshold_sell && fhd >= threshold_sell)
			return SELL;
	} else {
		if (d <= threshold_buy)
			return BUY;
		if (d >= threshold_sell)
			return SELL;
	}
	return NEUTRAL;
}

string Stochastic::get_desc() {
	return "STOCH(" + to_string(fastk_period) + "," + to_string(slowk_period) + "," + to_string(slowd_period) +
		"," + to_string(threshold_buy) + "," + to_string(threshold_sell) + ")";
}
