#ifndef _MX_PRICE_INDICATOR_MARKERS_H_
#define _MX_PRICE_INDICATOR_MARKERS_H_

namespace ns {

enum PriceIndicatorMarker {
    AsOfTrade = 'A',
    BlockTrade = 'B',
    ActualTransactionTookPlace = ' ',
    Crossed = 'D',
    EFPReporting = 'E',
    ContingentTrade = 'G',
    RisklessBasisCross = 'H',
    ImpliedTrade = 'I',
    DeltaTrade = 'J',
    CommittedBlock = 'K',
    LateTrade = 'L',
    ForFutureUse1 = 'M',
    ForFutureUse2 = 'N',
    StrategyReporting = 'P',
    ForFutureUse3 = 'Q',
    EFRReporting = 'R',
    ReferencePrice = 'S',
    TradeCorrection = 't',
    Committed = 'T',
    BasisOnClose = 'U',
    PriceVolumeAdjustment = 'V',
    ForFutureUse4 = 'Y',
    ForFutureUse5 = 'Z',
};

enum StatusMarker {
    PreOpening = 'Y',
    Opening = 'O',
    ContinuousTrading = 'T',
    Forbidden = 'F',
    InterventionBeforeOpening = 'E',
    HaltedTrading = 'H',
    Reserved = 'R',
    Suspended = 'S',
    SurveillanceIntervention = 'A',
    EndOfDayInquiries = 'C',
    IfNotUsed = ' ',
};

enum ReasonMarker {
    EndOfDay = 'E',
    InstrumentNewOrUpdate = 'U',
    TradeCancellation = 'C',
};

}//end namespace

#endif
