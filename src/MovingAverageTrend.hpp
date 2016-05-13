#pragma once

#include "Parser.hpp"

// indicator of trend built off three moving averages with differing time periods
class MovingAverageTrend : AbstractIndicator {

private:
	bool simple_ma;

	const int period_a;
	const int period_b;
	const int period_c;

	double *a_ma, *b_ma, *c_ma;
	int a_begin, b_begin, c_begin;
	int a_num, b_num, c_num;

public:
	// default constructor
	MovingAverageTrend(Parser *parser, bool simple_ma, int period_a, int period_b, int period_c);

	~MovingAverageTrend();

	// runs this TA tool
	void process();

	// checks what this TA tool is signalling at the given candlestick
	Signal get_signal(int index);

	// prints out a string description of the settings of this indicator
	string get_desc();
};
