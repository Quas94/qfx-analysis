#include <stdexcept>
#include <string>

#include "Timeframe.hpp"

using namespace std;

const int max_candle_bars(int start_year, int end_year, Timeframe tf) {
	int num_years = end_year - start_year + 1; // +1 since end_year is inclusive
											   // work out the answer for timeframe of 1 day
	int answer_day = 365 * num_years;
	switch (tf) {
	case Week:
		return answer_day / 5;
	case Day:
		return answer_day;
	case FourHour:
		return answer_day * 6;
	case Hour:
		return answer_day * 24;
	case HalfHour:
		return answer_day * 48;
	case QuarterHour:
		return answer_day * 96;
	case Minute:
		return answer_day * 1440;
	default:
		throw runtime_error("Invalid timeframe: " + to_string(tf));
	}
}
