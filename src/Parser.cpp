#include <iostream>
#include <sstream>
#include <vector>

#include "Constants.hpp"
#include "Util.hpp"
#include "Parser.hpp"
#include "SimpleDate.hpp"

using namespace std;

Parser::Parser(SimpleDate &start_date, SimpleDate &end_date, Timeframe timeframe) : start_date(start_date), end_date(end_date),
	current_date(start_date), timeframe(timeframe), num_candles(0), current_hour(NONE) {
	if (timeframe != Hour) {
		throw runtime_error("Only Timeframe.Hour is currently supported");
	}
	open_prices = new double[MAX_BARS];
	high_prices = new double[MAX_BARS];
	low_prices = new double[MAX_BARS];
	close_prices = new double[MAX_BARS];
}

Parser::~Parser() {
	delete open_prices;
	delete high_prices;
	delete low_prices;
	delete close_prices;
}

void Parser::create_candle(double open, double high, double low, double close) {
	// tries to create a candle with the given inputs, if current_hour isn't equivalent to NONE
	if (current_hour != NONE) {
		// add to prices
		open_prices[num_candles] = open;
		high_prices[num_candles] = high;
		low_prices[num_candles] = low;
		close_prices[num_candles] = close;
		// increment candle count
		num_candles++;
	}
}

int Parser::get_num_candles() const {
	return num_candles;
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
bool Parser::next_week() {
	// close and clear ifs, if open
	if (ifs.is_open()) {
		ifs.close();
		ifs.clear();
	}

	// check to see if we're finished or not
	if (current_date > end_date) {
		return false;
	}

	// proceed to open the next week
	string file_path;
	do {
		if (ifs.fail()) {
			if (current_date.week == SimpleDate::WEEK_FIRST || current_date.week == SimpleDate::WEEK_LAST) {
				// sometimes, week 1 or week 5 are missing from certain months
				current_date++;
			} else {
				throw runtime_error("Error: opening file failed: " + file_path);
			}
		}
		file_path = "./" + to_string(current_date.year) + "/" + SimpleDate::monthToString(current_date.month) +
			"/" + CURRENT_PAIR + "_Week" + to_string(current_date.week) + ".csv";
		ifs.open(file_path, fstream::in);
	} while (ifs.fail());

	cout << "Opened successfully: " << file_path << endl;

	string line;
	ifs >> line;
	if (line != FIRST_LINE) {
		throw runtime_error("Error: first line of input invalid for date: " + current_date.to_string());
	}

	// lastly, increment date for next call
	current_date++;
	return true;
}

void Parser::parse() {
	while (next_week()) {
		string line;
		double open = NONE;
		double high = NONE;
		double low = NONE;
		// don't need a close variable - it's just the price when we create the candle
		double avg_price;

		int line_no = 0;
		// read in all lines
		while (getline(ifs, line)) {
			line_no++;
			// skip empty lines
			if (line.size() == 0) continue;

			stringstream ss(line);
			string part;

			// discard the first three comma-separated blocks: lTid, dealable and currency pair
			for (int i = 0; i < 3; i++) {
				getline(ss, part, COMMA);
			}

			// fetch timestamp
			string timestamp_raw;
			getline(ss, timestamp_raw, COMMA);

			// fetch bid and ask prices
			string bid_raw;
			string ask_raw;
			getline(ss, bid_raw, COMMA);
			getline(ss, ask_raw, COMMA);

			// convert raw to appropriate data types
			vector<string> timestamp_split = split(timestamp_raw, SPACE);

			string date = timestamp_split[0];
			string time = timestamp_split[1];
			vector<string> date_split = split(date, DASH);
			vector<string> time_split = split(time, COLON);

			int year = stoi(date_split[0]);
			int month = stoi(date_split[1]);
			int day = stoi(date_split[2]);

			int hour = stoi(time_split[0]);
			int minute = stoi(time_split[1]);
			int second = (int) round(stod(time_split[2]));

			double bid = stod(bid_raw);
			double ask = stod(ask_raw);
			// @TODO: track bid and ask separately, maybe? spread shouldn't be a big issue though
			avg_price = (bid + ask) / 2;

			// @TODO: implement multiple timeframe support, currently only hours
			if (hour != current_hour) {
				create_candle(open, high, low, avg_price);

				// check for missing hours
				int diff = abs(hour - current_hour);
				if (current_hour != NONE && diff > 1 && (hour != 0 || current_hour != 23) && line_no > 2) {
					string msg = "Warning: Missing hour(s)! hours diff = " + to_string(diff) + ", hour = " + to_string(hour) +
						", cur_hour = " + to_string(current_hour) + ". date = " + date + " (line " + to_string(line_no) + ")";
					// throw runtime_error(msg);
					cout << msg << endl;
				}

				// reset OHLC and current_hour
				current_hour = NONE;
				open = NONE;
				high = NONE;
				low = NONE;
			}

			// update current values
			if (current_hour == NONE) {
				// just overwrite, since new hour
				current_hour = hour;
				open = avg_price;
				high = avg_price;
				low = avg_price;
			}
			else {
				if (avg_price > high) high = avg_price;
				if (avg_price < low) low = avg_price;
			}
		}
		// add the last hour
		create_candle(open, high, low, avg_price);
	}
}
