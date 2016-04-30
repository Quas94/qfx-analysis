#pragma once
#include <string>

using namespace std;

const char COMMA = ',';
const char SPACE = ' ';
const char DASH = '-';
const char COLON = ':';
const string FIRST_LINE = "lTid,cDealable,CurrencyPair,RateDateTime,RateBid,RateAsk";

const string CURRENT_PAIR = "EUR_USD";

// maximum number of bars predicted for one currency pair, on a 1 hour timescale
const int MAX_BARS = 20000;

const double PIP_PRICE_RATIO = 10000.0;
