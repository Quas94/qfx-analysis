#include <iostream>
#include <sstream>
#include <vector>

#include "Constants.hpp"
#include "Parser.hpp"
#include "Timeframe.hpp"
#include "SimpleDate.hpp"

using namespace std;

Parser::Parser(SimpleDate start_date, SimpleDate end_date, Timeframe timeframe) : start_date(start_date), end_date(end_date),
current_date(start_date), current_file_year(start_date.get_year()), timeframe(timeframe), num_candles(0) {
	if (start_date > end_date) throw runtime_error("end_date can't be earlier than start_date");
	max_candles = max_candle_bars(start_date.get_year(), end_date.get_year(), timeframe);
	open_prices = new double[max_candles];
	high_prices = new double[max_candles];
	low_prices = new double[max_candles];
	close_prices = new double[max_candles];
}

Parser::~Parser() {
	delete open_prices;
	delete high_prices;
	delete low_prices;
	delete close_prices;
}

void Parser::create_candle(double open, double high, double low, double close) {
	// add to prices
	open_prices[num_candles] = open;
	high_prices[num_candles] = high;
	low_prices[num_candles] = low;
	close_prices[num_candles] = close;
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

/**
* Checks if the next file should be opened, and if so, opens it, and returns true.
* If the next file should not be opened (ie. we're past the end_date), this function returns false.
*/
bool Parser::next_year() {
	// close and clear ifs, if open
	if (ifs.is_open()) {
		ifs.close();
		ifs.clear();
	}

	// check to see if we're finished or not
	if (current_date > end_date) {
		return false;
	}

	// proceed to open the next year
	string file_path;
	string current_year = to_string(current_file_year);
	file_path = "./" + current_year + ".csv";
	ifs.open(file_path, fstream::in);
	if (ifs.fail()) {
		string message = "Failed to open data file for " + current_year;
		cout << message << endl;
		throw runtime_error(message);
	}

	cout << "Opened successfully: " << file_path << endl;

	// lastly, increment year for next call
	current_file_year++;
	return true;
}

void Parser::parse() {

	double candle_open = NONE;
	double candle_high = NONE;
	double candle_low = NONE;
	double candle_close = NONE;

	string line;
	int lines_read = 0;
	while (next_year()) {
		while (!ifs.eof()) {
			// read in all lines
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

				// only increment lines_read counter if we actually process this line
				lines_read++;

				// fetch hour and minute
				string hour_raw, minute_raw;
				getline(ss, hour_raw, COMMA);
				getline(ss, minute_raw, COMMA);

				// fetch open, high, low and close
				string line_open_raw, line_high_raw, line_low_raw, line_close_raw;
				getline(ss, line_open_raw, COMMA);
				getline(ss, line_high_raw, COMMA);
				getline(ss, line_low_raw, COMMA);
				getline(ss, line_close_raw, COMMA);

				double line_open = stod(line_open_raw);
				double line_high = stod(line_high_raw);
				double line_low = stod(line_low_raw);
				double line_close = stod(line_close_raw);

				if (candle_open == NONE) { // lines_read should be 0 here
					if (lines_read != 1)
						throw runtime_error("candle_open == NONE when lines_read is " + to_string(lines_read));

					if (stoi(minute_raw) != 0) {
						cout << "Candle open starting on non-zero: " << hour_raw << ":" << minute_raw << endl;
					}

					// first line of this candle, so just overwrite
					candle_open = line_open;
					candle_high = line_high;
					candle_low = line_low;
				} else {
					// otherwise we have to check if it's more extreme before overwriting
					if (candle_high < line_high)
						candle_high = line_high;
					if (candle_low > line_low)
						candle_low = line_low;

					// check if we've read in enough lines to constitute a candle
					if (lines_read >= timeframe) {
						candle_close = line_close;
						break;
					}
				}
			}

			if (lines_read >= timeframe) {
				if (lines_read > timeframe)
					throw runtime_error("lines_read (" + to_string(lines_read) + ") > timeframe ("
						+ to_string(timeframe) + ") somehow");
				// add the candle
				create_candle(candle_open, candle_high, candle_low, candle_close);
				// reset flags
				lines_read = 0;
				candle_open = NONE;
				candle_high = NONE;
				candle_low = NONE;
				candle_close = NONE;
			}
		}
	}
}
