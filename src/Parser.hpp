#pragma once

#include <fstream>

#include "SimpleDate.hpp"

using namespace std;

// takes in a filestream and timeframe
class Parser {
private:
	ifstream ifs;

	const SimpleDate &start_date;
	const SimpleDate &end_date;
	SimpleDate current_date; // will be copy-constructed from start_date in uniform initialisation

	Timeframe timeframe;
	int num_candles;
	double *open_prices;
	double *high_prices;
	double *low_prices;
	double *close_prices;

	// hour that we've parsed up to
	int current_hour;

	const int NONE = -1;

public:

	Parser(SimpleDate &start_date, SimpleDate &end_date, Timeframe timeframe);

	~Parser();

	void create_candle(double open, double high, double low, double close);

	int get_num_candles() const;

	const double * get_open_prices() const;
	const double * get_high_prices() const;
	const double * get_low_prices() const;
	const double * get_close_prices() const;

	/**
	* Checks if the next file should be opened, and if so, opens it, and returns true.
	* If the next file should not be opened (ie. we're past the end_date), this function returns false.
	*/
	bool next_week();

	void parse();
};
