#pragma once

#include "ta_libc.h"
#include "AbstractIndicator.hpp"
#include "Parser.hpp"

class Stochastic : public AbstractIndicator {
private:

	int threshold_buy;
	int threshold_sell;

	int fastk_period;
	int slowk_period;
	int slowd_period;
	TA_MAType slowk_ma_type;
	TA_MAType slowd_ma_type;

	int out_begin;
	int out_num;

	// one-hour
	double *out_slowk;
	double *out_slowd;
	// four-hour
	double *fhour_out_slowk;
	double *fhour_out_slowd;

	const bool multiple_timeframes;

	pair<double, double> calc_fhour_stoch(int index);

public:
	Stochastic(Parser *parser, int buy_threshold = 20, int sell_threshold = 80,
		int fastk = 14, int slowk = 3, int slowd = 3, bool multiple_timeframes = false);

	~Stochastic();

	void process();

	Signal get_signal(int index);

	string get_desc();
};
