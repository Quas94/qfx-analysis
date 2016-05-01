#pragma once

#include "ReverseIndicator.hpp"

ReverseIndicator::ReverseIndicator(AbstractIndicator *internal_indicator) :
	AbstractIndicator(NULL), internal_indicator(internal_indicator) {
	// everything relevant has been uniform initialised, nothing else to do here
}

ReverseIndicator::~ReverseIndicator() {
	delete internal_indicator;
}

// runs this TA tool
void ReverseIndicator::process() {
	internal_indicator->process();
}

// checks what this TA tool is signalling at the given candlestick
Signal ReverseIndicator::get_signal(int index) {
	Signal signal = internal_indicator->get_signal(index);
	if (signal == BUY) return SELL;
	if (signal == SELL) return BUY;
	// only flip buy and sell, leave the other results untouched
	return signal;
}

// prints out a string description of the settings of this indicator
string ReverseIndicator::get_desc() {
	return "!!!REVERSED!!! " + internal_indicator->get_desc();
}
