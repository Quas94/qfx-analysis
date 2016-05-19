#include <iostream>

#include "Util.hpp"
#include "Constants.hpp"

/**
* Contains helper functions for a variety of different things.
*/
using namespace std;

vector<string> & Util::split(const string &s, char delim, vector<string> &elems) {
	stringstream ss(s);
	string item;
	while (getline(ss, item, delim)) {
		elems.push_back(item);
	}
	return elems;
}

vector<string> Util::split(const string &s, char delim) {
	vector<string> elems;
	split(s, delim, elems);
	return elems;
}

double Util::get_pip_value(string pair) {
	vector<string> pair_split = split(pair, UNDERSCORE);
	if (pair_split.size() != 2) {
		cout << "Pair fed to Util::get_pip_value() isn't valid: " << pair << endl;
		throw runtime_error("Util error");
	}
	if (pair_split[0] == JPY || pair_split[1] == JPY)
		return 1.0 / PIP_PRICE_RATIO_JPY;

	return 1 / PIP_PRICE_RATIO;
}
