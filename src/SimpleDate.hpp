#pragma once

#include <string>

using namespace std;

// date class represented in the format of year, month, week (week of month, from 1 to 5 inclusive)
class SimpleDate {
public:
	static const int WEEK_FIRST = 1;
	static const int WEEK_LAST = 5;

	int year;
	int month;
	int week;

	// constructor
	SimpleDate(int year, int month, int week);

	// copy constructor
	SimpleDate(const SimpleDate &copy);

	// returns string representation of this object
	string to_string();

	// post-increment operator overload, increments the date by a week
	SimpleDate operator++(int);

	// greater than operator overload
	friend bool operator>(const SimpleDate &a, const SimpleDate &b);

	// converts month to string
	static string monthToString(int month);
};

// valid time frames
enum Timeframe {
	Minute,
	QuarterHour,
	HalfHour,
	Hour,
	FourHour,
	Day,
	Week,
	Month
};
