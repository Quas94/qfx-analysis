#include "Util.hpp"

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
