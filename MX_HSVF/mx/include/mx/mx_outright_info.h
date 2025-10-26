#ifndef _MX_OUTRIGHT_INFO_H_
#define _MX_OUTRIGHT_INFO_H_

namespace ns {

struct OutrightInfo {
    OutrightInfo(std::string productSymbol, int64_t tickValueNumerator, uint16_t priceFactor, std::string currency, std::string securityId, const bool isOption) 
        : productSymbol(productSymbol),
          tickValueNumerator(tickValueNumerator),
          priceFactor(priceFactor),
          currency(currency), 
          securityId(securityId),
          isOption(isOption)
    {
    }
    std::string productSymbol;
    int64_t tickValueNumerator;
    uint16_t priceFactor;
    std::string currency;
    std::string securityId;
    bool isOption;
    
};//end class

}//end namespace

#endif
