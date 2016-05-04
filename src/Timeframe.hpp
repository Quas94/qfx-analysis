#pragma once

// valid time frames
enum Timeframe {
	// Minute = 1,
	// QuarterHour = Minute * 15,
	// HalfHour = Minute * 30,
	Hour,
	FourHour,
	Day,
	Week
};

// calculates the number of candlebars that will be generated with the
// given number of years and timeframe per candlebar
// errs on the side of too large (using a little extra memory is fine,
// whilst erring on the side of too small would be disastrous)
const int max_candle_bars(int start_year, int end_year, Timeframe tf);
