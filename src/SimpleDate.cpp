#include <string>

#include "SimpleDate.hpp"

using namespace std;

SimpleDate::SimpleDate(int year, int month, int week) : year(year), month(month), week(week) {
	// nothing to do here, all done in uniform initialisation
}

// default copy constructor
SimpleDate::SimpleDate(const SimpleDate &copy) = default;

string SimpleDate::to_string() {
	return std::to_string(year) + ", " + SimpleDate::monthToString(month) + ", week " + std::to_string(week);
}

// increments the date by 1 week
SimpleDate SimpleDate::operator++(int) {
	// save original value
	SimpleDate original(year, month, week);

	week++;
	if (week > WEEK_LAST) {
		week = WEEK_FIRST;
		month++;
		if (month > 12) {
			month = 1;
			year++;
		}
	}

	return original;
}

string SimpleDate::monthToString(int month) {
	switch (month) {
	case 1:
		return "Jan";
	case 2:
		return "Feb";
	case 3:
		return "Mar";
	case 4:
		return "Apr";
	case 5:
		return "May";
	case 6:
		return "Jun";
	case 7:
		return "Jul";
	case 8:
		return "Aug";
	case 9:
		return "Sep";
	case 10:
		return "Oct";
	case 11:
		return "Nov";
	case 12:
		return "Dec";
	default:
		throw runtime_error("Invalid month passed into SimpleDate::monthToString - " + std::to_string(month));
	}
}

bool operator>(const SimpleDate &a, const SimpleDate &b) {
	if (a.year > b.year) return true;
	if (a.year == b.year && a.month > b.month) return true;
	if (a.year == b.year && a.month == b.month && a.week > b.week) return true;

	return false;
}
