#include "ta_libc.h"
#include "MovingAverageTrend.hpp"

MovingAverageTrend::MovingAverageTrend(Parser *parser, bool simple_ma, int period_a, int period_b, int period_c) :
	AbstractIndicator(parser), simple_ma(simple_ma), period_a(period_a), period_b(period_b), period_c(period_c) {

	if (period_a >= period_b || period_b >= period_c) {
		throw runtime_error("periods must be incremental, ie. a < b < c");
	}
	
	int max_candles = parser->get_max_candles();
	this->a_ma = new double[max_candles];
	this->b_ma = new double[max_candles];
	this->c_ma = new double[max_candles];
}

MovingAverageTrend::~MovingAverageTrend() {
	delete a_ma;
	delete b_ma;
	delete c_ma;
}

void MovingAverageTrend::process() {
	const double *close_prices = parser->get_close_prices();
	int num_candles = parser->get_num_candles();

	TA_RetCode ret_code_a = TA_MA(0, num_candles - 1, close_prices, period_a,
		simple_ma ? TA_MAType_SMA : TA_MAType_EMA, &a_begin, &a_num, a_ma);
	TA_RetCode ret_code_b = TA_MA(0, num_candles - 1, close_prices, period_b,
		simple_ma ? TA_MAType_SMA : TA_MAType_EMA, &b_begin, &b_num, b_ma);
	TA_RetCode ret_code_c = TA_MA(0, num_candles - 1, close_prices, period_c,
		simple_ma ? TA_MAType_SMA : TA_MAType_EMA, &c_begin, &c_num, c_ma);
}

Signal MovingAverageTrend::get_signal(int index) {
	int a_index = index - a_begin;
	int b_index = index - b_begin;
	int c_index = index - c_begin;
	if (a_index < 0 || b_index < 0 || c_index < 0) {
		return NOT_APPLICABLE;
	}

	double a = a_ma[a_index];
	double b = b_ma[b_index];
	double c = c_ma[c_index];
	if (a > b && b > c)
		return BUY;
	if (a < b && b < c)
		return SELL;

	return NEUTRAL;
}

string MovingAverageTrend::get_desc() {
	return ((simple_ma ? "SMA (" : "EMA (") + to_string(period_a) + "," + to_string(period_b) + "," +
		to_string(period_c) + ")");
}
