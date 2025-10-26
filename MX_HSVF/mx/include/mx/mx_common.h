#ifndef _MX_COMMON_H_
#define _MX_COMMON_H_

#include <algorithm>

namespace ns {

constexpr char STX = 0x02;
constexpr char ETX = 0x03;

unsigned constexpr consthash(char const *input, unsigned hash = 5381) {
    return *input ?
        consthash(input + 1, hash * 33 + static_cast<unsigned>(*input)): 
        hash;
}

inline std::string GetString(std::string input) {
    input.erase(std::find(input.begin(), input.end(), ' '), input.end());
    return input;
}

inline std::string TrimRight(std::string input) {
    input.erase(std::find_if(input.rbegin(), input.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), input.end());
    return input;
}

inline uint32_t GetMultiplierFromIndicatorCode(const char marker) {
    switch(marker) {
        case 'C': return 100;
        case 'D': return 1000;
        case 'E': return 10000;
        case 'F': return 100000;
        case 'G': return 1000000;
        case 'H': return 10000000;
        case 'I': return 100000000;
        case 'J': return 100000000;
        default:
            assert(!"GetMultiplierFromIndicatorCode() - unhandled indicator code marker");
            return 1;
    }
}

inline double GetPrice(const int64_t price, const char fractionIndicator) {
    double fractionMultiplier = 1;

    switch(fractionIndicator) {
    case '0': fractionMultiplier = 1.0; break;
    case '1': fractionMultiplier = 1.0/10; break;
    case '2': fractionMultiplier = 1.0/100; break;
    case '3': fractionMultiplier = 1.0/1000; break;
    case '4': fractionMultiplier = 1.0/10000; break;
    case '5': fractionMultiplier = 1.0/100000; break;
    case '6': fractionMultiplier = 1.0/1000000; break;
    case '7': fractionMultiplier = 1.0/10000000; break;
    case '8': fractionMultiplier = 1.0/100000000; break;
    case '9': fractionMultiplier = 1.0/1000000000; break;
    case 'A': fractionMultiplier = -1.0; break;
    case 'B': fractionMultiplier = -1.0/10; break;
    case 'C': fractionMultiplier = -1.0/100; break;
    case 'D': fractionMultiplier = -1.0/1000; break;
    case 'E': fractionMultiplier = -1.0/10000; break;
    case 'F': fractionMultiplier = -1.0/100000; break;
    case 'G': fractionMultiplier = -1.0/1000000; break;
    case 'Z': fractionMultiplier = 10.0; break;
    case 'Y': fractionMultiplier = 100.0; break;
    case 'X': fractionMultiplier = 1000.0; break;
    case 'W': fractionMultiplier = 10000.0; break;
    case 'V': fractionMultiplier = 100000.0; break;
    case 'U': fractionMultiplier = 1000000.0; break;
    default:
        assert(!"GetPrice() - unhandled fraction indicator");
        break;
    }

    const double ret = price * fractionMultiplier;
    return ret;
}

inline int GetDecimalsToPrecision(const int decimals) {
    switch(decimals) {
    case 0: return 1;
    case 1: return 10;
    case 2: return 100;
    case 3: return 1000;
    case 4: return 10000;
    case 5: return 100000;
    case 6: return 1000000;
    case 7: return 10000000;
    case 8: return 100000000;
    case 9: return 1000000000;
    default:
        assert(!"GetDecimalsToPrecision() - unhandled value");
        return 1;
    }
}

inline void AdjustPrice(const std::string& identifier, const char fractionIndicator, int64_t& price, const std::unordered_map<std::string, int>& map) {
    auto it = map.find(identifier);
    if(std::end(map) == it){
        //noop if we cannot find it in the map
        return;
    }

    const int instrumentDecimals = it->second;
    const int msgDecimals = isalpha(fractionIndicator) ? fractionIndicator - 'A' : fractionIndicator - '0';
    const int diff = instrumentDecimals - msgDecimals;

    if(diff > 0) {
        price = price * GetDecimalsToPrecision(diff);
    } else if(diff < 0) {
        price = price / GetDecimalsToPrecision(-diff);
    }
}

inline std::string GetMonth(const char monthCode) {
    switch(monthCode) {
    case 'F': return "Jan";
    case 'G': return "Feb";
    case 'H': return "Mar";
    case 'J': return "Apr";
    case 'K': return "May";
    case 'M': return "Jun";
    case 'N': return "Jul";
    case 'Q': return "Aug";
    case 'U': return "Sep";
    case 'V': return "Oct";
    case 'X': return "Nov";
    case 'Z': return "Dec";
    default: {
        assert(!"GetMonth() - unhandled month code");
        return ""; 
    }    
    }
}

inline std::string GetMonthNumber(const char monthCode) {
    switch(monthCode) {
    case 'F': return "01";
    case 'G': return "02";
    case 'H': return "03";
    case 'J': return "04";
    case 'K': return "05";
    case 'M': return "06";
    case 'N': return "07";
    case 'Q': return "08";
    case 'U': return "09";
    case 'V': return "10";
    case 'X': return "11";
    case 'Z': return "12";
    default: {
        assert(!"GetMonthNumber() - unhandled month code");
        return ""; 
    }    
    }
}

inline int DecodeMonth(const char monthCode) {
    switch (monthCode) {
    case 'F': return 1;
    case 'G': return 2;
    case 'H': return 3;
    case 'J': return 4;
    case 'K': return 5;
    case 'M': return 6;
    case 'N': return 7;
    case 'Q': return 8;
    case 'U': return 9;
    case 'V': return 10;
    case 'X': return 11;
    case 'Z': return 12;
    default: {
        assert(!"DecodeMonth() - unhandled month code");
        return 0;
    }    
    }
}

inline int DecodeOptionMonth(const char monthCode) {
    switch (monthCode) {
    case 'A':
    case 'M': return 1;
    case 'B':
    case 'N': return 2;
    case 'C':
    case 'O': return 3;
    case 'D':
    case 'P': return 4;
    case 'E':
    case 'Q': return 5;
    case 'F':
    case 'R': return 6;
    case 'G':
    case 'S': return 7;
    case 'H':
    case 'T': return 8;
    case 'I':
    case 'U': return 9;
    case 'J':
    case 'V': return 10;
    case 'K':
    case 'W': return 11;
    case 'L':
    case 'X': return 12;
    default: {
        assert(!"DecodeOptionMonth() - unhandled month code");
        return 0;
    }
    }
}

inline int DecodeStrategyMonth(const char monthCode) {
    switch (monthCode) {
    case 'A': return 1;
    case 'B': return 2;
    case 'C': return 3;
    case 'D': return 4;
    case 'E': return 5;
    case 'F': return 6;
    case 'G': return 7;
    case 'H': return 8;
    case 'I': return 9;
    case 'J': return 10;
    case 'K': return 11;
    case 'L': return 12;
    default: {
        assert(!"DecodeStrategyMonth() - unhandled month code");
        return 0;
    }
    }
}

inline uint8_t GetQuarter(const int month) {
	if(month >= 1 && month <= 3) return 1;
	if(month >= 4 && month <= 6) return 2;
	if(month >= 7 && month <= 9) return 3;
    if(month >= 10 && month <= 12) return 4;

	assert(!"GetQuarter() - unhandled month");
	return 0;
}

inline DateTime GetExpiryDate(const std::string& expiryDate) {
    //expiryDate=240816
    assert(expiryDate.size() == 6);
    const int expiryYear = std::stoi(expiryDate.substr(0 ,2)) + 2000;
    const int expiryMonth = std::stoi(expiryDate.substr(2, 2));
    const int expiryDay = std::stoi(expiryDate.substr(4, 2));
    return DateTime(expiryMonth, expiryDay, expiryYear);
}

inline DateTime GetLTD(const std::string& lastTradingDate) {
    //ltd=20250321
    assert(lastTradingDate.size() == 8);
    const int expiryYear = std::stoi(lastTradingDate.substr(0 ,4));
    const int expiryMonth = std::stoi(lastTradingDate.substr(4, 2));
    const int expiryDay = std::stoi(lastTradingDate.substr(6, 2));
    return DateTime(expiryMonth, expiryDay, expiryYear);
}


}//end namespace

#endif

