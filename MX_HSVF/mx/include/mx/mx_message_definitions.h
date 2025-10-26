#ifndef _MX_MESSAGE_DEFINITIONS_H_
#define _MX_MESSAGE_DEFINITIONS_H_

#include "mx_common.h"

namespace ns {

#pragma pack(push)
#pragma pack(1)

const std::string delimiter = ", ";


template<typename T>
T ConvertCharArray(const char* buf, int len) {
    T ret = 0;
    for(int i = len; i > 0; --i) {
        ret = ret * 10 + *buf - '0';
        ++buf;
    }
    return ret;
}

/** As per doc:
 
    If size is greater than 99999, the 5th character becomes an exponent.
    Examples:

    Data          						Message sent 									Participant should display...
    Bid size of 124,872 				Size field will indicate ‘1248C’				124,800
    Volume of 8,457,‘8457188’			Volume will indicate ‘8457188’					8,457,188
    Volume of 258,487,797				Volume will indicate ‘2584877C’					258,487,700
    Open Interest of 544,‘544871’		Size field will indicate ‘544871’ 				544,871
    Open Interest of 17,458,795			Size field will indicate ‘174587C’ 				17,458,700
*/
template<typename T>
T ConvertCharArrayWithLastByteCheck(const char* buf, int len) {
    const bool shouldMultiply = buf[len - 1] > '9';
    const uint32_t multiplier = shouldMultiply ? ns::GetMultiplierFromIndicatorCode(buf[len - 1]) : 1;
    T value = ConvertCharArray<T>(buf, shouldMultiply ? len - 1: len);
    return value * multiplier;
}

struct MsgHeader {
    char seqNum[10];
    char msgType[2];

    uint64_t GetSeqNum() const {
        return ConvertCharArray<uint64_t>(seqNum, 10);
    }
    std::string GetMsgType() const {
        return std::string(msgType, sizeof(msgType));
    }  

    std::string ToString() const {
        std::stringstream ss;
        ss << "Seq_num=" << std::string(seqNum, sizeof(seqNum))
        << ", Msg=" << std::string(msgType, sizeof(msgType))
        ;
        return ss.str();
    }
};

struct LongMsgHeader {
    char seqNum[10];
    char msgType[2];
    char timestamp[20];

    uint64_t GetSeqNum() const {
        return ConvertCharArray<uint64_t>(seqNum, 10);
    }
    std::string GetMsgType() const {
        return std::string(msgType, sizeof(msgType));
    }
    std::string GetTimestamp() const {
        return std::string(timestamp, sizeof(timestamp));
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << "Seq_num=" << std::string(seqNum, sizeof(seqNum))
        << ", Msg=" << std::string(msgType, sizeof(msgType))
        << ", Timestamp=" << std::string(timestamp, sizeof(timestamp))
        ;
        return ss.str();
    }
};


//Outbound msgs
//LI
struct Login {
    char STX;
    MsgHeader header;
    char username[16];
    char password[16];
    char timestamp[6];
    char protocol[2];
    char ETX;
};

//LO
struct Logout {
    char STX;
    MsgHeader header;
    char ETX;
};

//KI
struct LoginAcknowledgement {
    MsgHeader msgHeader;
};

//KO
struct LogoutAcknowledgement {
    MsgHeader msgHeader;
};

//RT
struct RetransmissionRequest {
    char STX;
    MsgHeader header;
    char line[2];
    char startMsgNumber[10];
    char endMsgNumber[10];
    char ETX;
};

//RB
struct RetransmissionBegin {
    MsgHeader msgHeader;
};

//RE
struct RestransmissionEnd {
    MsgHeader msgHeader;
};

//ER
struct ErrorMessage {
    MsgHeader msgHeader;
    char errorCode[4];
    char errorMsg[80];

    std::string GetErrorCode() const {
        return std::string(errorCode, sizeof(errorCode));
    }
    std::string GetErrorMsg() const {
        return std::string (errorMsg, sizeof(errorMsg));
    }
};


//Inbound msgs

struct DepthLevel {
    char level;
    char bidPrice[7];
    char bidPriceFractionIndicator;
    char bidSize[5];
    char bidOrdersNum[2];
    char askPrice[7];
    char askPriceFractionIndicator;
    char askSize[5];
    char askOrdersNum[2];

    int GetLevel() const            { return level - '1'; }
    int64_t GetBidPrice() const     { return ConvertCharArray<int64_t>(bidPrice, 7); }
    char GetBidPriceFI() const      { return bidPriceFractionIndicator; }
    int32_t GetBidSize() const      { return ConvertCharArrayWithLastByteCheck<int32_t>(bidSize, 5); }
    int32_t GetBidOrdersNum() const { return ConvertCharArrayWithLastByteCheck<int32_t>(bidOrdersNum, 2); }
    int64_t GetAskPrice() const     { return ConvertCharArray<int64_t>(askPrice, 7); }
    char GetAskPriceFI() const      { return askPriceFractionIndicator; }
    int32_t GetAskSize() const      { return ConvertCharArrayWithLastByteCheck<int32_t>(askSize, 5); }
    int32_t GetAskOrdersNum() const { return ConvertCharArrayWithLastByteCheck<int32_t>(askOrdersNum, 2); }

    std::string ToString(const std::string& id) const {
        std::stringstream ss;
        ss << "\nLevel=" << level
        << ", id=" << id
        << ", Bid price=" << std::string(bidPrice, 7)
        << ", Bid price fi=" << bidPriceFractionIndicator
        << ", Bid size=" << std::string(bidSize, 5)
        << ", Bid orders num=" << std::string(bidOrdersNum, 2)
        << ", Ask price=" << std::string(askPrice, 7)
        << ", Ask price fi=" << askPriceFractionIndicator
        << ", Ask size=" << std::string(askSize, 5)
        << ", Ask orders num=" << std::string(askOrdersNum, 2)
        ;
        return ss.str();
    }
};

struct StrategyDepthLevel {
    char level;
    char bidPriceSign;
    char bidPrice[7];
    char bidPriceFractionIndicator;
    char bidSize[5];
    char bidOrdersNum[2];
    char askPriceSign;
    char askPrice[7];
    char askPriceFractionIndicator;
    char askSize[5];
    char askOrdersNum[2];

    int GetLevel() const            { return level - '1'; }
    int64_t GetBidPrice() const { 
        const int64_t value = ConvertCharArray<int64_t>(bidPrice, 7);
        return bidPriceSign == '-' ? value * -1 : value;
    }
    char GetBidPriceFI() const      { return bidPriceFractionIndicator; }
    int32_t GetBidSize() const      { return ConvertCharArrayWithLastByteCheck<int32_t>(bidSize, 5); }
    int32_t GetBidOrdersNum() const { return ConvertCharArrayWithLastByteCheck<int32_t>(bidOrdersNum, 2); }
    int64_t GetAskPrice() const {
        const int64_t value = ConvertCharArray<int64_t>(askPrice, 7);
        return askPriceSign == '-' ? value * -1 : value;
    }
    char GetAskPriceFI() const      { return askPriceFractionIndicator; }
    int32_t GetAskSize() const      { return ConvertCharArrayWithLastByteCheck<int32_t>(askSize, 5); }
    int32_t GetAskOrdersNum() const { return ConvertCharArrayWithLastByteCheck<int32_t>(askOrdersNum, 2); }

    std::string ToString(const std::string& id) const {
        std::stringstream ss;
        ss << "\nLevel=" << level
        << ", id=" << id
        << ", Bid price sign=" << std::string(1, bidPriceSign)
        << ", Bid price=" << std::string(bidPrice, 7)
        << ", Bid price fi=" << bidPriceFractionIndicator
        << ", Bid size=" << std::string(bidSize, 5)
        << ", Bid orders num=" << std::string(bidOrdersNum, 5)
        << ", Ask price sign=" << std::string(1, askPriceSign)
        << ", Ask price=" << std::string(askPrice, 7)
        << ", Ask price fi=" << askPriceFractionIndicator
        << ", Ask size=" << std::string(askSize, 5)
        << ", Ask orders num=" << std::string(askOrdersNum, 2)
        ;
        return ss.str();
    }
};


//HF
struct FutureMarketDepth {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];
    char statusMarker;
    char numOfLevel;
    DepthLevel depthLevels[5];

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    char GetStatusMarker() const {
        return statusMarker;
    }
    int GetLevelsNum() const { 
        return numOfLevel - '0'; 
    }
    
    std::string ToString() const {
        const std::string id = GetIdentifier();
        
        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "rootSymbol=" << std::string(rootSymbol, sizeof(rootSymbol))
        << delimiter << "symbolMonth=" << symbolMonth
        << delimiter << "symbolYear=" << std::string(symbolYear, sizeof(symbolYear))
        << delimiter << "expiryDay=" << std::string(expiryDay, sizeof(expiryDay))
        << delimiter << "statusMarker=" << statusMarker
        << delimiter << "numOfLevel=" << (numOfLevel - '0')
        ;

        const int levels = GetLevelsNum();
        for (int i = 0; i < levels; ++i) {
            ss << depthLevels[i].ToString(id);
        }
        return ss.str();
    } 
};

//H
struct OptionMarketDepth {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char expiryMonth;
    char strikePrice[7];
    char strikePriceFractionIndicator;
    char expiryYear[2];
    char expiryDay[2];
    char statusMarker;
    char numOfLevel;
    DepthLevel depthLevels[5];

    std::string GetIdentifier() const {
        return  GetRootSymbol() 
                + GetExpiryMonth() 
                + GetStrikePrice() 
                + GetStrikePriceFI()
                + GetExpiryYear() 
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetExpiryMonth() const {
        return expiryMonth;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFI() const {
        return strikePriceFractionIndicator;
    }
    std::string GetExpiryYear() const {
        return std::string(expiryYear, sizeof(expiryYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    char GetStatusMarker() const {
        return statusMarker;
    }
    int GetLevelsNum() const { 
        return numOfLevel - '0'; 
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "rootSymbol=" << std::string(rootSymbol, sizeof(rootSymbol))
        << delimiter << "expiryMonth=" << expiryMonth
        << delimiter << "strikePrice=" << std::string(strikePrice, sizeof(strikePrice))
        << delimiter << "strikePriceFractionIndicator=" << strikePriceFractionIndicator
        << delimiter << "expiryYear=" << std::string(expiryYear, sizeof(expiryYear))
        << delimiter << "expiryDay=" << std::string(expiryDay, sizeof(expiryDay))
        << delimiter << "statusMarker=" << statusMarker
        << delimiter << "numOfLevel=" << (numOfLevel - '0')
        ;

        const int levels = GetLevelsNum();
        for (int i = 0; i < levels; ++i) {
            ss << depthLevels[i].ToString(id);
        }
        return ss.str();
    }
};

//HB
struct FutureOptionsMarketDepth {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];
    char callPut;
    char strikePrice[7];
    char strikePriceFractionIndicator;    
    char statusMarker;
    char numOfLevel;
    DepthLevel depthLevels[5];

    std::string GetIdentifier() const {
        return  GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                + GetCallPut()
                + GetStrikePrice()
                + GetStrikePriceFI()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    char GetCallPut() const {
        return callPut;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFI() const {
        return strikePriceFractionIndicator;
    }
    char GetStatusMarker() const {
        return statusMarker;
    }
    int GetLevelsNum() const { 
        return numOfLevel - '0'; 
    } 

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "rootSymbol=" << std::string(rootSymbol, sizeof(rootSymbol))
        << delimiter << "symbolMonth=" << symbolMonth
        << delimiter << "symbolYear=" << std::string(symbolYear, sizeof(symbolYear))
        << delimiter << "expiryDay=" << std::string(expiryDay, sizeof(expiryDay))
        << delimiter << "callPut=" << callPut
        << delimiter << "strikePrice=" << std::string(strikePrice, sizeof(strikePrice))
        << delimiter << "strikePriceFractionIndicator=" << strikePriceFractionIndicator
        << delimiter << "statusMarker=" << statusMarker
        << delimiter << "numOfLevel=" << (numOfLevel - '0')
        ;

        const int levels = GetLevelsNum();
        for (int i = 0; i < levels; ++i) {
            ss << depthLevels[i].ToString(id);
        }
        return ss.str();
    }
};


struct StrategyMarketDepth {
    LongMsgHeader msgHeader;
    char exchangeId;
    char symbol[30];
    char statusMarker;
    char numOfLevel;
    StrategyDepthLevel depthLevels[5];

    std::string GetIdentifier() const {
        return GetSymbol();
    }
    std::string GetSymbol() const {
        return std::string(symbol, sizeof(symbol));
    }
    char GetStatusMarker() const {
        return statusMarker;
    }
    int GetLevelsNum() const { 
        return numOfLevel - '0'; 
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "identifier=" << GetIdentifier()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "symbol=" << std::string(symbol, sizeof(symbol))
        << delimiter << "statusMarker=" << statusMarker
        << delimiter << "numOfLevel=" << (numOfLevel - '0')
        ;

        const int levels = GetLevelsNum();
        for (int i = 0; i < levels; ++i) {
            ss << depthLevels[i].ToString(id);
        }
        return ss.str();
    } 
};

//NF
struct FuturesSummary {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];
    char bidPrice[7];
    char bidPriceFractionIndicator;
    char bidSize[5];
    char askPrice[7];
    char askPriceFractionIndicator;
    char askSize[5];
    char lastPrice[7];
    char lastPriceFractionIndicator;
    char openPrice[7];
    char openPriceFractionIndicator;
    char highPrice[7];
    char highPriceFractionIndicator;
    char lowPrice[7];
    char lowPriceFractionIndicator;
    char settlementPrice[7];
    char settlementPriceFractionIndicator;
    char netChangeSign;
    char netChange[7];
    char netChangeFractionIndicator;
    char volume[8];
    char previousSettlementPrice[7];
    char previousSettlementPriceFractionIndicator;
    char openInterest[7];
    char openInterestDate[6];
    char reason;
    char externalPriceAtSource[7];
    char externalPriceFractionIndicator;

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    int64_t GetLastPrice() const {
        return ConvertCharArray<int64_t>(lastPrice, 7);
    }
    char GetLastPriceFractionIndicator() const {
        return lastPriceFractionIndicator;
    }
    int64_t GetOpenPrice() const {
        return ConvertCharArray<int64_t>(openPrice, 7);
    }
    char GetOpenPriceFractionIndicator() const {
        return openPriceFractionIndicator;
    }
    int64_t GetHighPrice() const {
        return ConvertCharArray<int64_t>(highPrice, 7);
    }
    char GetHighPriceFractionIndicator() const {
        return highPriceFractionIndicator;
    }
    int64_t GetLowPrice() const {
        return ConvertCharArray<int64_t>(lowPrice, 7);
    }
    char GetLowPriceFractionIndicator() const {
        return lowPriceFractionIndicator;
    }
    int64_t GetSettlementPrice() const {
        return ConvertCharArray<int64_t>(settlementPrice, 7);
    }
    char GetSettlemenetPriceFractionIndicator() const {
        return settlementPriceFractionIndicator;
    }
    int64_t GetVolume() const {
        return ConvertCharArray<int64_t>(lastPrice, 8);
    }
    int64_t GetPreviousSettlementPrice() const {
        return ConvertCharArray<int64_t>(previousSettlementPrice, 7);
    }
    char GetPreviousSettlementPriceFI() const {
        return previousSettlementPriceFractionIndicator;
    }
    char GetReason() const {
        return reason;
    }
  
    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "lastPrice=" << std::string(lastPrice, sizeof(lastPrice))
        << delimiter << "lastPriceFI=" << lastPriceFractionIndicator
        << delimiter << "openPrice=" << std::string(openPrice, sizeof(openPrice))
        << delimiter << "openPriceFI=" << openPriceFractionIndicator
        << delimiter << "highPrice=" << std::string(highPrice, sizeof(highPrice))
        << delimiter << "highPriceFI=" << highPriceFractionIndicator
        << delimiter << "lowPrice=" << std::string(lowPrice, sizeof(lowPrice))
        << delimiter << "lowPriceFI=" << lowPriceFractionIndicator
        << delimiter << "settlementPrice=" << std::string(settlementPrice, sizeof(settlementPrice))
        << delimiter << "settlementPriceFI=" << settlementPriceFractionIndicator
        << delimiter << "volume=" << std::string(volume, sizeof(volume))
        << delimiter << "previousSettlementPrice=" << std::string(previousSettlementPrice, sizeof(previousSettlementPrice))
        << delimiter << "previousSettlementPriceFI=" << previousSettlementPriceFractionIndicator
        << delimiter << "reason=" << reason
        << delimiter << "externalPriceAtSource=" << std::string(externalPriceAtSource, sizeof(externalPriceAtSource))
        << delimiter << "externalPriceFI=" << externalPriceFractionIndicator
        ;
        return ss.str();
    }   
};

//N
struct OptionSummary {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char expiryMonth;
    char strikePrice[7];
    char strikePriceFractionIndicator;
    char expiryYear[2];
    char expiryDay[2];
    char bidPrice[7];
    char bidPriceFractionIndicator;
    char bidSize[5];
    char askPrice[7];
    char askPriceFractionIndicator;
    char askSize[5];
    char lastPrice[7];
    char lastPriceFractionIndicator;
    char openInterest[7];
    char openInterestDate[6];
    char tick;
    char volume[8];
    char netChangeSign;
    char netChange[7];
    char netChangeFractionIndicator;
    char openPrice[7];
    char openPriceFractionIndicator;
    char highPrice[7];
    char highPriceFractionIndicator;
    char lowPrice[7];
    char lowPriceFractionIndicator;
    char optionMarker[2];
    char settlementPrice[7];
    char settlementPriceFractionIndicator;
    char previousSettlementPrice[7];
    char previousSettlementPriceFractionIndicator;
    char reason;

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetExpiryMonth()
                + GetStrikePrice()
                + GetStrikePriceFractionIndicator()
                + GetExpiryYear()
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetExpiryMonth() const {
        return expiryMonth;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFractionIndicator() const {
        return strikePriceFractionIndicator;
    }
    std::string GetExpiryYear() const {
        return std::string(expiryYear, sizeof(expiryYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    int64_t GetLastPrice() const {
        return ConvertCharArray<int64_t>(lastPrice, 7);
    }
    char GetLastPriceFractionIndicator() const {
        return lastPriceFractionIndicator;
    }
    int64_t GetOpenPrice() const {
        return ConvertCharArray<int64_t>(openPrice, 7);
    }
    char GetOpenPriceFractionIndicator() const {
        return openPriceFractionIndicator;
    }
    int64_t GetHighPrice() const {
        return ConvertCharArray<int64_t>(highPrice, 7);
    }
    char GetHighPriceFractionIndicator() const {
        return highPriceFractionIndicator;
    }
    int64_t GetLowPrice() const {
        return ConvertCharArray<int64_t>(lowPrice, 7);
    }
    char GetLowPriceFractionIndicator() const {
        return lowPriceFractionIndicator;
    }
    int64_t GetVolume() const {
        return ConvertCharArray<int64_t>(volume, 8);
    }
    char GetReason() const {
        return reason;
    }
    int64_t GetSettlementPrice() const {
        return ConvertCharArray<int64_t>(settlementPrice, 7);
    }
    char GetSettlemenetPriceFractionIndicator() const {
        return settlementPriceFractionIndicator;
    }
    int64_t GetPreviousSettlementPrice() const {
        return ConvertCharArray<int64_t>(previousSettlementPrice, 7);
    }
    char GetPreviousSettlementPriceFI() const {
        return previousSettlementPriceFractionIndicator;
    } 

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "lastPrice=" << std::string(lastPrice, sizeof(lastPrice))
        << delimiter << "lastPriceFI=" << lastPriceFractionIndicator
        << delimiter << "tick=" << tick
        << delimiter << "volume=" << std::string(volume, sizeof(volume))
        << delimiter << "openPrice=" << std::string(openPrice, sizeof(openPrice))
        << delimiter << "openPriceFI=" << openPriceFractionIndicator
        << delimiter << "highPrice=" << std::string(highPrice, sizeof(highPrice))
        << delimiter << "highPriceFI=" << highPriceFractionIndicator
        << delimiter << "lowPrice=" << std::string(lowPrice, sizeof(lowPrice))
        << delimiter << "lowPriceFI=" << lowPriceFractionIndicator
        << delimiter << "optionMarker=" << std::string(optionMarker, sizeof(optionMarker))
        << delimiter << "settlementPrice=" << std::string(settlementPrice, sizeof(settlementPrice))
        << delimiter << "settlementPriceFI=" << settlementPriceFractionIndicator
        << delimiter << "previousSettlementPrice=" << std::string(previousSettlementPrice, sizeof(previousSettlementPrice))
        << delimiter << "previousSettlementPriceFI=" << previousSettlementPriceFractionIndicator
        << delimiter << "reason=" << reason
        ;
        return ss.str();
    }
};

//NB
struct FutureOptionsSummary {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];
    char callPut;
    char strikePrice[7];
    char strikePriceFractionIndicator;
    char bidPrice[7];
    char bidPriceFractionIndicator;
    char bidSize[5];
    char askPrice[7];
    char askPriceFractionIndicator;
    char askSize[5];
    char lastPrice[7];
    char lastPriceFractionIndicator;
    char openInterest[7];
    char openInterestDate[6];
    char tick;
    char volume[8];
    char netChangeSign;
    char netChange[7];
    char netChangeFractionIndicator;
    char openPrice[7];
    char openPriceFractionIndicator;
    char highPrice[7];
    char highPriceFractionIndicator;
    char lowPrice[7];
    char lowPriceFractionIndicator;
    char settlementPrice[7];
    char settlementPriceFractionIndicator;
    char previousSettlementPrice[7];
    char previousSettlementPriceFractionIndicator;
    char reason;
    
    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                + GetCallPut()
                + GetStrikePrice()
                + GetStrikePriceFractionIndicator()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    char GetCallPut() const {
        return callPut;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFractionIndicator() const {
        return strikePriceFractionIndicator;
    } 
    int64_t GetLastPrice() const {
        return ConvertCharArray<int64_t>(lastPrice, 7);
    }
    char GetLastPriceFractionIndicator() const {
        return lastPriceFractionIndicator;
    }
    int64_t GetOpenPrice() const {
        return ConvertCharArray<int64_t>(openPrice, 7);
    }
    char GetOpenPriceFractionIndicator() const {
        return openPriceFractionIndicator;
    }
    int64_t GetHighPrice() const {
        return ConvertCharArray<int64_t>(highPrice, 7);
    }
    char GetHighPriceFractionIndicator() const {
        return highPriceFractionIndicator;
    }
    int64_t GetLowPrice() const {
        return ConvertCharArray<int64_t>(lowPrice, 7);
    }
    char GetLowPriceFractionIndicator() const {
        return lowPriceFractionIndicator;
    }
    int64_t GetVolume() const {
        return ConvertCharArray<int64_t>(volume, 8);
    }
    int64_t GetSettlementPrice() const {
        return ConvertCharArray<int64_t>(settlementPrice, 7);
    }
    char GetSettlemenetPriceFractionIndicator() const {
        return settlementPriceFractionIndicator;
    }
    int64_t GetPreviousSettlementPrice() const {
        return ConvertCharArray<int64_t>(previousSettlementPrice, 7);
    }
    char GetPreviousSettlementPriceFI() const {
        return previousSettlementPriceFractionIndicator;
    }
    char GetReason() const {
        return reason;
    }  

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "lastPrice=" << std::string(lastPrice, sizeof(lastPrice))
        << delimiter << "lastPriceFI=" << lastPriceFractionIndicator
        << delimiter << "tick=" << tick
        << delimiter << "volume=" << std::string(volume, sizeof(volume))
        << delimiter << "openPrice=" << std::string(openPrice, sizeof(openPrice))
        << delimiter << "openPriceFI=" << openPriceFractionIndicator
        << delimiter << "highPrice=" << std::string(highPrice, sizeof(highPrice))
        << delimiter << "highPriceFI=" << highPriceFractionIndicator
        << delimiter << "lowPrice=" << std::string(lowPrice, sizeof(lowPrice))
        << delimiter << "lowPriceFI=" << lowPriceFractionIndicator
        << delimiter << "settlementPrice=" << std::string(settlementPrice, sizeof(settlementPrice))
        << delimiter << "settlementPriceFI=" << settlementPriceFractionIndicator
        << delimiter << "previousSettlementPrice=" << std::string(previousSettlementPrice, sizeof(previousSettlementPrice))
        << delimiter << "previousSettlementPriceFI=" << previousSettlementPriceFractionIndicator
        << delimiter << "reason=" << reason
        ;
        return ss.str();
    }
};

//NS
struct StrategySummary {
    LongMsgHeader msgHeader;
    char exchangeId;
    char strategySymbol[30];
    char bidPriceSign;
    char bidPrice[7];
    char bidPriceFractionIndicator;
    char bidSize[5];
    char askPriceSign;
    char askPrice[7];
    char askPriceFractionIndicator;
    char askSize[5];
    char lastPriceSign;
    char lastPrice[7];
    char lastPriceFractionIndicator;
    char openPriceSign;
    char openPrice[7];
    char openPriceFractionIndicator;
    char highPriceSign;
    char highPrice[7];
    char highPriceFractionIndicator;
    char lowPriceSign;
    char lowPrice[7];
    char lowPriceFractionIndicator;
    char netChangeSign;
    char netChange[7];
    char netChangeFractionIndicator;
    char volume[8];
    char reason;

    std::string GetIdentifier() const {
        return GetStrategySymbol();
    }
    std::string GetStrategySymbol() const {
        return std::string(strategySymbol, sizeof(strategySymbol));
    }
    int64_t GetLastPrice() const {
        return ConvertCharArray<int64_t>(lastPrice, 7);
    }
    char GetLastPriceFractionIndicator() const {
        return lastPriceFractionIndicator;
    }
    int64_t GetOpenPrice() const {
        const int64_t value = ConvertCharArray<int64_t>(openPrice, 7);
        return openPriceSign == '-' ? value * -1 : value;
    }
    char GetOpenPriceFractionIndicator() const {
        return openPriceFractionIndicator;
    }
    int64_t GetHighPrice() const {
        const int64_t value = ConvertCharArray<int64_t>(highPrice, 7);
        return highPriceSign == '-' ? value * -1 : value;
    }
    char GetHighPriceFractionIndicator() const {
        return highPriceFractionIndicator;
    }
    int64_t GetLowPrice() const {
        const int64_t value = ConvertCharArray<int64_t>(lowPrice, 7);
        return lowPriceSign == '-' ? value * -1 : value;
    }
    char GetLowPriceFractionIndicator() const {
        return lowPriceFractionIndicator;
    }
    int64_t GetVolume() const {
        return ConvertCharArray<int64_t>(lastPrice, 8);
    }
    char GetReason() const {
        return reason;
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "lastPriceSign=" << lastPriceSign
        << delimiter << "lastPrice=" << std::string(lastPrice, sizeof(lastPrice))
        << delimiter << "lastPriceFI=" << lastPriceFractionIndicator
        << delimiter << "openPriceSign=" << openPriceSign
        << delimiter << "openPrice=" << std::string(openPrice, sizeof(openPrice))
        << delimiter << "openPriceFI=" << openPriceFractionIndicator
        << delimiter << "highPriceSign=" << highPriceSign
        << delimiter << "highPrice=" << std::string(highPrice, sizeof(highPrice))
        << delimiter << "highPriceFI=" << highPriceFractionIndicator
        << delimiter << "lowPriceSign=" << lowPriceSign
        << delimiter << "lowPrice=" << std::string(lowPrice, sizeof(lowPrice))
        << delimiter << "lowPriceFI=" << lowPriceFractionIndicator
        << delimiter << "volume=" << std::string(volume, sizeof(volume))
        << delimiter << "reason=" << reason
        ;
        return ss.str();
    }
};


//JF
struct FuturesInstrumentKeys {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth; //delivery
    char symbolYear[2]; //delivery
    char expiryDay[2]; //delivery
    char expiryDate[6];
    char maxNumOfContractsPerOrder[6];
    char minNumOfContractsPerOrder[6];
    char maxThresholdPrice[7];
    char maxThresholdPriceFractionIndicator;
    char minThresholdPrice[7];
    char minThresholdPriceFractionIndicator;
    char tickIncrement[7];
    char tickIncrementFractionIndicator;
    char marketFlowIndicator[2];
    char groupInstrument[2];
    char instrument[4];
    char instrumentExternalCode[30];
    char contractSize[8];
    char tickValue[7];
    char tickValueFractionIndicator;
    char currency[3];
    char underlyingSymbol[12];
    char deliveryType;
    char apRootSymbol[6];
    char apSymbolMonth;
    char apSymbolYear[2];
    char apExpiryDay[2];
    char lastTradingDate[8];
    char lastTradingTime[6];

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                ;
    }
    std::string GetInstrumentName() const {
        return GetString(GetRootSymbol())
                + GetSymbolMonth()
                + GetSymbolYear().back()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    std::string GetExpiryDate() const {
        return std::string(expiryDate, sizeof(expiryDate));
    }
    std::string GetGroup() const {
        return std::string(groupInstrument, sizeof(groupInstrument));
    }
    std::string GetInstrument() const {
        return std::string(instrument, sizeof(instrument));
    }
    std::string GetInstrumentExternalCode() const {
        return std::string(instrumentExternalCode, sizeof(instrumentExternalCode));
    }
    std::string GetCurrency() const {
        return std::string(currency, sizeof(currency));
    }
    std::string GetLastTradingDate() const {
        return std::string(lastTradingDate, sizeof(lastTradingDate));
    }  
    int64_t GetContractSize() const {
        return ConvertCharArray<int64_t>(contractSize, 8);
    }
    std::string GetTickIncrement() const {
        return std::string(tickIncrement, sizeof(tickIncrement));
    }
    char GetTickIncrementFI() const {
        return tickIncrementFractionIndicator;
    }
    int64_t GetTickValue() const {
        return ConvertCharArray<int64_t>(tickValue, 7);
    }
    char GetTickValueFI() const {
        return tickValueFractionIndicator;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "rootSymbol=" << std::string(rootSymbol, sizeof(rootSymbol))
        << delimiter << "symbolMonth=" << symbolMonth
        << delimiter << "symbolYear=" << std::string(symbolYear, sizeof(symbolYear))
        << delimiter << "expiryDay=" << std::string(expiryDay, sizeof(expiryDay))
        << delimiter << "expiryDate=" << std::string(expiryDate, sizeof(expiryDate))
        << delimiter << "maxNumOfContractsPerOrder=" << std::string(maxNumOfContractsPerOrder, sizeof(maxNumOfContractsPerOrder))
        << delimiter << "minNumOfContractsPerOrder=" << std::string(minNumOfContractsPerOrder, sizeof(minNumOfContractsPerOrder))
        << delimiter << "maxThresholdPrice=" << std::string(maxThresholdPrice, sizeof(maxThresholdPrice))
        << delimiter << "maxThresholdPriceFractionIndicator=" << maxThresholdPriceFractionIndicator
        << delimiter << "minThresholdPrice=" << std::string(minThresholdPrice, sizeof(minThresholdPrice))
        << delimiter << "minThresholdPriceFractionIndicator=" << minThresholdPriceFractionIndicator
        << delimiter << "tickIncrement=" << std::string(tickIncrement, sizeof(tickIncrement))
        << delimiter << "tickIncrementFractionIndicator=" << tickIncrementFractionIndicator
        << delimiter << "marketFlowIndicator=" << std::string(marketFlowIndicator, sizeof(marketFlowIndicator))
        << delimiter << "groupInstrument=" << std::string(groupInstrument, sizeof(groupInstrument))
        << delimiter << "instrument=" << std::string(instrument, sizeof(instrument))
        << delimiter << "instrumentExternalCode=" << std::string(instrumentExternalCode, sizeof(instrumentExternalCode))
        << delimiter << "contractSize=" << std::string(contractSize, sizeof(contractSize))
        << delimiter << "tickValue=" << std::string(tickValue, sizeof(tickValue))
        << delimiter << "tickValueFractionIndicator=" << tickValueFractionIndicator
        << delimiter << "currency=" << std::string(currency, sizeof(currency))
        << delimiter << "underlyingSymbol=" << std::string(underlyingSymbol, sizeof(underlyingSymbol))
        << delimiter << "deliveryType=" << deliveryType
        << delimiter << "apRootSymbol=" << std::string(apRootSymbol, sizeof(apRootSymbol))
        << delimiter << "apSymbolMonth=" << apSymbolMonth
        << delimiter << "apSymbolYear=" << std::string(apSymbolYear, sizeof(apSymbolYear))
        << delimiter << "apExpiryDay=" << std::string(apExpiryDay, sizeof(apExpiryDay))
        << delimiter << "lastTradingDate=" << std::string(lastTradingDate, sizeof(lastTradingDate))
        << delimiter << "lastTradingTime=" << std::string(lastTradingTime, sizeof(lastTradingTime))
        ;
        return ss.str();
    }
};

//J
struct OptionInstrumentKeys {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char expiryMonth; //cdd DecodeOption
    char strikePrice[7];
    char strikePriceFractionIndicator;
    char expiryYear[2];  //cdd
    char expiryDay[2];   //cdd
    char strikePriceCurrency[3];
    char maxNumOfContractsPerOrder[6];
    char minNumOfContractsPerOrder[6];
    char maxThresholdPrice[7];
    char maxThresholdPriceFractionIndicator;
    char minThresholdPrice[7];
    char minThresholdPriceFractionIndicator;
    char tickIncrement[7];
    char tickIncrementFractionIndicator;
    char optionType;
    char marketFlowIndicator[2];
    char groupInstrument[2];
    char instrument[4];
    char instrumentExternalCode[30];
    char optionMarker[2];
    char underlyingSymbol[12];
    char contractSize[8];
    char tickValue[7];
    char tickValueFractionIndicator;
    char currency[3];
    char deliveryType;
    char lastTradingDate[8];
    char lastTradingTime[6];

    std::string GetIdentifier() const {
        return GetRootSymbol() 
                + GetExpiryMonth()
                + GetStrikePrice()
                + GetStrikePriceFI()
                + GetExpiryYear()
                + GetExpiryDay()
                ;
    }
    std::string GetInstrumentName() const {
        return GetIdentifier();
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetExpiryMonth() const {
        return expiryMonth;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFI() const {
        return strikePriceFractionIndicator;
    }
    std::string GetExpiryYear() const {
        return std::string(expiryYear, sizeof(expiryYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    std::string GetGroup() const {
        return std::string(groupInstrument, sizeof(groupInstrument));
    }
    std::string GetInstrument() const {
        return std::string(instrument, sizeof(instrument));
    }
    std::string GetInstrumentExternalCode() const {
        return std::string(instrumentExternalCode, sizeof(instrumentExternalCode));
    }
    std::string GetCurrency() const {
        return std::string(currency, sizeof(currency));
    }
    std::string GetLastTradingDate() const {
        return std::string(lastTradingDate, sizeof(lastTradingDate));
    }
    int64_t GetContractSize() const {
        return ConvertCharArray<int64_t>(contractSize, 8);
    }
    std::string GetTickIncrement() const {
        return std::string(tickIncrement, sizeof(tickIncrement));
    }
    char GetTickIncrementFI() const {
        return tickIncrementFractionIndicator;
    }
    int64_t GetTickValue() const {
        return ConvertCharArray<int64_t>(tickValue, 7);
    }
    char GetTickValueFI() const {
        return tickValueFractionIndicator;
    }
    
    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "rootSymbol=" << std::string(rootSymbol, sizeof(rootSymbol))
        << delimiter << "expiryMonth=" << expiryMonth
        << delimiter << "strikPrice=" << std::string(strikePrice, sizeof(strikePrice))
        << delimiter << "strikePriceFractionIndicator=" << strikePriceFractionIndicator
        << delimiter << "expiryYear=" << std::string(expiryYear, sizeof(expiryYear))
        << delimiter << "expiryDay=" << std::string(expiryDay, sizeof(expiryDay))
        << delimiter << "strikePriceCurrency=" << std::string(strikePriceCurrency, sizeof(strikePriceCurrency))
        << delimiter << "maxNumOfContractsPerOrder=" << std::string(maxNumOfContractsPerOrder, sizeof(maxNumOfContractsPerOrder))
        << delimiter << "minNumOfContractsPerOrder=" << std::string(minNumOfContractsPerOrder, sizeof(minNumOfContractsPerOrder))
        << delimiter << "maxThresholdPrice=" << std::string(maxThresholdPrice, sizeof(maxThresholdPrice))
        << delimiter << "maxThresholdPriceFractionIndicator=" << maxThresholdPriceFractionIndicator
        << delimiter << "minThresholdPrice=" << std::string(minThresholdPrice, sizeof(minThresholdPrice))
        << delimiter << "minThresholdPriceFractionIndicator=" << minThresholdPriceFractionIndicator
        << delimiter << "tickIncrement=" << std::string(tickIncrement, sizeof(tickIncrement))
        << delimiter << "tickIncrementFractionIndicator=" << tickIncrementFractionIndicator
        << delimiter << "optionType=" << optionType
        << delimiter << "marketFlowIndicator=" << std::string(marketFlowIndicator, sizeof(marketFlowIndicator))
        << delimiter << "groupInstrument=" << std::string(groupInstrument, sizeof(groupInstrument))
        << delimiter << "instrument=" << std::string(instrument, sizeof(instrument))
        << delimiter << "instrumentExternalCode=" << std::string(instrumentExternalCode, sizeof(instrumentExternalCode))
        << delimiter << "optionMarker=" << std::string(optionMarker, sizeof(optionMarker))
        << delimiter << "underlyingSymbol=" << std::string(underlyingSymbol, sizeof(underlyingSymbol))
        << delimiter << "contractSize=" << std::string(contractSize, sizeof(contractSize))
        << delimiter << "tickValue=" << std::string(tickValue, sizeof(tickValue))
        << delimiter << "tickValueFractionIndicator=" << tickValueFractionIndicator
        << delimiter << "currency=" << std::string(currency, sizeof(currency))
        << delimiter << "deliveryType=" << deliveryType
        << delimiter << "lastTradingDate=" << std::string(lastTradingDate, sizeof(lastTradingDate))
        << delimiter << "lastTradingTime=" << std::string(lastTradingTime, sizeof(lastTradingTime))
        ;
        return ss.str();
    }
};

//JB
struct FutureOptionsInstrumentKeys {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth; //cdd - deliverymonth
    char symbolYear[2]; //cdd - deliveryyear
    char expiryDay[2]; //expiryday
    char callPut;
    char strikePrice[7];
    char strikePriceFractionIndicator;
    char expiryDate[6];
    char strikePriceCurrency[3];
    char maxNumOfContractsPerOrder[6];
    char minNumOfContractsPerOrder[6];
    char maxThresholdPrice[7];
    char maxThresholdPriceFractionIndicator;
    char minThresholdPrice[7];
    char minThresholdPriceFractionIndicator;
    char tickIncrement[7];
    char tickIncrementFractionIndicator;
    char marketFlowIndicator[2];
    char groupInstrument[2];
    char instrument[4];
    char instrumentExternalCode[30];
    char contractSize[8];
    char tickValue[7];
    char tickValueFractionIndicator;
    char currency[3];
    char deliveryType;
    char underlyingRootSymbol[12];
    char underylyingSymbolMonth;
    char underlyingSymbolYear[2];
    char lastTradingDate[8];
    char lastTradingTime[6];

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                + GetCallPut()
                + GetStrikePrice()
                + GetStrikePriceFI()
                ;
    }
    std::string GetInstrumentName() const {
        return GetIdentifier();
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    char GetCallPut() const {
        return callPut;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFI() const {
        return strikePriceFractionIndicator;
    }
    std::string GetExpiryDate() const {
        return std::string(expiryDate, sizeof(expiryDate));
    }
    std::string GetStrikePriceCurrency() const {
        return std::string(strikePriceCurrency, sizeof(strikePriceCurrency));
    }
    std::string GetGroup() const {
        return std::string(groupInstrument, sizeof(groupInstrument));
    }
    std::string GetInstrument() const {
        return std::string(instrument, sizeof(instrument));
    }
    std::string GetInstrumentExternalCode() const {
        return std::string(instrumentExternalCode, sizeof(instrumentExternalCode));
    }
    std::string GetCurrency() const {
        return std::string(currency, sizeof(currency));
    }
    std::string GetLastTradingDate() const {
        return std::string(lastTradingDate, sizeof(lastTradingDate));
    }
    int64_t GetContractSize() const {
        return ConvertCharArray<int64_t>(contractSize, 8);
    }
    std::string GetTickIncrement() const {
        return std::string(tickIncrement, sizeof(tickIncrement));
    }
    char GetTickIncrementFI() const {
        return tickIncrementFractionIndicator;
    }
    int64_t GetTickValue() const {
        return ConvertCharArray<int64_t>(tickValue, 7);
    }
    char GetTickValueFI() const {
        return tickValueFractionIndicator;
    }    
    
    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "rootSymbol=" << std::string(rootSymbol, sizeof(rootSymbol))
        << delimiter << "symbolMonth=" << symbolMonth
        << delimiter << "symbolYear=" << std::string(symbolYear, sizeof(symbolYear))
        << delimiter << "expiryDay=" << std::string(expiryDay, sizeof(expiryDay))
        << delimiter << "callPut=" << callPut
        << delimiter << "strikePrice=" << std::string(strikePrice, sizeof(strikePrice))
        << delimiter << "strikePriceFractionIndicator=" << strikePriceFractionIndicator
        << delimiter << "expiryDate=" << std::string(expiryDate, sizeof(expiryDate))
        << delimiter << "strikePriceCurrency=" << std::string(strikePriceCurrency, sizeof(strikePriceCurrency))
        << delimiter << "maxNumOfContractsPerOrder=" << std::string(maxNumOfContractsPerOrder, sizeof(maxNumOfContractsPerOrder))
        << delimiter << "minNumOfContractsPerOrder=" << std::string(minNumOfContractsPerOrder, sizeof(minNumOfContractsPerOrder))
        << delimiter << "maxThresholdPrice=" << std::string(maxThresholdPrice, sizeof(maxThresholdPrice))
        << delimiter << "maxThresholdPriceFractionIndicator=" << maxThresholdPriceFractionIndicator
        << delimiter << "minThresholdPrice=" << std::string(minThresholdPrice, sizeof(minThresholdPrice))
        << delimiter << "minThresholdPriceFractionIndicator=" << minThresholdPriceFractionIndicator
        << delimiter << "tickIncrement=" << std::string(tickIncrement, sizeof(tickIncrement))
        << delimiter << "tickIncrementFractionIndicator=" << tickIncrementFractionIndicator
        << delimiter << "marketFlowIndicator=" << std::string(marketFlowIndicator, sizeof(marketFlowIndicator))
        << delimiter << "groupInstrument=" << std::string(groupInstrument, sizeof(groupInstrument))
        << delimiter << "instrument=" << std::string(instrument, sizeof(instrument))
        << delimiter << "instrumentExternalCode=" << std::string(instrumentExternalCode, sizeof(instrumentExternalCode))
        << delimiter << "contractSize=" << std::string(contractSize, sizeof(contractSize))
        << delimiter << "tickValue=" << std::string(tickValue, sizeof(tickValue))
        << delimiter << "tickValueFractionIndicator=" << tickValueFractionIndicator
        << delimiter << "currency=" << std::string(currency, sizeof(currency))
        << delimiter << "deliveryType=" << deliveryType
        << delimiter << "underlyingRootSymbol=" << std::string(underlyingRootSymbol, sizeof(underlyingRootSymbol))
        << delimiter << "underylyingSymbolMonth=" << underylyingSymbolMonth
        << delimiter << "underlyingSymbolYear=" << std::string(underlyingSymbolYear, sizeof(underlyingSymbolYear))
        << delimiter << "lastTradingDate=" << std::string(lastTradingDate, sizeof(lastTradingDate))
        << delimiter << "lastTradingTime=" << std::string(lastTradingTime, sizeof(lastTradingTime))
        ;
        return ss.str();
    }
};


struct Legs {
    char legGroupInstrument[2];
    char legInstrument[4];
    char legRatio[4];
    char legRatioFractionIndicator;
    char legPrice[7];
    char legPriceFractionIndicator;

    std::string GetIdentifier() const {
        return GetLegGroupInstrument() + GetLegInstrument();
    }
    std::string GetLegGroupInstrument() const {
        return std::string(legGroupInstrument, sizeof(legGroupInstrument));
    }
    std::string GetLegInstrument() const {
        return std::string(legInstrument, sizeof(legInstrument));
    }
    int32_t GetLegRatio() const {
        return ConvertCharArray<int32_t>(legRatio, 4);
    }
    char GetLegRatioFI() const {
        return legRatioFractionIndicator;
    }
    int64_t GetLegPrice() const {
        return ConvertCharArray<int64_t>(legPrice, 7);
    }
    char GetLegPriceFI() const {
        return legPriceFractionIndicator;
    }
    
    std::string ToString() const {
        std::stringstream ss;
        ss << delimiter << "Leg - " 
        << ", legGroupInstrument=" << std::string(legGroupInstrument, sizeof(legGroupInstrument))
        << ", legInstrument=" << std::string(legInstrument, sizeof(legInstrument))
        << ", legRatio=" << std::string(legRatio, sizeof(legRatio))
        << ", legRatioFractionIndicator=" << legRatioFractionIndicator
        << ", legPrice=" << std::string(legPrice, sizeof(legPrice))
        << ", legPriceFractionIndicator=" << legPriceFractionIndicator
        ;
        return ss.str();
    }
};


//JS
struct StrategyInstrumentKeys {
    LongMsgHeader msgHeader;
    char exchangeId;
    char strategySymbol[30];
    char expiryYear[2];
    char expiryMonth;
    char expiryDay[2];
    char maxNumOfContractsPerOrder[6];
    char minNumOfContractsPerOrder[6];
    char maxThresholdPrice[7];
    char maxThresholdPriceFractionIndicator;
    char minThresholdPrice[7];
    char minThresholdPriceFractionIndicator;
    char tickIncrement[7];
    char tickIncrementFractionIndicator;
    char marketFlowIndicator[2];
    char groupInstrument[2];
    char instrument[4];
    char instrumentExternalCode[30];
    char impliedAllowed;
    char strategyCode[2];
    char strategyType;
    char lastTradingDate[8];
    char lastTradingTime[6];
    char legsNum[2];
    Legs legs[40];

    std::string GetIdentifier() const {
        return GetStrategySymbol();
    }
    std::string GetInstrumentName() const {
        return GetString(GetStrategySymbol());
    }
    std::string GetStrategySymbol() const {
        return std::string(strategySymbol, sizeof(strategySymbol));
    }
    std::string GetExpiryYear() const {
        return std::string(expiryYear, sizeof(expiryYear));
    }
    char GetExpiryMonth() const {
        return expiryMonth;
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    std::string GetTickIncrement() const {
        return std::string(tickIncrement, sizeof(tickIncrement));
    }
    char GetTickIncrementFI() const {
        return tickIncrementFractionIndicator;
    }
    std::string GetMarketFlowIndicator() const {
        return std::string(marketFlowIndicator, sizeof(marketFlowIndicator));
    }
    std::string GetGroup() const {
        return std::string(groupInstrument, sizeof(groupInstrument));
    }
    std::string GetInstrument() const {
        return std::string(instrument, sizeof(instrument));
    }
    std::string GetInstrumentExternalCode() const {
        return std::string(instrumentExternalCode, sizeof(instrumentExternalCode));
    }
    std::string GetLastTradingDate() const {
        return std::string(lastTradingDate, sizeof(lastTradingDate));
    }
    int GetLegsNum() const {
        return ConvertCharArray<int>(legsNum, 2);
    }
    std::vector<std::string> GetLegIdentifiers() const {
        std::vector<std::string> ret;
        const int legsNum = GetLegsNum();
        for (int i = 0; i < legsNum; ++i) {
            ret.push_back(legs[i].GetIdentifier());
        }

        return ret;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "strategySymbol=" << std::string(strategySymbol, sizeof(strategySymbol))
        << delimiter << "expiryYear=" << std::string(expiryYear, sizeof(expiryYear))
        << delimiter << "expiryMonth=" << expiryMonth
        << delimiter << "expiryDay=" << std::string(expiryDay, sizeof(expiryDay))
        << delimiter << "maxNumOfContractsPerOrder=" << std::string(maxNumOfContractsPerOrder, sizeof(maxNumOfContractsPerOrder))
        << delimiter << "minNumOfContractsPerOrder=" << std::string(minNumOfContractsPerOrder, sizeof(minNumOfContractsPerOrder))
        << delimiter << "maxThresholdPrice=" << std::string(maxThresholdPrice, sizeof(maxThresholdPrice))
        << delimiter << "maxThresholdPriceFractionIndicator=" << maxThresholdPriceFractionIndicator
        << delimiter << "minThresholdPrice=" << std::string(minThresholdPrice, sizeof(minThresholdPrice))
        << delimiter << "minThresholdPriceFractionIndicator=" << minThresholdPriceFractionIndicator
        << delimiter << "tickIncrement=" << std::string(tickIncrement, sizeof(tickIncrement))
        << delimiter << "tickIncrementFractionIndicator=" << tickIncrementFractionIndicator
        << delimiter << "marketFlowIndicator=" << std::string(marketFlowIndicator, sizeof(marketFlowIndicator))
        << delimiter << "groupInstrument=" << std::string(groupInstrument, sizeof(groupInstrument))
        << delimiter << "instrument=" << std::string(instrument, sizeof(instrument))
        << delimiter << "instrumentExternalCode=" << std::string(instrumentExternalCode, sizeof(instrumentExternalCode))
        << delimiter << "impliedAllowed=" << impliedAllowed
        << delimiter << "strategyCode=" << std::string(strategyCode, sizeof(strategyCode))
        << delimiter << "strategyType=" << strategyType
        << delimiter << "lastTradingDate=" << std::string(lastTradingDate, sizeof(lastTradingDate))
        << delimiter << "lastTradingTime=" << std::string(lastTradingTime, sizeof(lastTradingTime))
        << delimiter << "legsNum=" << std::string(legsNum, sizeof(legsNum))
        << delimiter << "nums=" << GetLegsNum()
        ;

        const int legsNum = GetLegsNum();
        for (int i = 0; i < legsNum; ++i) {
            ss << legs[i].ToString();
        }
        return ss.str();
    }
};


//CF
struct FuturesTrade {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];
    char volume[8];
    char tradePrice[7];
    char tradePriceFractionIndicator;
    char netChangeSign;
    char netChange[7];
    char netChangeFractionIndicator;
    char priceIndicatorMarker;
    char tradeNumber[8];
    //skipping AuctionID

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    int64_t GetVolume() const {
        return ConvertCharArrayWithLastByteCheck<int64_t>(volume, 8);
    }
    int64_t GetTradePrice() const {
        return ConvertCharArray<int64_t>(tradePrice, 7);
    }
    char GetTradePriceFractionIndicator() const {
        return tradePriceFractionIndicator;
    }
    char GetPriceIndicatorMarker() const {
        return priceIndicatorMarker;
    }
    std::string GetTradeNumber() const {
        return std::string(tradeNumber, sizeof(tradeNumber));
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "volume=" << std::string(volume, sizeof(volume))
        << delimiter << "tradePrice=" << std::string(tradePrice, sizeof(tradePrice))
        << delimiter << "tradePriceFI=" << tradePriceFractionIndicator
        << delimiter << "priceIndicatorMarker=" << priceIndicatorMarker
        << delimiter << "tradeNumber=" << std::string(tradeNumber, sizeof(tradeNumber))
        ;
        return ss.str();
    }
};


//C
struct OptionTrade {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char expiryMonth;
    char strikePrice[7];
    char strikePriceFractionIndicator;
    char expiryYear[2];
    char expiryDay[2];
    char volume[8];
    char tradePrice[7];
    char tradePriceFractionIndicator;
    char netChangeSign;
    char netChange[7];
    char netChangeFractionIndicator;
    char priceIndicatorMarker;
    char tradeNumber[8];
    //skipping AuctionID

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetExpiryMonth()
                + GetStrikePrice()
                + GetStrikePriceFractionIndicator()
                + GetExpiryYear()
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetExpiryMonth() const {
        return expiryMonth;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFractionIndicator() const {
        return strikePriceFractionIndicator;
    }
    std::string GetExpiryYear() const {
        return std::string(expiryYear, sizeof(expiryYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    int64_t GetVolume() const {
        return ConvertCharArrayWithLastByteCheck<int64_t>(volume, 8);
    }
    int64_t GetTradePrice() const {
        return ConvertCharArray<int64_t>(tradePrice, 7);
    }
    char GetTradePriceFractionIndicator() const {
        return tradePriceFractionIndicator;
    }
    char GetPriceIndicatorMarker() const {
        return priceIndicatorMarker;
    }
    std::string GetTradeNumber() const {
        return std::string(tradeNumber, sizeof(tradeNumber));
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "volume=" << std::string(volume, sizeof(volume))
        << delimiter << "tradePrice=" << std::string(tradePrice, sizeof(tradePrice))
        << delimiter << "tradePriceFI=" << tradePriceFractionIndicator
        << delimiter << "priceIndicatorMarker=" << priceIndicatorMarker
        << delimiter << "tradeNumber=" << std::string(tradeNumber, sizeof(tradeNumber))
        ;
        return ss.str();
    }
};


//CB
struct FutureOptionsTrade {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];
    char callPut;
    char strikePrice[7];
    char strikePriceFractionIndicator;
    char volume[8];
    char tradePrice[7];
    char tradePriceFractionIndicator;
    char priceIndicatorMarker;
    char netChangeSign;
    char netChange[7];
    char netChangeFractionIndicator;
    char tradeNumber[8];
    //skipping AuctionID

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                + GetCallPut()
                + GetStrikePrice()
                + GetStrikePriceFractionIndicator()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    char GetCallPut() const {
        return callPut;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFractionIndicator() const {
        return strikePriceFractionIndicator;
    }
    int64_t GetVolume() const {
        return ConvertCharArrayWithLastByteCheck<int64_t>(volume, 8);
    }
    int64_t GetTradePrice() const {
        return ConvertCharArray<int64_t>(tradePrice, 7);
    }
    char GetTradePriceFractionIndicator() const {
        return tradePriceFractionIndicator;
    }
    char GetPriceIndicatorMarker() const {
        return priceIndicatorMarker;
    }
    std::string GetTradeNumber() const {
        return std::string(tradeNumber, sizeof(tradeNumber));
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "volume=" << std::string(volume, sizeof(volume))
        << delimiter << "tradePrice=" << std::string(tradePrice, sizeof(tradePrice))
        << delimiter << "tradePriceFI=" << tradePriceFractionIndicator
        << delimiter << "priceIndicatorMarker=" << priceIndicatorMarker
        << delimiter << "tradeNumber=" << std::string(tradeNumber, sizeof(tradeNumber))
        ;
        return ss.str();
    }
};


//CS
struct StrategyTrade {
    LongMsgHeader msgHeader;
    char exchangeId;
    char symbol[30];
    char volume[8];
    char tradePriceSign;
    char tradePrice[7];
    char tradePriceFractionIndicator;
    char netChangeSign;
    char netChange[7];
    char netChangeFractionIndicator;
    char priceIndicatorMarker;
    char tradeNumber[8];
    //skipping AuctionID

    std::string GetIdentifier() const {
        return GetSymbol();
    }
    std::string GetSymbol() const {
        return std::string(symbol, sizeof(symbol));
    }
    int64_t GetVolume() const {
        return ConvertCharArrayWithLastByteCheck<int64_t>(volume, 8);
    }
    char GetTradePriceSign() const {
        return tradePriceSign;
    }
    int64_t GetTradePrice() const {
        const int64_t value = ConvertCharArray<int64_t>(tradePrice, 7);
        return tradePriceSign == '-' ? value * -1 : value;
    }
    char GetTradePriceFractionIndicator() const {
        return tradePriceFractionIndicator;
    }
    char GetPriceIndicatorMarker() const {
        return priceIndicatorMarker;
    }
    std::string GetTradeNumber() const {
        return std::string(tradeNumber, sizeof(tradeNumber));
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << ", volume=" << std::string(volume, sizeof(volume))
        << ", trade price sign=" << tradePriceSign
        << ", tradePrice=" << std::string(tradePrice, sizeof(tradePrice))
        << ", tradePriceFI=" << tradePriceFractionIndicator
        << ", priceIndicatorMarker=" << priceIndicatorMarker
        << ", tradeNumber=" << std::string(tradeNumber, sizeof(tradeNumber))
        ;
        return ss.str();
    }
};


//DF
struct FuturesRequestForQuote {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];   
    char requestedSize[8];
    char requestedMarketSide;

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    int64_t GetRequestedSize() const {
        return ConvertCharArrayWithLastByteCheck<int64_t>(requestedSize, 8);
    }
    char GetRequestedMarketSide() const {
        return requestedMarketSide;
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "requestedSize=" << std::string(requestedSize, sizeof(requestedSize))
        << delimiter << "requestedMarketSide=" << requestedMarketSide
        ;
        return ss.str();
    }
};

//D
struct OptionRequestForQuote {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char expiryMonth;
    char strikePrice[7];
    char strikePriceFractionIndicator;
    char expiryYear[2];
    char expiryDay[2];
    char requestedSize[8];
    char requestedMarketSide;

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetExpiryMonth()
                + GetStrikePrice()
                + GetStrikePriceFractionIndicator()
                + GetExpiryYear()
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetExpiryMonth() const {
        return expiryMonth;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFractionIndicator() const {
        return strikePriceFractionIndicator;
    }
    std::string GetExpiryYear() const {
        return std::string(expiryYear, sizeof(expiryYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    int64_t GetRequestedSize() const {
        return ConvertCharArrayWithLastByteCheck<int64_t>(requestedSize, 8);
    }
    char GetRequestedMarketSide() const {
        return requestedMarketSide;
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "requestedSize=" << std::string(requestedSize, sizeof(requestedSize))
        << delimiter << "requestedMarketSide=" << requestedMarketSide
        ;
        return ss.str();
    }
};

//DB
struct FutureOptionsRequestForQuote {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];
    char callPut;
    char strikePrice[7];
    char strikePriceFractionIndicator; 
    char requestedSize[8];
    char requestedMarketSide;

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                + GetCallPut()
                + GetStrikePrice()
                + GetStrikePriceFractionIndicator()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    char GetCallPut() const {
        return callPut;
    }
    std::string GetStrikePrice() const {
        return std::string(strikePrice, sizeof(strikePrice));
    }
    char GetStrikePriceFractionIndicator() const {
        return strikePriceFractionIndicator;
    }
    int64_t GetRequestedSize() const {
        return ConvertCharArrayWithLastByteCheck<int64_t>(requestedSize, 8);
    }
    char GetRequestedMarketSide() const {
        return requestedMarketSide;
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "requestedSize=" << std::string(requestedSize, sizeof(requestedSize))
        << delimiter << "requestedMarketSide=" << requestedMarketSide
        ;
        return ss.str();
    }
};


//DS
struct StrategyRequestForQuote {
    LongMsgHeader msgHeader;
    char exchangeId;
    char symbol[30]; 
    char requestedSize[8];
    char requestedMarketSide;

    std::string GetIdentifier() const {
        return GetSymbol();
    }
    std::string GetSymbol() const {
        return std::string(symbol, sizeof(symbol));
    }
    int64_t GetRequestedSize() const {
        return ConvertCharArrayWithLastByteCheck<int64_t>(requestedSize, 8);
    }
    char GetRequestedMarketSide() const {
        return requestedMarketSide;
    }

    std::string ToString() const {
        const std::string id = GetIdentifier();

        std::stringstream ss;
        ss << "id=" << id
        << delimiter << msgHeader.ToString()
        << delimiter << "requestedSize=" << std::string(requestedSize, sizeof(requestedSize))
        << delimiter << "requestedMarketSide=" << requestedMarketSide
        ;
        return ss.str();
    }
};

//GR
struct GroupStatus {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6];
    char groupStatus;

    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetGroupStatus() const {
        return groupStatus;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "rootSymbol=" << std::string(rootSymbol, sizeof(rootSymbol))
        << delimiter << "groupStatus=" << groupStatus
        ;
        return ss.str();
    }
};

//GS
struct GroupStatusStrategies {
    LongMsgHeader msgHeader;
    char exchangeId;
    char instGroup[2];
    char groupStatus;

    std::string GetInstrumentGroup() const {
        return std::string(instGroup, sizeof(instGroup));
    }
    char GetGroupStatus() const {
        return groupStatus;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "instGroup=" << std::string(instGroup, sizeof(instGroup))
        << delimiter << "groupStatus=" << groupStatus
        ;
        return ss.str();
    }
};

struct Bond {
    char maturityDate[8]; 
    char coupon[7];
    char couponFI;
    char outstandingBondValue[8];
    char conversionFactor[7];
    char conversionFactorFI;

    std::string GetMaturityDate() const {
        return std::string(maturityDate, sizeof(maturityDate));
    }
    int64_t GetCoupon() const {
        return ConvertCharArray<int64_t>(coupon, 7);
    }
    char GetCouponFI() const {
        return couponFI;
    }
    int64_t GetOutstandingBondValue() const {
        return ConvertCharArray<int64_t>(outstandingBondValue, 8);
    }
    int64_t GetConversionFactor() const {
        return ConvertCharArray<int64_t>(conversionFactor, 7);
    }
    char GetConversionFactorFI() const {
        return conversionFactorFI;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << delimiter << "BondEntry -"
        << " maturityDate=" << std::string(maturityDate, sizeof(maturityDate))
        << ", coupon=" << std::string(coupon, sizeof(coupon))
        << ", couponFI=" << couponFI
        << ", outstandingBondValue=" << std::string(outstandingBondValue, sizeof(outstandingBondValue))
        << ", conversionFactor=" << std::string(conversionFactor, sizeof(conversionFactor))
        << ", conversionFactorFI=" << conversionFactorFI
        ;
        return ss.str();
    }
};

//KF
struct FutureDeliverables {
    LongMsgHeader msgHeader;
    char exchangeId;
    char rootSymbol[6]; 
    char symbolMonth;
    char symbolYear[2];
    char expiryDay[2];
    char numOfBonds[2];
    Bond bonds[20];

    std::string GetIdentifier() const {
        return GetRootSymbol()
                + GetSymbolMonth()
                + GetSymbolYear()
                + GetExpiryDay()
                ;
    }
    std::string GetRootSymbol() const {
        return std::string(rootSymbol, sizeof(rootSymbol));
    }
    char GetSymbolMonth() const {
        return symbolMonth;
    }
    std::string GetSymbolYear() const {
        return std::string(symbolYear, sizeof(symbolYear));
    }
    std::string GetExpiryDay() const {
        return std::string(expiryDay, sizeof(expiryDay));
    }
    int GetBondsNum() const {
        return ConvertCharArray<int>(numOfBonds, 2);
    }
    std::vector<Bond> GetBonds() const {
        std::vector<Bond> ret;
        const int num = GetBondsNum();
        for(int i = 0; i < num; ++i) {
            ret.push_back(bonds[i]);
        }
        return ret;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "rootSymbol=" << std::string(rootSymbol, sizeof(rootSymbol))
        << delimiter << "symbolMonth=" << symbolMonth
        << delimiter << "symbolYear=" << std::string(symbolYear, sizeof(symbolYear))
        << delimiter << "expiryDay=" << std::string(expiryDay, sizeof(expiryDay))
        << delimiter << "numOfBonds=" << std::string(numOfBonds, sizeof(numOfBonds))
        ;

        const int num = GetBondsNum();
        for(int i = 0; i < num; ++i) {
            ss << bonds[i].ToString();
        }
        return ss.str();
    }
};

struct TTEntry {
    char minPrice[7];
    char minPriceFractionIndicator;
    char tickPrice[7];
    char tickPriceFractionIndicator;

    int64_t GetMinPrice() const {
        return ConvertCharArray<int64_t>(minPrice, 7);
    }
    char GetMinPriceFractionIndicator() const {
        return minPriceFractionIndicator;
    }
    int64_t GetTickPrice() const {
        return ConvertCharArray<int64_t>(tickPrice, 7);
    }
    char GetTickPriceFractionIndicator() const {
        return tickPriceFractionIndicator;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << delimiter << "TTEntry -"
        << " minPrice=" << std::string(minPrice, sizeof(minPrice))
        << ", minPriceFractionIndicator=" << minPriceFractionIndicator
        << ", tickPrice=" << std::string(tickPrice, sizeof(tickPrice))
        << ", tickPriceFractionIndicator=" << tickPriceFractionIndicator
        ;
        return ss.str();
    }
};

//TT
struct TickTable {
    LongMsgHeader msgHeader;
    char exchangeId;
    char tickTableName[50];
    char tickTableShortName[2];
    char entriesNum[2];
    TTEntry entries[30];

    std::string GetTTName() const {
        return std::string(tickTableName, sizeof(tickTableName));
    }
    std::string GetTTShortName() const {
        return std::string(tickTableShortName, sizeof(tickTableShortName));
    }
    int GetEntriesNum() const {
        return ConvertCharArray<int>(entriesNum, 2);
    }
    std::vector<TTEntry> GetTTEntries() const {
        std::vector<TTEntry> ret;
        const int num = GetEntriesNum();
        for(int i = 0; i < num; ++i) {
            ret.push_back(entries[i]);
        }
        return ret;
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << delimiter << "exchangeId=" << exchangeId
        << delimiter << "tickTableName=" << std::string(tickTableName, sizeof(tickTableName))
        << delimiter << "tickTableShortName=" << std::string(tickTableShortName, sizeof(tickTableShortName))
        << delimiter << "entriesNum=" << GetEntriesNum()
        ;

        const int num = GetEntriesNum();
        for(int i = 0; i < num; ++i) {
            ss << entries[i].ToString();
        }
        return ss.str();
    }
};


//QF
struct BeginningOfFuturesSummary {
    LongMsgHeader msgHeader;
    char exchangeId;

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << ", exchangeId=" << exchangeId
        ;
        return ss.str();
    }
};

//Q
struct BeginningOfOptionsSummary {
    LongMsgHeader msgHeader;
    char exchangeId;

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << ", exchangeId=" << exchangeId
        ;
        return ss.str();
    }
};

//QB
struct BeginningOfFutureOptionsSummary {
    LongMsgHeader msgHeader;
    char exchangeId;

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << ", exchangeId=" << exchangeId
        ;
        return ss.str();
    }
};

//QS
struct BeginningOfStrategySummary {
    LongMsgHeader msgHeader;
    char exchangeId;

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << ", exchangeId=" << exchangeId
        ;
        return ss.str();
    }
};


//SD
struct StartOfDay {
    LongMsgHeader msgHeader;
    char exchangeId;
    char businessDate[8];

    std::string GetBusinessDate() const {
        return std::string(businessDate, sizeof(businessDate));
    }

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << ", exchangeId=" << exchangeId
        << ", businessDate=" << std::string(businessDate, sizeof(businessDate))
        ;
        return ss.str();
    }
};

//U
struct EndOfTransmission {
    LongMsgHeader msgHeader;
    char exchangeId;
    char time[6];

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << ", exchangeId=" << exchangeId
        << ", time=" << std::string(time, sizeof(time))
        ;
        return ss.str();
    }
};

//S
struct EndOfSales {
    LongMsgHeader msgHeader;
    char reserved;
    char time[6];

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << ", time=" << std::string(time, sizeof(time))
        ;
        return ss.str();
    }
};

//V
struct Heartbeat {
    LongMsgHeader msgHeader;
    char time[6];

    std::string ToString() const {
        std::stringstream ss;
        ss << msgHeader.ToString()
        << ", time=" << std::string(time, sizeof(time))
        ;
        return ss.str();
    }
};


}//end namespace

#endif
