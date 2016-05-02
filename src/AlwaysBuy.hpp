#pragma once

#include "ta_libc.h"
#include "AbstractIndicator.hpp"
#include "Parser.hpp"

class AlwaysBuy : public AbstractIndicator {
private:

public:
	AlwaysBuy(Parser *parser);

	void process();

	Signal get_signal(int index);

	string get_desc();
};
