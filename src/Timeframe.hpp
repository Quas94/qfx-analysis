#pragma once

// valid time frames
enum Timeframe {
	Minute = 1,
	QuarterHour = Minute * 15,
	HalfHour = Minute * 30,
	Hour = Minute * 60,
	FourHour = Hour * 4,
	Day = Hour * 24,
	Week = Day * 5, // weekends excluded
};

// calculates the number of candlebars that will be generated with the
// given number of years and timeframe per candlebar
// errs on the side of too large (using a little extra memory is fine,
// whilst erring on the side of too small would be disastrous)
const int max_candle_bars(int start_year, int end_year, Timeframe tf);
