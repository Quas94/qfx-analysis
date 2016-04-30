#pragma once

#include "Parser.hpp"

enum Signal {
	BUY,
	SELL,
	NEUTRAL, // neither buy nor sell indicated
	NOT_APPLICABLE, // TA was unable to calculate a decision for this index

	_NULL // placeholder for uninitialised
};

class AbstractIndicator {
protected:
	const Parser *parser;
	
public:
	// default constructor
	AbstractIndicator(Parser *parser);

	// runs this TA tool
	virtual void process() = 0;

	// checks what this TA tool is signalling at the given candlestick
	virtual Signal get_signal(int index) = 0;

	// prints out a string description of the settings of this indicator
	virtual string get_desc() = 0;
};
