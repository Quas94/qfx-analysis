#include <iostream>
#include <sstream>
#include <vector>

#include "Constants.hpp"
#include "Parser.hpp"

using namespace std;

Parser::Parser(string currency_pair, SimpleDate start_date, SimpleDate end_date, Timeframe timeframe) :
timeframe(timeframe), currency_pair(currency_pair), start_date(start_date), end_date(end_date),
current_date(start_date), num_candles(0) {

	if (start_date > end_date) throw runtime_error("start_date can't be later than end_date");
	// @TODO higher than 1 hour timeframes
	max_candles = max_candle_bars(start_date.get_year(), end_date.get_year(), timeframe);
	open_prices = new double[max_candles];
	high_prices = new double[max_candles];
	low_prices = new double[max_candles];
	close_prices = new double[max_candles];
	date_tracker = new SimpleDate[max_candles];
}

Parser::~Parser() {
	delete open_prices;
	delete high_prices;
	delete low_prices;
	delete close_prices;
	delete date_tracker;
}

string Parser::get_currency_pair() const {
	return currency_pair;
}

void Parser::create_candle(double open, double high, double low, double close, const SimpleDate &current_date) {
	// add to prices
	open_prices[num_candles] = open;
	high_prices[num_candles] = high;
	low_prices[num_candles] = low;
	close_prices[num_candles] = close;
	// add to month tracker
	date_tracker[num_candles] = current_date;
	// increment candle count
	num_candles++;
}

int Parser::get_num_candles() const {
	return num_candles;
}

int Parser::get_max_candles() const {
	return max_candles;
}

const double * Parser::get_open_prices() const {
	return open_prices;
}

const double * Parser::get_high_prices() const {
	return high_prices;
}

const double * Parser::get_low_prices() const {
	return low_prices;
}

const double * Parser::get_close_prices() const {
	return close_prices;
}

const SimpleDate * Parser::get_date_tracker() const {
	return date_tracker;
}

void Parser::parse() {
	// open the input file
	string file_path;
	file_path = "./" + currency_pair + ".csv";
	ifs.open(file_path, fstream::in);
	if (ifs.fail()) {
		string message = "Failed to open data file for " + currency_pair;
		cout << message << endl;
		throw runtime_error(message);
	}
	// read in all lines
	string line;
	while (getline(ifs, line)) {
		// skip empty lines
		if (line.size() == 0) continue;

		stringstream ss(line);

		// fetch date
		string line_date_raw;
		getline(ss, line_date_raw, COMMA);

		// determine if the date we're at is between start_date and end_date
		current_date = SimpleDate(line_date_raw);
		if (current_date < start_date) continue;
		if (current_date > end_date) return;

		// fetch hour and minute
		string time_raw;
		getline(ss, time_raw, COMMA);
		vector<string> time_split = Util::split(time_raw, COLON);
		int hour = stoi(time_split[0]);
		// ignore minute (ie. time_split[1])

		// fetch open, high, low and close
		string line_open_raw, line_high_raw, line_low_raw, line_close_raw;
		getline(ss, line_open_raw, COMMA);
		getline(ss, line_high_raw, COMMA);
		getline(ss, line_low_raw, COMMA);
		getline(ss, line_close_raw, COMMA);

		// last value is "volume" but we don't care about that

		double open = stod(line_open_raw);
		double high = stod(line_high_raw);
		double low = stod(line_low_raw);
		double close = stod(line_close_raw);
		create_candle(open, high, low, close, current_date);
	}
}
