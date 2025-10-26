#ifndef _EOBI_COMMON_H_
#define _EOBI_COMMON_H_

#include "eobi_messages.h"

namespace ns {

using SecurityIdT = int64_t;
using MarketSegmentIdT = int32_t;
using ID = MarketSegmentIdT;
using MsgSeqNumT = uint32_t;
using LastMsgSeqNumT = MsgSeqNumT;


inline bool IsValid(const int64_t value) {
    return value != NO_VALUE_SLONG;
}

inline bool IsValid(const uint32_t value) {
    return value != NO_VALUE_UINT;
}

inline bool IsValid(const int32_t value) {
    return value != NO_VALUE_SINT;
}

inline std::string GetSideAsString(const uint8_t value) {
    switch(value) {
    case 1: return "Bid";
    case 2: return "Ask";
    default:
        assert(!"GetSideAsString() - unhandled value");
        return "Invalid";
    }
}

inline MarketBookSide GetSide(const uint8_t value) {
    switch(value) {
    case 1: return MarketBookSide::Bid;
    case 2: return MarketBookSide::Ask;
    default:
        assert(!"GetSide() - unhandled value");
        return MarketBookSide::Invalid;
    }
}

inline HitOrTake::Value GetHitOrTake(const uint8_t value) {
    switch(value) {
    case 1: return HitOrTake::Take;
    case 2: return HitOrTake::Hit;
    default:
        assert(!"GetHitOrTake() - unhandled value");
        return HitOrTake::None;
    }
}

}//end namespace

#endif