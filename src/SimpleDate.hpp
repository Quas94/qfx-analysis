#pragma once

#include <string>

using namespace std;

class SimpleDate {
private:
	int year;
	int month;
	int day;

public:

	SimpleDate(int year, int month, int day);
	SimpleDate(const string &line);
	SimpleDate(const SimpleDate &from);

	int get_year();
	int get_month();
	int get_day();

	friend bool operator==(const SimpleDate &a, const SimpleDate &b);
	friend bool operator>(const SimpleDate &a, const SimpleDate &b);
	friend bool operator>=(const SimpleDate &a, const SimpleDate &b);
	friend bool operator<(const SimpleDate &a, const SimpleDate &b);
	friend bool operator<=(const SimpleDate &a, const SimpleDate &b);
};
