#pragma once

#include <string>

using namespace std;

class SimpleDate {
private:
	int year;
	int month;
	int day;

public:

	SimpleDate();
	SimpleDate(int year, int month, int day);
	SimpleDate(const string &line);
	SimpleDate(const SimpleDate &from);

	int get_year() const;
	int get_month() const;
	int get_day() const;

	friend bool operator==(const SimpleDate &a, const SimpleDate &b);
	friend bool operator>(const SimpleDate &a, const SimpleDate &b);
	friend bool operator>=(const SimpleDate &a, const SimpleDate &b);
	friend bool operator<(const SimpleDate &a, const SimpleDate &b);
	friend bool operator<=(const SimpleDate &a, const SimpleDate &b);
};
