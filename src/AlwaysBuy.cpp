#include "AlwaysBuy.hpp"

AlwaysBuy::AlwaysBuy(Parser *parser) : AbstractIndicator(parser) {
	// do nothing
}

void AlwaysBuy::process() {
	// do nothing
}

// always return buy no matter what
Signal AlwaysBuy::get_signal(int index) {
	return BUY;
}

string AlwaysBuy::get_desc() {
	return "ALWAYS BUY";
}
