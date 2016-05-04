#include <vector>
#include <string>

#include "SimpleDate.hpp"
#include "Util.hpp"
#include "Constants.hpp"

using namespace std;

SimpleDate::SimpleDate() : SimpleDate(0, 0, 0) {
	// do nothing
}

SimpleDate::SimpleDate(int year, int month, int day) : year(year), month(month), day(day) {
	// all fields uniform initialised
}

SimpleDate::SimpleDate(const string &line) {
	vector<string> line_date_split = Util::split(line, PERIOD);
	year = stoi(line_date_split[0]);
	month = stoi(line_date_split[1]);
	day = stoi(line_date_split[2]);
}

// copy constructor: default
SimpleDate::SimpleDate(const SimpleDate &from) = default;

int SimpleDate::get_year() const {
	return year;
}

int SimpleDate::get_month() const {
	return month;
}

int SimpleDate::get_day() const {
	return day;
}

string SimpleDate::to_string() const {
	return std::to_string(year) + "-" + std::to_string(month) + "-" + std::to_string(day);
}

bool operator==(const SimpleDate &a, const SimpleDate &b) {
	return a.year == b.year && a.month == b.month && a.day == b.day;
}

bool operator>(const SimpleDate &a, const SimpleDate &b) {
	if (a.year > b.year)
		return true;
	if (a.year == b.year && a.month > b.month)
		return true;
	if (a.year == b.year && a.month == b.month && a.day > b.day)
		return true;

	return false;
}

bool operator>=(const SimpleDate &a, const SimpleDate &b) {
	return a > b || a == b;
}

bool operator<(const SimpleDate &a, const SimpleDate &b) {
	return b > a;
}

bool operator<=(const SimpleDate &a, const SimpleDate &b) {
	return b > a || b == a;
}
