//
// Created by hugo on 29/04/2026.
//

#include "OFI.h"

OFI::OFI(const string &instrument, const string &exchange):MarketSignal(fmt::format("OFI({};{}",instrument,exchange),instrument,exchange,3) {}


