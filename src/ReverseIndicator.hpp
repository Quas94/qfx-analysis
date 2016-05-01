#pragma once

#include "AbstractIndicator.hpp"
#include "Parser.hpp"

// class which wraps around another indicator and reverses its signals
class ReverseIndicator : public AbstractIndicator {
private:
	AbstractIndicator *internal_indicator;

public:
	// default constructor
	ReverseIndicator(AbstractIndicator *internal_indicator);

	~ReverseIndicator();

	// runs this TA tool
	void process();

	// checks what this TA tool is signalling at the given candlestick
	Signal get_signal(int index);

	// prints out a string description of the settings of this indicator
	string get_desc();
};
