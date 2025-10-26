/*******************************************************************************
 *
 *  FILE NAME: EOBILayouts.h
 *
 *  INTERFACE VERSION:   13.0
 *
 *  BUILD NUMBER:        130.540.0.ga-130005042-4
 *
 *  DESCRIPTION:
 *
 *    This header file documents the binary message format of EOBI.
 *    - All integers are in little endian byte order.
 *    - Padding bytes in following structures (char PadX[...]) are not required to be initialized.
 *
 *    DISCLAIMER:
 *
 *      Supported on Linux/x64 platforms with GNU C/C++ version 4.1 and 4.4.
 *
 *      This header file is meant to be compatible (but not supported) with any C/C++
 *      compiler/architecture that defines C99 compliant integer types in stdint.h and
 *      corresponds with the following alignment and padding requirements:
 *
 *    Padding:
 *      The compiler does not add implicit padding bytes between any of the following
 *      structure members. All padding bytes required for the alignment rules below are
 *      already explicitly contained in the structures.
 *
 *    Alignment rules:
 *      1 byte alignment for  int8_t and  uint8_t types
 *      2 byte alignment for int16_t and uint16_t types
 *      4 byte alignment for int32_t and uint32_t types
 *      8 byte alignment for int64_t and uint64_t types
 *
 *******************************************************************************/

#ifndef __EOBI_EOBI_LAYOUTS__
#define __EOBI_EOBI_LAYOUTS__

#include <stdint.h>

#if defined(__cplusplus) || defined(c_plusplus)
extern "C"
{
#endif

#define EOBI_INTERFACE_VERSION "13.0"
#define EOBI_BUILD_NUMBER      "130.540.0.ga-130005042-4"

/*
 * No Value defines
 */
#define NO_VALUE_SLONG                                   ((int64_t) 0x8000000000000000L)
#define NO_VALUE_ULONG                                   ((uint64_t) 0xffffffffffffffffUL)
#define NO_VALUE_SINT                                    ((int32_t) 0x80000000)
#define NO_VALUE_UINT                                    ((uint32_t) 0xffffffff)
#define NO_VALUE_SSHORT                                  ((int16_t) 0x8000)
#define NO_VALUE_USHORT                                  ((uint16_t) 0xffff)
#define NO_VALUE_SCHAR                                   ((int8_t) 0x80)
#define NO_VALUE_UCHAR                                   ((uint8_t) 0xff)
#define NO_VALUE_STR                                     0
#define NO_VALUE_DATA_16                                 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

/*
 * Template IDs defines
 */
#define	TID_ADD_COMPLEX_INSTRUMENT                       13400		// < AddComplexInstrument (Add Complex Instrument)
#define	TID_ADD_FLEXIBLE_INSTRUMENT                      13401		// < AddFlexibleInstrument (Add Flexible Instrument)
#define	TID_ADD_SCALED_SIMPLE_INSTRUMENT                 13402		// < AddScaledSimpleInstrument (Add Scaled Simple Instrument)
#define	TID_AUCTION_BBO                                  13500		// < AuctionBBO (Auction Best Bid/Offer)
#define	TID_AUCTION_CLEARING_PRICE                       13501		// < AuctionClearingPrice (Auction Clearing Price)
#define	TID_CROSS_REQUEST                                13502		// < CrossRequest (Cross Request)
#define	TID_EXECUTION_SUMMARY                            13202		// < ExecutionSummary (Execution Summary)
#define	TID_FULL_ORDER_EXECUTION                         13104		// < FullOrderExecution (Full Order Execution)
#define	TID_HEARTBEAT                                    13001		// < Heartbeat (Heartbeat)
#define	TID_INSTRUMENT_STATE_CHANGE                      13301		// < InstrumentStateChange (Instrument State Change)
#define	TID_INSTRUMENT_SUMMARY                           13601		// < InstrumentSummary (Instrument Summary)
#define	TID_MASS_INSTRUMENT_STATE_CHANGE                 13302		// < MassInstrumentStateChange (Mass Instrument State Change)
#define	TID_ORDER_ADD                                    13100		// < OrderAdd (Order Add)
#define	TID_ORDER_DELETE                                 13102		// < OrderDelete (Order Delete)
#define	TID_ORDER_MASS_DELETE                            13103		// < OrderMassDelete (Order Mass Delete)
#define	TID_ORDER_MODIFY                                 13101		// < OrderModify (Order Modify)
#define	TID_ORDER_MODIFY_SAME_PRIO                       13106		// < OrderModifySamePrio (Order Modify Same Priority)
#define	TID_PACKET_HEADER                                13004		// < PacketHeader (Packet Header)
#define	TID_PARTIAL_ORDER_EXECUTION                      13105		// < PartialOrderExecution (Partial Order Execution)
#define	TID_PRODUCT_STATE_CHANGE                         13300		// < ProductStateChange (Product State Change)
#define	TID_PRODUCT_SUMMARY                              13600		// < ProductSummary (Product Summary)
#define	TID_QUOTE_REQUEST                                13503		// < QuoteRequest (Quote Request)
#define	TID_SNAPSHOT_ORDER                               13602		// < SnapshotOrder (Snapshot Order)
#define	TID_TES_TRADE_REPORT                             13203		// < TESTradeReport (TES Trade Report)
#define	TID_TOP_OF_BOOK                                  13504		// < TopOfBook (Top of Book)
#define	TID_TRADE_REPORT                                 13201		// < TradeReport (Trade Report)
#define	TID_TRADE_REVERSAL                               13200		// < TradeReversal (Trade Reversal)

const int EOBI_EOBI_TID_MIN = 13001;  // lowest assigned template ID
const int EOBI_EOBI_TID_MAX = 13602;  // highest assigned template ID

/*
 * Max defines for sequences defines
 */
#define MAX_ADD_COMPLEX_INSTRUMENT_INSTRMT_LEG_GRP      	20
#define MAX_INSTRUMENT_SUMMARY_MD_INSTRUMENT_ENTRY_GRP  	15
#define MAX_MASS_INSTRUMENT_STATE_CHANGE_SEC_MASS_STAT_GRP	24
#define MAX_TRADE_REVERSAL_MD_TRADE_ENTRY_GRP           	15

/*
 * Data Type defines
 */

// DataType AggressorSide
#define ENUM_AGGRESSOR_SIDE_BUY                          1
#define ENUM_AGGRESSOR_SIDE_SELL                         2
#define ENUM_AGGRESSOR_SIDE_NO_VALUE                     ((uint8_t) 0xff)

// DataType AlgorithmicTradeIndicator
#define ENUM_ALGORITHMIC_TRADE_INDICATOR_ALGORITHMIC_TRADE 1
#define ENUM_ALGORITHMIC_TRADE_INDICATOR_NO_VALUE        ((uint8_t) 0xff)

// DataType ApplSeqResetIndicator
#define ENUM_APPL_SEQ_RESET_INDICATOR_NO_RESET           0
#define ENUM_APPL_SEQ_RESET_INDICATOR_RESET              1
#define ENUM_APPL_SEQ_RESET_INDICATOR_NO_VALUE           ((uint8_t) 0xff)

// DataType BidOrdType
#define ENUM_BID_ORD_TYPE_MARKET                         1
#define ENUM_BID_ORD_TYPE_NO_VALUE                       ((uint8_t) 0xff)

// DataType CompletionIndicator
#define ENUM_COMPLETION_INDICATOR_INCOMPLETE             0
#define ENUM_COMPLETION_INDICATOR_COMPLETE               1
#define ENUM_COMPLETION_INDICATOR_NO_VALUE               ((uint8_t) 0xff)

// DataType CrossRequestType
#define ENUM_CROSS_REQUEST_TYPE_CROSS_ANNOUNCEMENT       1
#define ENUM_CROSS_REQUEST_TYPE_LIQUIDITY_IMPROVEMENT_CROSS 2
#define ENUM_CROSS_REQUEST_TYPE_NO_VALUE                 ((uint8_t) 0xff)

// DataType ExerciseStyle
#define ENUM_EXERCISE_STYLE_EUROPEAN                     0
#define ENUM_EXERCISE_STYLE_AMERICAN                     1
#define ENUM_EXERCISE_STYLE_NO_VALUE                     ((uint8_t) 0xff)

// DataType FastMarketIndicator
#define ENUM_FAST_MARKET_INDICATOR_NO                    0
#define ENUM_FAST_MARKET_INDICATOR_YES                   1
#define ENUM_FAST_MARKET_INDICATOR_NO_VALUE              ((uint8_t) 0xff)

// DataType FuncCategory
#define LEN_FUNC_CATEGORY                                100
#define ENUM_FUNC_CATEGORY_GENERAL                       "General                                                                                             "
#define ENUM_FUNC_CATEGORY_ORDER_DATA                    "Order Data                                                                                          "
#define ENUM_FUNC_CATEGORY_TRADE_DATA                    "Trade Data                                                                                          "
#define ENUM_FUNC_CATEGORY_STATE_CHANGE                  "State Change                                                                                        "
#define ENUM_FUNC_CATEGORY_REFERENCE_DATA                "Reference Data                                                                                      "
#define ENUM_FUNC_CATEGORY_SNAPSHOT                      "Snapshot                                                                                            "
#define ENUM_FUNC_CATEGORY_NO_VALUE                      0

// DataType HHIIndicator
#define ENUM_HHI_INDICATOR_NO_UPDATE                     -1
#define ENUM_HHI_INDICATOR_NO_VALUE                      ((int8_t) 0x80)

// DataType ImpliedMarketIndicator
#define ENUM_IMPLIED_MARKET_INDICATOR_NOT_IMPLIED        0
#define ENUM_IMPLIED_MARKET_INDICATOR_IMPLIED_IN_OUT     3
#define ENUM_IMPLIED_MARKET_INDICATOR_NO_VALUE           ((uint8_t) 0xff)

// DataType InputSource
#define ENUM_INPUT_SOURCE_CLIP_CLIENT_BROKER             1
#define ENUM_INPUT_SOURCE_NO_VALUE                       ((uint8_t) 0xff)

// DataType InstrumentScopeProductComplex
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_SIMPLE_INSTRUMENT 1
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_STANDARD_OPTION_STRATEGY 2
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_NON_STANDARD_OPTION_STRATEGY 3
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_VOLATILITY_STRATEGY 4
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_FUTURES_SPREAD 5
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_INTER_PRODUCT_SPREAD 6
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_STANDARD_FUTURES_STRATEGY 7
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_PACK_AND_BUNDLE 8
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_STRIP      9
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_FLEXIBLE_INSTRUMENT 10
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_COMMODITY_STRIP 11
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_SCALED_SIMPLE_INSTRUMENT 12
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_NON_STANDARD_VOLATILITY_STRATEGY 13
#define ENUM_INSTRUMENT_SCOPE_PRODUCT_COMPLEX_NO_VALUE   ((uint8_t) 0xff)

// DataType LastFragment
#define ENUM_LAST_FRAGMENT_N                             0
#define ENUM_LAST_FRAGMENT_Y                             1
#define ENUM_LAST_FRAGMENT_NO_VALUE                      ((uint8_t) 0xff)

// DataType LegSecurityIDSource
#define LEN_LEG_SECURITY_ID_SOURCE                       1
#define ENUM_LEG_SECURITY_ID_SOURCE_MARKETPLACE          "M"
#define ENUM_LEG_SECURITY_ID_SOURCE_MARKETPLACE_CHAR     'M'
#define ENUM_LEG_SECURITY_ID_SOURCE_NO_VALUE             0

// DataType LegSecurityType
#define ENUM_LEG_SECURITY_TYPE_LEG_SECURITY_MULTI_LEG    1
#define ENUM_LEG_SECURITY_TYPE_LEG_SECURITY_UNDERLYING_LEG 2
#define ENUM_LEG_SECURITY_TYPE_NO_VALUE                  ((uint8_t) 0xff)

// DataType LegSide
#define ENUM_LEG_SIDE_BUY                                1
#define ENUM_LEG_SIDE_SELL                               2
#define ENUM_LEG_SIDE_NO_VALUE                           ((uint8_t) 0xff)

// DataType MDEntryType
#define ENUM_MD_ENTRY_TYPE_TRADE                         2
#define ENUM_MD_ENTRY_TYPE_OPENING_PRICE                 4
#define ENUM_MD_ENTRY_TYPE_CLOSING_PRICE                 5
#define ENUM_MD_ENTRY_TYPE_HIGH_PRICE                    7
#define ENUM_MD_ENTRY_TYPE_LOW_PRICE                     8
#define ENUM_MD_ENTRY_TYPE_TRADE_VOLUME                  66
#define ENUM_MD_ENTRY_TYPE_PREVIOUS_CLOSING_PRICE        101
#define ENUM_MD_ENTRY_TYPE_OPENING_AUCTION               200
#define ENUM_MD_ENTRY_TYPE_INTRADAY_AUCTION              201
#define ENUM_MD_ENTRY_TYPE_CIRCUIT_BREAKER_AUCTION       202
#define ENUM_MD_ENTRY_TYPE_CLOSING_AUCTION               203
#define ENUM_MD_ENTRY_TYPE_IPO_AUCTION                   204
#define ENUM_MD_ENTRY_TYPE_NO_VALUE                      ((uint8_t) 0xff)

// DataType MDOriginType
#define ENUM_MD_ORIGIN_TYPE_BOOK                         0
#define ENUM_MD_ORIGIN_TYPE_OFF_BOOK                     1
#define ENUM_MD_ORIGIN_TYPE_NO_VALUE                     ((uint8_t) 0xff)

// DataType MDReportEvent
#define ENUM_MD_REPORT_EVENT_SCOPE_DEFINITION            0
#define ENUM_MD_REPORT_EVENT_NO_VALUE                    ((uint8_t) 0xff)

// DataType MDUpdateAction
#define ENUM_MD_UPDATE_ACTION_NEW                        0
#define ENUM_MD_UPDATE_ACTION_CHANGE                     1
#define ENUM_MD_UPDATE_ACTION_DELETE                     2
#define ENUM_MD_UPDATE_ACTION_OVERLAY                    5
#define ENUM_MD_UPDATE_ACTION_NO_VALUE                   ((uint8_t) 0xff)

// DataType MarketCondition
#define ENUM_MARKET_CONDITION_NORMAL                     0
#define ENUM_MARKET_CONDITION_STRESSED                   1
#define ENUM_MARKET_CONDITION_NO_VALUE                   ((uint8_t) 0xff)

// DataType MarketDataType
#define ENUM_MARKET_DATA_TYPE_ORDER_BOOK_MAINTENANCE     1
#define ENUM_MARKET_DATA_TYPE_ORDER_BOOK_EXECUTION       2
#define ENUM_MARKET_DATA_TYPE_TRADE_REVERSAL             3
#define ENUM_MARKET_DATA_TYPE_TRADE_REPORT               4
#define ENUM_MARKET_DATA_TYPE_AUCTION_BBO                5
#define ENUM_MARKET_DATA_TYPE_AUCTION_CLEARING_PRICE     6
#define ENUM_MARKET_DATA_TYPE_CROSS_TRADE_ANNOUNCEMENT   7
#define ENUM_MARKET_DATA_TYPE_QUOTE_REQUEST              8
#define ENUM_MARKET_DATA_TYPE_MARKET_SEGMENT_SNAPSHOT    9
#define ENUM_MARKET_DATA_TYPE_SINGLE_INSTRUMENT_SNAPSHOT 10
#define ENUM_MARKET_DATA_TYPE_ORDER_BOOK_SNAPSHOT        11
#define ENUM_MARKET_DATA_TYPE_MATCH_EVENT                12
#define ENUM_MARKET_DATA_TYPE_TOP_OF_BOOK                13
#define ENUM_MARKET_DATA_TYPE_ADD_COMPLEX_INSTRUMENT     14
#define ENUM_MARKET_DATA_TYPE_TES_TRADE_REPORT           15
#define ENUM_MARKET_DATA_TYPE_HH_INDEX                   16
#define ENUM_MARKET_DATA_TYPE_ADD_FLEXIBLE_INSTRUMENT    17
#define ENUM_MARKET_DATA_TYPE_ADD_SCALED_SIMPLE_INSTRUMENT 18
#define ENUM_MARKET_DATA_TYPE_NO_VALUE                   ((uint8_t) 0xff)

// DataType MassMarketCondition
#define ENUM_MASS_MARKET_CONDITION_NORMAL                0
#define ENUM_MASS_MARKET_CONDITION_STRESSED              1
#define ENUM_MASS_MARKET_CONDITION_NO_VALUE              ((uint8_t) 0xff)

// DataType MassSoldOutIndicator
#define ENUM_MASS_SOLD_OUT_INDICATOR_SOLD_OUT            1
#define ENUM_MASS_SOLD_OUT_INDICATOR_NO_VALUE            ((uint8_t) 0xff)

// DataType MatchSubType
#define ENUM_MATCH_SUB_TYPE_OPENING_AUCTION              1
#define ENUM_MATCH_SUB_TYPE_CLOSING_AUCTION              2
#define ENUM_MATCH_SUB_TYPE_INTRADAY_AUCTION             3
#define ENUM_MATCH_SUB_TYPE_CIRCUIT_BREAKER_AUCTION      4
#define ENUM_MATCH_SUB_TYPE_IPO_AUCTION                  5
#define ENUM_MATCH_SUB_TYPE_NO_VALUE                     ((uint8_t) 0xff)

// DataType MatchType
#define ENUM_MATCH_TYPE_CONFIRMED_TRADE_REPORT           3
#define ENUM_MATCH_TYPE_CROSS_AUCTION                    5
#define ENUM_MATCH_TYPE_CALL_AUCTION                     7
#define ENUM_MATCH_TYPE_LIQUIDITY_IMPROVEMENT_CROSS      13
#define ENUM_MATCH_TYPE_CONTINUOUS_AUCTION               14
#define ENUM_MATCH_TYPE_NO_VALUE                         ((uint8_t) 0xff)

// DataType MsgType
#define LEN_MSG_TYPE                                     3
#define ENUM_MSG_TYPE_MARKET_DATA_REPORT                 "U20"
#define ENUM_MSG_TYPE_HEARTBEAT                          "0  "
#define ENUM_MSG_TYPE_MARKET_DATA_ORDER                  "U21"
#define ENUM_MSG_TYPE_MARKET_DATA_TRADE                  "U22"
#define ENUM_MSG_TYPE_TRADING_SESSION_STATUS             "h  "
#define ENUM_MSG_TYPE_SECURITY_STATUS                    "f  "
#define ENUM_MSG_TYPE_SECURITY_MASS_STATUS               "CO "
#define ENUM_MSG_TYPE_SECURITY_DEFINITION_UPDATE_REPORT  "BP "
#define ENUM_MSG_TYPE_MARKET_DATA_INSTRUMENT             "U23"
#define ENUM_MSG_TYPE_NO_VALUE                           0

// DataType MultiLegPriceModel
#define ENUM_MULTI_LEG_PRICE_MODEL_STANDARD              0
#define ENUM_MULTI_LEG_PRICE_MODEL_USER_DEFINED          1
#define ENUM_MULTI_LEG_PRICE_MODEL_NO_VALUE              ((uint8_t) 0xff)

// DataType MultiLegReportingType
#define ENUM_MULTI_LEG_REPORTING_TYPE_SINGLE_SECURITY    1
#define ENUM_MULTI_LEG_REPORTING_TYPE_INDIVIDUAL_LEG_OFA_MULTI_LEG_SECURITY 2
#define ENUM_MULTI_LEG_REPORTING_TYPE_MULTI_LEG_SECURITY 3
#define ENUM_MULTI_LEG_REPORTING_TYPE_NO_VALUE           ((uint8_t) 0xff)

// DataType NoMarketSegments
#define ENUM_NO_MARKET_SEGMENTS_ONE                      1
#define ENUM_NO_MARKET_SEGMENTS_NO_VALUE                 ((uint8_t) 0xff)

// DataType OfferOrdType
#define ENUM_OFFER_ORD_TYPE_MARKET                       1
#define ENUM_OFFER_ORD_TYPE_NO_VALUE                     ((uint8_t) 0xff)

// DataType OrdType
#define ENUM_ORD_TYPE_MARKET                             1
#define ENUM_ORD_TYPE_NO_VALUE                           ((uint8_t) 0xff)

// DataType Pad1
#define LEN_PAD1                                         1

// DataType Pad2
#define LEN_PAD2                                         2

// DataType Pad3
#define LEN_PAD3                                         3

// DataType Pad4
#define LEN_PAD4                                         4

// DataType Pad5
#define LEN_PAD5                                         5

// DataType Pad6
#define LEN_PAD6                                         6

// DataType Pad7
#define LEN_PAD7                                         7

// DataType PotentialSecurityTradingEvent
#define ENUM_POTENTIAL_SECURITY_TRADING_EVENT_NONE       0
#define ENUM_POTENTIAL_SECURITY_TRADING_EVENT_PRICE_VOLATILITY_AUCTION_IS_EXTENDED 10
#define ENUM_POTENTIAL_SECURITY_TRADING_EVENT_NO_VALUE   ((uint8_t) 0xff)

// DataType PrevPriceHHIIndicator
#define ENUM_PREV_PRICE_HHI_INDICATOR_NO_UPDATE          -1
#define ENUM_PREV_PRICE_HHI_INDICATOR_NO_VALUE           ((int8_t) 0x80)

// DataType ProductComplex
#define ENUM_PRODUCT_COMPLEX_SIMPLE_INSTRUMENT           1
#define ENUM_PRODUCT_COMPLEX_STANDARD_OPTION_STRATEGY    2
#define ENUM_PRODUCT_COMPLEX_NON_STANDARD_OPTION_STRATEGY 3
#define ENUM_PRODUCT_COMPLEX_VOLATILITY_STRATEGY         4
#define ENUM_PRODUCT_COMPLEX_FUTURES_SPREAD              5
#define ENUM_PRODUCT_COMPLEX_INTER_PRODUCT_SPREAD        6
#define ENUM_PRODUCT_COMPLEX_STANDARD_FUTURES_STRATEGY   7
#define ENUM_PRODUCT_COMPLEX_PACK_AND_BUNDLE             8
#define ENUM_PRODUCT_COMPLEX_STRIP                       9
#define ENUM_PRODUCT_COMPLEX_FLEXIBLE_INSTRUMENT         10
#define ENUM_PRODUCT_COMPLEX_COMMODITY_STRIP             11
#define ENUM_PRODUCT_COMPLEX_SCALED_SIMPLE_INSTRUMENT    12
#define ENUM_PRODUCT_COMPLEX_NON_STANDARD_VOLATILITY_STRATEGY 13
#define ENUM_PRODUCT_COMPLEX_NO_VALUE                    ((uint8_t) 0xff)

// DataType PutOrCall
#define ENUM_PUT_OR_CALL_PUT                             0
#define ENUM_PUT_OR_CALL_CALL                            1
#define ENUM_PUT_OR_CALL_NO_VALUE                        ((uint8_t) 0xff)

// DataType RelatedSecurityIDSource
#define LEN_RELATED_SECURITY_ID_SOURCE                   1
#define ENUM_RELATED_SECURITY_ID_SOURCE_MARKETPLACE      "M"
#define ENUM_RELATED_SECURITY_ID_SOURCE_MARKETPLACE_CHAR 'M'
#define ENUM_RELATED_SECURITY_ID_SOURCE_NO_VALUE         0

// DataType SecurityDesc
#define LEN_SECURITY_DESC                                40

// DataType SecurityIDSource
#define LEN_SECURITY_ID_SOURCE                           1
#define ENUM_SECURITY_ID_SOURCE_MARKETPLACE              "M"
#define ENUM_SECURITY_ID_SOURCE_MARKETPLACE_CHAR         'M'
#define ENUM_SECURITY_ID_SOURCE_NO_VALUE                 0

// DataType SecurityMassStatus
#define ENUM_SECURITY_MASS_STATUS_ACTIVE                 1
#define ENUM_SECURITY_MASS_STATUS_INACTIVE               2
#define ENUM_SECURITY_MASS_STATUS_EXPIRED                4
#define ENUM_SECURITY_MASS_STATUS_KNOCKED_OUT            6
#define ENUM_SECURITY_MASS_STATUS_KNOCK_OUT_REVOKED      7
#define ENUM_SECURITY_MASS_STATUS_SUSPENDED              9
#define ENUM_SECURITY_MASS_STATUS_PENDING_DELETION       11
#define ENUM_SECURITY_MASS_STATUS_KNOCKED_OUT_AND_SUSPENDED 12
#define ENUM_SECURITY_MASS_STATUS_NO_VALUE               ((uint8_t) 0xff)

// DataType SecurityMassTradingEvent
#define ENUM_SECURITY_MASS_TRADING_EVENT_PRICE_VOLATILITY_AUCTION_IS_EXTENDED 10
#define ENUM_SECURITY_MASS_TRADING_EVENT_PRICE_VOLATILITY_AUCTION_IS_EXTENDED_AGAIN 11
#define ENUM_SECURITY_MASS_TRADING_EVENT_NO_VALUE        ((uint8_t) 0xff)

// DataType SecurityMassTradingStatus
#define ENUM_SECURITY_MASS_TRADING_STATUS_TRADING_HALT   2
#define ENUM_SECURITY_MASS_TRADING_STATUS_MARKET_IMBALANCE_BUY 7
#define ENUM_SECURITY_MASS_TRADING_STATUS_MARKET_IMBALANCE_SELL 8
#define ENUM_SECURITY_MASS_TRADING_STATUS_CLOSED         200
#define ENUM_SECURITY_MASS_TRADING_STATUS_RESTRICTED     201
#define ENUM_SECURITY_MASS_TRADING_STATUS_BOOK           202
#define ENUM_SECURITY_MASS_TRADING_STATUS_CONTINUOUS     203
#define ENUM_SECURITY_MASS_TRADING_STATUS_OPENING_AUCTION 204
#define ENUM_SECURITY_MASS_TRADING_STATUS_OPENING_AUCTION_FREEZE 205
#define ENUM_SECURITY_MASS_TRADING_STATUS_INTRADAY_AUCTION 206
#define ENUM_SECURITY_MASS_TRADING_STATUS_INTRADAY_AUCTION_FREEZE 207
#define ENUM_SECURITY_MASS_TRADING_STATUS_CIRCUIT_BREAKER_AUCTION 208
#define ENUM_SECURITY_MASS_TRADING_STATUS_CIRCUIT_BREAKER_AUCTION_FREEZE 209
#define ENUM_SECURITY_MASS_TRADING_STATUS_CLOSING_AUCTION 210
#define ENUM_SECURITY_MASS_TRADING_STATUS_CLOSING_AUCTION_FREEZE 211
#define ENUM_SECURITY_MASS_TRADING_STATUS_IPO_AUCTION    212
#define ENUM_SECURITY_MASS_TRADING_STATUS_IPO_AUCTION_FREEZE 213
#define ENUM_SECURITY_MASS_TRADING_STATUS_PRE_CALL       214
#define ENUM_SECURITY_MASS_TRADING_STATUS_CALL           215
#define ENUM_SECURITY_MASS_TRADING_STATUS_FREEZE         216
#define ENUM_SECURITY_MASS_TRADING_STATUS_TRADE_AT_CLOSE 217
#define ENUM_SECURITY_MASS_TRADING_STATUS_CIRCUIT_BREAKER_AUCTION_TRIGGERED_BY_STATIC_LIMIT_BREACH 220
#define ENUM_SECURITY_MASS_TRADING_STATUS_CIRCUIT_BREAKER_AUCTION_TRIGGERED_BY_STATIC_LIMIT_BREACH_FREEZE 221
#define ENUM_SECURITY_MASS_TRADING_STATUS_NO_VALUE       ((uint8_t) 0xff)

// DataType SecurityStatus
#define ENUM_SECURITY_STATUS_ACTIVE                      1
#define ENUM_SECURITY_STATUS_INACTIVE                    2
#define ENUM_SECURITY_STATUS_EXPIRED                     4
#define ENUM_SECURITY_STATUS_KNOCKED_OUT                 6
#define ENUM_SECURITY_STATUS_KNOCK_OUT_REVOKED           7
#define ENUM_SECURITY_STATUS_SUSPENDED                   9
#define ENUM_SECURITY_STATUS_PENDING_DELETION            11
#define ENUM_SECURITY_STATUS_KNOCKED_OUT_AND_SUSPENDED   12
#define ENUM_SECURITY_STATUS_NO_VALUE                    ((uint8_t) 0xff)

// DataType SecurityTradingEvent
#define ENUM_SECURITY_TRADING_EVENT_PRICE_VOLATILITY_AUCTION_IS_EXTENDED 10
#define ENUM_SECURITY_TRADING_EVENT_PRICE_VOLATILITY_AUCTION_IS_EXTENDED_AGAIN 11
#define ENUM_SECURITY_TRADING_EVENT_NO_VALUE             ((uint8_t) 0xff)

// DataType SecurityTradingStatus
#define ENUM_SECURITY_TRADING_STATUS_TRADING_HALT        2
#define ENUM_SECURITY_TRADING_STATUS_MARKET_IMBALANCE_BUY 7
#define ENUM_SECURITY_TRADING_STATUS_MARKET_IMBALANCE_SELL 8
#define ENUM_SECURITY_TRADING_STATUS_CLOSED              200
#define ENUM_SECURITY_TRADING_STATUS_RESTRICTED          201
#define ENUM_SECURITY_TRADING_STATUS_BOOK                202
#define ENUM_SECURITY_TRADING_STATUS_CONTINUOUS          203
#define ENUM_SECURITY_TRADING_STATUS_OPENING_AUCTION     204
#define ENUM_SECURITY_TRADING_STATUS_OPENING_AUCTION_FREEZE 205
#define ENUM_SECURITY_TRADING_STATUS_INTRADAY_AUCTION    206
#define ENUM_SECURITY_TRADING_STATUS_INTRADAY_AUCTION_FREEZE 207
#define ENUM_SECURITY_TRADING_STATUS_CIRCUIT_BREAKER_AUCTION 208
#define ENUM_SECURITY_TRADING_STATUS_CIRCUIT_BREAKER_AUCTION_FREEZE 209
#define ENUM_SECURITY_TRADING_STATUS_CLOSING_AUCTION     210
#define ENUM_SECURITY_TRADING_STATUS_CLOSING_AUCTION_FREEZE 211
#define ENUM_SECURITY_TRADING_STATUS_IPO_AUCTION         212
#define ENUM_SECURITY_TRADING_STATUS_IPO_AUCTION_FREEZE  213
#define ENUM_SECURITY_TRADING_STATUS_PRE_CALL            214
#define ENUM_SECURITY_TRADING_STATUS_CALL                215
#define ENUM_SECURITY_TRADING_STATUS_FREEZE              216
#define ENUM_SECURITY_TRADING_STATUS_TRADE_AT_CLOSE      217
#define ENUM_SECURITY_TRADING_STATUS_CIRCUIT_BREAKER_AUCTION_TRIGGERED_BY_STATIC_LIMIT_BREACH 220
#define ENUM_SECURITY_TRADING_STATUS_CIRCUIT_BREAKER_AUCTION_TRIGGERED_BY_STATIC_LIMIT_BREACH_FREEZE 221
#define ENUM_SECURITY_TRADING_STATUS_NO_VALUE            ((uint8_t) 0xff)

// DataType SecurityType
#define ENUM_SECURITY_TYPE_OPT                           1
#define ENUM_SECURITY_TYPE_FUT                           2
#define ENUM_SECURITY_TYPE_MLEG                          3
#define ENUM_SECURITY_TYPE_NO_VALUE                      ((uint8_t) 0xff)

// DataType SecurityUpdateAction
#define LEN_SECURITY_UPDATE_ACTION                       1
#define ENUM_SECURITY_UPDATE_ACTION_ADD                  "A"
#define ENUM_SECURITY_UPDATE_ACTION_ADD_CHAR             'A'
#define ENUM_SECURITY_UPDATE_ACTION_NO_VALUE             0

// DataType SettlMethod
#define ENUM_SETTL_METHOD_CASH                           0
#define ENUM_SETTL_METHOD_PHYSICAL                       1
#define ENUM_SETTL_METHOD_NO_VALUE                       ((uint8_t) 0xff)

// DataType Side
#define ENUM_SIDE_BUY                                    1
#define ENUM_SIDE_SELL                                   2
#define ENUM_SIDE_NO_VALUE                               ((uint8_t) 0xff)

// DataType SoldOutIndicator
#define ENUM_SOLD_OUT_INDICATOR_SOLD_OUT                 1
#define ENUM_SOLD_OUT_INDICATOR_NO_VALUE                 ((uint8_t) 0xff)

// DataType TESSecurityMassStatus
#define ENUM_TES_SECURITY_MASS_STATUS_ACTIVE             1
#define ENUM_TES_SECURITY_MASS_STATUS_INACTIVE           2
#define ENUM_TES_SECURITY_MASS_STATUS_EXPIRED            4
#define ENUM_TES_SECURITY_MASS_STATUS_SUSPENDED          9
#define ENUM_TES_SECURITY_MASS_STATUS_NO_VALUE           ((uint8_t) 0xff)

// DataType TESSecurityStatus
#define ENUM_TES_SECURITY_STATUS_ACTIVE                  1
#define ENUM_TES_SECURITY_STATUS_INACTIVE                2
#define ENUM_TES_SECURITY_STATUS_EXPIRED                 4
#define ENUM_TES_SECURITY_STATUS_SUSPENDED               9
#define ENUM_TES_SECURITY_STATUS_NO_VALUE                ((uint8_t) 0xff)

// DataType TESTradSesStatus
#define ENUM_TES_TRAD_SES_STATUS_HALTED                  1
#define ENUM_TES_TRAD_SES_STATUS_OPEN                    2
#define ENUM_TES_TRAD_SES_STATUS_CLOSED                  3
#define ENUM_TES_TRAD_SES_STATUS_PRE_CLOSE               5
#define ENUM_TES_TRAD_SES_STATUS_NO_VALUE                ((uint8_t) 0xff)

// DataType TradSesEvent
#define ENUM_TRAD_SES_EVENT_TBD                          0
#define ENUM_TRAD_SES_EVENT_STATUS_CHANGE                3
#define ENUM_TRAD_SES_EVENT_NO_VALUE                     ((uint8_t) 0xff)

// DataType TradSesStatus
#define ENUM_TRAD_SES_STATUS_HALTED                      1
#define ENUM_TRAD_SES_STATUS_OPEN                        2
#define ENUM_TRAD_SES_STATUS_CLOSED                      3
#define ENUM_TRAD_SES_STATUS_NO_VALUE                    ((uint8_t) 0xff)

// DataType TradeCondition
#define ENUM_TRADE_CONDITION_IMPLIED_TRADE               1
#define ENUM_TRADE_CONDITION_OUT_OF_SEQUENCE             107
#define ENUM_TRADE_CONDITION_MIDPOINT_PRICE              155
#define ENUM_TRADE_CONDITION_TRADING_ON_TERMS_OF_ISSUE   156
#define ENUM_TRADE_CONDITION_SPECIAL_AUCTION             596
#define ENUM_TRADE_CONDITION_TRADE_AT_CLOSE              624
#define ENUM_TRADE_CONDITION_RETAIL                      743
#define ENUM_TRADE_CONDITION_NO_VALUE                    ((uint16_t) 0xffff)

// DataType TradingSessionID
#define ENUM_TRADING_SESSIONID_DAY                       1
#define ENUM_TRADING_SESSIONID_MORNING                   3
#define ENUM_TRADING_SESSIONID_EVENING                   5
#define ENUM_TRADING_SESSIONID_AFTER_HOURS               6
#define ENUM_TRADING_SESSIONID_HOLIDAY                   7
#define ENUM_TRADING_SESSIONID_NO_VALUE                  ((uint8_t) 0xff)

// DataType TradingSessionSubID
#define ENUM_TRADING_SESSION_SUBID_PRE_TRADING           1
#define ENUM_TRADING_SESSION_SUBID_CONTINUOUS            3
#define ENUM_TRADING_SESSION_SUBID_CLOSING               4
#define ENUM_TRADING_SESSION_SUBID_POST_TRADING          5
#define ENUM_TRADING_SESSION_SUBID_QUIESCENT             7
#define ENUM_TRADING_SESSION_SUBID_NO_VALUE              ((uint8_t) 0xff)

// DataType TrdType
#define ENUM_TRD_TYPE_BLOCK_TRADE                        1
#define ENUM_TRD_TYPE_EFP                                2
#define ENUM_TRD_TYPE_EFS                                12
#define ENUM_TRD_TYPE_OTC                                54
#define ENUM_TRD_TYPE_EXCHANGE_BASIS_FACILITY            55
#define ENUM_TRD_TYPE_VOLA_TRADE                         1000
#define ENUM_TRD_TYPE_EFP_FIN_TRADE                      1001
#define ENUM_TRD_TYPE_EFP_INDEX_FUTURES_TRADE            1002
#define ENUM_TRD_TYPE_BLOCK_TRADE_AT_MARKET              1004
#define ENUM_TRD_TYPE_XETRA_EUREX_ENLIGHT_TRIGGERED_TRADE 1006
#define ENUM_TRD_TYPE_BLOCK_QTP_IP_TRADE                 1007
#define ENUM_TRD_TYPE_DELTA_TRADE_AT_MARKET              1017
#define ENUM_TRD_TYPE_NO_VALUE                           ((uint16_t) 0xffff)

/*
 * Structure defines for components and sequences
 */

// Structure: InstrmtLegGrp
typedef struct
{
    int32_t LegSymbol;
    char Pad4[LEN_PAD4];
    int64_t LegSecurityID;
    int64_t LegPrice;
    int32_t LegRatioQty;
    uint8_t LegSecurityType;
    uint8_t LegSide;
    char Pad2[LEN_PAD2];
} InstrmtLegGrpSeqT;

// Structure: MDInstrumentEntryGrp
typedef struct
{
    int64_t MDEntryPx;
    int64_t MDEntrySize;
    uint8_t MDOriginType;
    uint8_t MDEntryType;
    uint16_t TradeCondition;
    uint16_t TrdType;
    uint8_t MultiLegReportingType;
    uint8_t MultiLegPriceModel;
    int64_t NonDisclosedTradeVolume;
} MDInstrumentEntryGrpSeqT;

// Structure: MDTradeEntryGrp
typedef struct
{
    int64_t MDEntryPx;
    int64_t MDEntrySize;
    uint8_t MDEntryType;
    char Pad7[LEN_PAD7];
} MDTradeEntryGrpSeqT;

// Structure: MessageHeader
typedef struct
{
    uint16_t BodyLen;
    uint16_t TemplateID;
    uint32_t MsgSeqNum;
} MessageHeaderCompT;

// Structure: OrderDetails
typedef struct
{
    uint64_t TrdRegTSTimePriority;
    int64_t DisplayQty;
    uint8_t Side;
    uint8_t OrdType;
    int8_t HHIIndicator;
    char Pad5[LEN_PAD5];
    int64_t Price;
} OrderDetailsCompT;

// Structure: RelatedInstrumentGrp
typedef struct
{
    int64_t RelatedSecurityID;
} RelatedInstrumentGrpCompT;

// Structure: RemainingOrderDetails
typedef struct
{
    uint64_t TrdRegTSPrevTimePriority;
    int64_t DisplayQty;
    int64_t Price;
} RemainingOrderDetailsCompT;

// Structure: SecMassStatGrp
typedef struct
{
    int64_t SecurityID;
    int64_t HighPx;
    int64_t LowPx;
    uint8_t SecurityStatus;
    uint8_t SecurityTradingStatus;
    uint8_t MarketCondition;
    uint8_t SecurityTradingEvent;
    uint8_t SoldOutIndicator;
    uint8_t TESSecurityStatus;
    char Pad2[LEN_PAD2];
} SecMassStatGrpSeqT;

/*
 * Structure defines for messages
 */

// Message:	    AddComplexInstrument
// TemplateID:  13400
// Alias:       Add Complex Instrument
// FIX MsgType: SecurityDefinitionUpdateReport = "BP"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t TransactTime;
    char SecurityDesc[LEN_SECURITY_DESC];
    int32_t SecuritySubType;
    uint8_t ProductComplex;
    uint8_t ImpliedMarketIndicator;
    uint16_t QuantityScalingFactor;
    uint32_t LegRatioMultiplier;
    uint8_t NoLegs;
    char Pad2[LEN_PAD2];
    uint8_t LastFragment;
    InstrmtLegGrpSeqT InstrmtLegGrp[MAX_ADD_COMPLEX_INSTRUMENT_INSTRMT_LEG_GRP];
} AddComplexInstrumentT;

// Message:	    AddFlexibleInstrument
// TemplateID:  13401
// Alias:       Add Flexible Instrument
// FIX MsgType: SecurityDefinitionUpdateReport = "BP"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t TransactTime;
    char SecurityDesc[LEN_SECURITY_DESC];
    uint8_t SecurityType;
    uint8_t PutOrCall;
    uint8_t ExerciseStyle;
    uint8_t SettlMethod;
    uint32_t MaturityDate;
    int64_t StrikePrice;
    uint32_t OptAttribute;
    uint32_t ContractDate;
    uint32_t SecurityReferenceDataSupplement;
    char Pad4[LEN_PAD4];
} AddFlexibleInstrumentT;

// Message:	    AddScaledSimpleInstrument
// TemplateID:  13402
// Alias:       Add Scaled Simple Instrument
// FIX MsgType: SecurityDefinitionUpdateReport = "BP"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t TransactTime;
    char SecurityDesc[LEN_SECURITY_DESC];
    uint8_t SecurityType;
    char Pad1[LEN_PAD1];
    uint16_t QuantityScalingFactor;
    char Pad4[LEN_PAD4];
    RelatedInstrumentGrpCompT RelatedInstrumentGrp;
} AddScaledSimpleInstrumentT;

// Message:	    AuctionBBO
// TemplateID:  13500
// Alias:       Auction Best Bid/Offer
// FIX MsgType: MarketDataInstrument = "U23"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint64_t TransactTime;
    int64_t SecurityID;
    int64_t BidPx;
    int64_t OfferPx;
    int64_t BidSize;
    int64_t OfferSize;
    uint8_t PotentialSecurityTradingEvent;
    uint8_t BidOrdType;
    uint8_t OfferOrdType;
    char Pad5[LEN_PAD5];
} AuctionBBOT;

// Message:	    AuctionClearingPrice
// TemplateID:  13501
// Alias:       Auction Clearing Price
// FIX MsgType: MarketDataInstrument = "U23"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint64_t TransactTime;
    int64_t SecurityID;
    int64_t LastPx;
    int64_t LastQty;
    int64_t ImbalanceQty;
    uint8_t SecurityTradingStatus;
    uint8_t PotentialSecurityTradingEvent;
    char Pad6[LEN_PAD6];
} AuctionClearingPriceT;

// Message:	    CrossRequest
// TemplateID:  13502
// Alias:       Cross Request
// FIX MsgType: MarketDataInstrument = "U23"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    int64_t LastPx;
    int64_t LastQty;
    uint8_t Side;
    uint8_t CrossRequestType;
    uint8_t InputSource;
    char Pad5[LEN_PAD5];
    uint64_t TransactTime;
} CrossRequestT;

// Message:	    ExecutionSummary
// TemplateID:  13202
// Alias:       Execution Summary
// FIX MsgType: MarketDataTrade = "U22"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t RequestTime;
    uint64_t ExecID;
    int64_t LastQty;
    uint8_t AggressorSide;
    char Pad1[LEN_PAD1];
    uint16_t TradeCondition;
    uint8_t TradingHHIIndicator;
    char Pad3[LEN_PAD3];
    int64_t LastPx;
    RemainingOrderDetailsCompT RemainingOrderDetails;
    int64_t RestingHiddenQty;
    int64_t RestingCxlQty;
    uint64_t AggressorTime;
} ExecutionSummaryT;

// Message:	    FullOrderExecution
// TemplateID:  13104
// Alias:       Full Order Execution
// FIX MsgType: MarketDataOrder = "U21"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint8_t Side;
    uint8_t OrdType;
    uint8_t AlgorithmicTradeIndicator;
    int8_t HHIIndicator;
    uint32_t TrdMatchID;
    int64_t Price;
    uint64_t TrdRegTSTimePriority;
    int64_t SecurityID;
    int64_t LastQty;
    int64_t LastPx;
} FullOrderExecutionT;

// Message:	    Heartbeat
// TemplateID:  13001
// Alias:       Heartbeat
// FIX MsgType: Heartbeat = "0"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint32_t LastMsgSeqNumProcessed;
    char Pad4[LEN_PAD4];
} HeartbeatT;

// Message:	    InstrumentStateChange
// TemplateID:  13301
// Alias:       Instrument State Change
// FIX MsgType: SecurityStatus = "f"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint8_t SecurityStatus;
    uint8_t SecurityTradingStatus;
    uint8_t MarketCondition;
    uint8_t FastMarketIndicator;
    uint8_t SecurityTradingEvent;
    uint8_t SoldOutIndicator;
    char Pad2[LEN_PAD2];
    int64_t HighPx;
    int64_t LowPx;
    uint64_t TransactTime;
    uint8_t TESSecurityStatus;
    char Pad7[LEN_PAD7];
} InstrumentStateChangeT;

// Message:	    InstrumentSummary
// TemplateID:  13601
// Alias:       Instrument Summary
// FIX MsgType: MarketDataInstrument = "U23"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t LastUpdateTime;
    uint64_t TrdRegTSExecutionTime;
    uint16_t TotNoOrders;
    uint8_t SecurityStatus;
    uint8_t SecurityTradingStatus;
    uint8_t MarketCondition;
    uint8_t FastMarketIndicator;
    uint8_t SecurityTradingEvent;
    uint8_t SoldOutIndicator;
    int64_t HighPx;
    int64_t LowPx;
    uint8_t ProductComplex;
    uint8_t NoMDEntries;
    uint8_t TESSecurityStatus;
    char Pad5[LEN_PAD5];
    MDInstrumentEntryGrpSeqT MDInstrumentEntryGrp[MAX_INSTRUMENT_SUMMARY_MD_INSTRUMENT_ENTRY_GRP];
} InstrumentSummaryT;

// Message:	    MassInstrumentStateChange
// TemplateID:  13302
// Alias:       Mass Instrument State Change
// FIX MsgType: SecurityMassStatus = "CO"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint8_t InstrumentScopeProductComplex;
    uint8_t SecurityMassStatus;
    uint8_t SecurityMassTradingStatus;
    uint8_t MassMarketCondition;
    uint8_t FastMarketIndicator;
    uint8_t SecurityMassTradingEvent;
    uint8_t MassSoldOutIndicator;
    uint8_t TESSecurityMassStatus;
    uint64_t TransactTime;
    uint8_t LastFragment;
    uint8_t NoRelatedSym;
    char Pad6[LEN_PAD6];
    SecMassStatGrpSeqT SecMassStatGrp[MAX_MASS_INSTRUMENT_STATE_CHANGE_SEC_MASS_STAT_GRP];
} MassInstrumentStateChangeT;

// Message:	    OrderAdd
// TemplateID:  13100
// Alias:       Order Add
// FIX MsgType: MarketDataOrder = "U21"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint64_t RequestTime;
    int64_t SecurityID;
    OrderDetailsCompT OrderDetails;
} OrderAddT;

// Message:	    OrderDelete
// TemplateID:  13102
// Alias:       Order Delete
// FIX MsgType: MarketDataOrder = "U21"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint64_t RequestTime;
    uint64_t TransactTime;
    int64_t SecurityID;
    OrderDetailsCompT OrderDetails;
} OrderDeleteT;

// Message:	    OrderMassDelete
// TemplateID:  13103
// Alias:       Order Mass Delete
// FIX MsgType: MarketDataOrder = "U21"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t TransactTime;
} OrderMassDeleteT;

// Message:	    OrderModify
// TemplateID:  13101
// Alias:       Order Modify
// FIX MsgType: MarketDataOrder = "U21"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint64_t RequestTime;
    uint64_t TrdRegTSPrevTimePriority;
    int64_t PrevPrice;
    int64_t PrevDisplayQty;
    int64_t SecurityID;
    OrderDetailsCompT OrderDetails;
    int8_t PrevPriceHHIIndicator;
    char Pad7[LEN_PAD7];
} OrderModifyT;

// Message:	    OrderModifySamePrio
// TemplateID:  13106
// Alias:       Order Modify Same Priority
// FIX MsgType: MarketDataOrder = "U21"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint64_t RequestTime;
    uint64_t TransactTime;
    int64_t PrevDisplayQty;
    int64_t SecurityID;
    OrderDetailsCompT OrderDetails;
} OrderModifySamePrioT;

// Message:	    PacketHeader
// TemplateID:  13004
// Alias:       Packet Header
// FIX MsgType: MarketDataReport = "U20"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint32_t ApplSeqNum;
    int32_t MarketSegmentID;
    uint8_t PartitionID;
    uint8_t CompletionIndicator;
    uint8_t ApplSeqResetIndicator;
    uint8_t DSCP;
    char Pad4[LEN_PAD4];
    uint64_t TransactTime;
} PacketHeaderT;

// Message:	    PartialOrderExecution
// TemplateID:  13105
// Alias:       Partial Order Execution
// FIX MsgType: MarketDataOrder = "U21"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint8_t Side;
    uint8_t OrdType;
    uint8_t AlgorithmicTradeIndicator;
    int8_t HHIIndicator;
    uint32_t TrdMatchID;
    int64_t Price;
    uint64_t TrdRegTSTimePriority;
    int64_t SecurityID;
    int64_t LastQty;
    int64_t LastPx;
} PartialOrderExecutionT;

// Message:	    ProductStateChange
// TemplateID:  13300
// Alias:       Product State Change
// FIX MsgType: TradingSessionStatus = "h"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint8_t TradingSessionID;
    uint8_t TradingSessionSubID;
    uint8_t TradSesStatus;
    uint8_t MarketCondition;
    uint8_t FastMarketIndicator;
    uint8_t TESTradSesStatus;
    char Pad2[LEN_PAD2];
    uint64_t TransactTime;
} ProductStateChangeT;

// Message:	    ProductSummary
// TemplateID:  13600
// Alias:       Product Summary
// FIX MsgType: MarketDataInstrument = "U23"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint32_t LastMsgSeqNumProcessed;
    uint8_t TradingSessionID;
    uint8_t TradingSessionSubID;
    uint8_t TradSesStatus;
    uint8_t MarketCondition;
    uint8_t FastMarketIndicator;
    uint8_t TESTradSesStatus;
    char Pad6[LEN_PAD6];
} ProductSummaryT;

// Message:	    QuoteRequest
// TemplateID:  13503
// Alias:       Quote Request
// FIX MsgType: MarketDataInstrument = "U23"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    int64_t LastQty;
    uint8_t Side;
    char Pad7[LEN_PAD7];
    uint64_t TransactTime;
} QuoteRequestT;

// Message:	    SnapshotOrder
// TemplateID:  13602
// Alias:       Snapshot Order
// FIX MsgType: MarketDataOrder = "U21"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    OrderDetailsCompT OrderDetails;
} SnapshotOrderT;

// Message:	    TESTradeReport
// TemplateID:  13203
// Alias:       TES Trade Report
// FIX MsgType: MarketDataTrade = "U22"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t TransactTime;
    int64_t LastQty;
    int64_t LastPx;
    uint32_t TrdMatchID;
    uint16_t TrdType;
    uint16_t TradeCondition;
    uint8_t MultiLegReportingType;
    uint8_t MultiLegPriceModel;
    char Pad6[LEN_PAD6];
    int64_t NonDisclosedTradeVolume;
    uint64_t TransBkdTime;
    uint16_t NumberOfBuySides;
    uint16_t NumberOfSellSides;
    char Pad4[LEN_PAD4];
} TESTradeReportT;

// Message:	    TopOfBook
// TemplateID:  13504
// Alias:       Top of Book
// FIX MsgType: MarketDataInstrument = "U23"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    uint64_t TransactTime;
    int64_t SecurityID;
    int64_t BidPx;
    int64_t OfferPx;
    int64_t BidSize;
    int64_t OfferSize;
    uint16_t NumberOfBuyOrders;
    uint16_t NumberOfSellOrders;
    char Pad4[LEN_PAD4];
} TopOfBookT;

// Message:	    TradeReport
// TemplateID:  13201
// Alias:       Trade Report
// FIX MsgType: MarketDataTrade = "U22"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t TransactTime;
    int64_t LastQty;
    int64_t LastPx;
    uint32_t TrdMatchID;
    uint8_t MatchType;
    uint8_t MatchSubType;
    uint8_t AlgorithmicTradeIndicator;
    char Pad1[LEN_PAD1];
    uint16_t TradeCondition;
    char Pad6[LEN_PAD6];
} TradeReportT;

// Message:	    TradeReversal
// TemplateID:  13200
// Alias:       Trade Reversal
// FIX MsgType: MarketDataTrade = "U22"
typedef struct
{
    MessageHeaderCompT MessageHeader;
    int64_t SecurityID;
    uint64_t TransactTime;
    int64_t LastQty;
    int64_t LastPx;
    uint64_t TrdRegTSExecutionTime;
    uint32_t TrdMatchID;
    uint16_t TradeCondition;
    uint8_t MDOriginType;
    uint8_t NoMDEntries;
    MDTradeEntryGrpSeqT MDTradeEntryGrp[MAX_TRADE_REVERSAL_MD_TRADE_ENTRY_GRP];
} TradeReversalT;

/*
 * Begin of DEPRECATED defines
 */

#define BYTE_ARRAY_OF_0_16 {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}

#define	TID_ADDCOMPLEXINSTRUMENT                         13400		// < AddComplexInstrument (Add Complex Instrument)
#define	TID_ADDFLEXIBLEINSTRUMENT                        13401		// < AddFlexibleInstrument (Add Flexible Instrument)
#define	TID_ADDSCALEDSIMPLEINSTRUMENT                    13402		// < AddScaledSimpleInstrument (Add Scaled Simple Instrument)
#define	TID_AUCTIONBBO                                   13500		// < AuctionBBO (Auction Best Bid/Offer)
#define	TID_AUCTIONCLEARINGPRICE                         13501		// < AuctionClearingPrice (Auction Clearing Price)
#define	TID_CROSSREQUEST                                 13502		// < CrossRequest (Cross Request)
#define	TID_EXECUTIONSUMMARY                             13202		// < ExecutionSummary (Execution Summary)
#define	TID_FULLORDEREXECUTION                           13104		// < FullOrderExecution (Full Order Execution)
#define	TID_HEARTBEAT                                    13001		// < Heartbeat (Heartbeat)
#define	TID_INSTRUMENTSTATECHANGE                        13301		// < InstrumentStateChange (Instrument State Change)
#define	TID_INSTRUMENTSUMMARY                            13601		// < InstrumentSummary (Instrument Summary)
#define	TID_MASSINSTRUMENTSTATECHANGE                    13302		// < MassInstrumentStateChange (Mass Instrument State Change)
#define	TID_ORDERADD                                     13100		// < OrderAdd (Order Add)
#define	TID_ORDERDELETE                                  13102		// < OrderDelete (Order Delete)
#define	TID_ORDERMASSDELETE                              13103		// < OrderMassDelete (Order Mass Delete)
#define	TID_ORDERMODIFY                                  13101		// < OrderModify (Order Modify)
#define	TID_ORDERMODIFYSAMEPRIO                          13106		// < OrderModifySamePrio (Order Modify Same Priority)
#define	TID_PACKETHEADER                                 13004		// < PacketHeader (Packet Header)
#define	TID_PARTIALORDEREXECUTION                        13105		// < PartialOrderExecution (Partial Order Execution)
#define	TID_PRODUCTSTATECHANGE                           13300		// < ProductStateChange (Product State Change)
#define	TID_PRODUCTSUMMARY                               13600		// < ProductSummary (Product Summary)
#define	TID_QUOTEREQUEST                                 13503		// < QuoteRequest (Quote Request)
#define	TID_SNAPSHOTORDER                                13602		// < SnapshotOrder (Snapshot Order)
#define	TID_TESTRADEREPORT                               13203		// < TESTradeReport (TES Trade Report)
#define	TID_TOPOFBOOK                                    13504		// < TopOfBook (Top of Book)
#define	TID_TRADEREPORT                                  13201		// < TradeReport (Trade Report)
#define	TID_TRADEREVERSAL                                13200		// < TradeReversal (Trade Reversal)

#define MAX_ADDCOMPLEXINSTRUMENT_INSTRMTLEGGRP          	20
#define MAX_INSTRUMENTSUMMARY_MDINSTRUMENTENTRYGRP      	15
#define MAX_MASSINSTRUMENTSTATECHANGE_SECMASSSTATGRP    	24
#define MAX_TRADEREVERSAL_MDTRADEENTRYGRP               	15

#define ENUM_AGGRESSORSIDE_BUY                           1
#define ENUM_AGGRESSORSIDE_SELL                          2
#define ENUM_ALGORITHMICTRADEINDICATOR_ALGORITHMICTRADE  1
#define ENUM_APPLSEQRESETINDICATOR_NORESET               0
#define ENUM_APPLSEQRESETINDICATOR_RESET                 1
#define ENUM_BIDORDTYPE_MARKET                           1
#define ENUM_COMPLETIONINDICATOR_INCOMPLETE              0
#define ENUM_COMPLETIONINDICATOR_COMPLETE                1
#define ENUM_CROSSREQUESTTYPE_CROSSANNOUNCEMENT          1
#define ENUM_CROSSREQUESTTYPE_LIQUIDITYIMPROVEMENTCROSS  2
#define ENUM_EXERCISESTYLE_EUROPEAN                      0
#define ENUM_EXERCISESTYLE_AMERICAN                      1
#define ENUM_FASTMARKETINDICATOR_NO                      0
#define ENUM_FASTMARKETINDICATOR_YES                     1
#define LEN_FUNCCATEGORY                                 100
#define ENUM_FUNCCATEGORY_GENERAL                        "General                                                                                             "
#define ENUM_FUNCCATEGORY_ORDERDATA                      "Order Data                                                                                          "
#define ENUM_FUNCCATEGORY_TRADEDATA                      "Trade Data                                                                                          "
#define ENUM_FUNCCATEGORY_STATECHANGE                    "State Change                                                                                        "
#define ENUM_FUNCCATEGORY_REFERENCEDATA                  "Reference Data                                                                                      "
#define ENUM_FUNCCATEGORY_SNAPSHOT                       "Snapshot                                                                                            "
#define ENUM_HHIINDICATOR_NOUPDATE                       -1
#define ENUM_IMPLIEDMARKETINDICATOR_NOTIMPLIED           0
#define ENUM_IMPLIEDMARKETINDICATOR_IMPLIEDINOUT         3
#define ENUM_INPUTSOURCE_CLIPCLIENTBROKER                1
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_SIMPLEINSTRUMENT 1
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_STANDARDOPTIONSTRATEGY 2
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_NONSTANDARDOPTIONSTRATEGY 3
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_VOLATILITYSTRATEGY 4
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_FUTURESSPREAD 5
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_INTERPRODUCTSPREAD 6
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_STANDARDFUTURESSTRATEGY 7
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_PACKANDBUNDLE 8
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_STRIP         9
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_FLEXIBLEINSTRUMENT 10
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_COMMODITYSTRIP 11
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_SCALEDSIMPLEINSTRUMENT 12
#define ENUM_INSTRUMENTSCOPEPRODUCTCOMPLEX_NONSTANDARDVOLATILITYSTRATEGY 13
#define ENUM_LASTFRAGMENT_N                              0
#define ENUM_LASTFRAGMENT_Y                              1
#define LEN_LEGSECURITYIDSOURCE                          1
#define ENUM_LEGSECURITYIDSOURCE_MARKETPLACE             "M"
#define ENUM_LEGSECURITYTYPE_LEGSECURITYMULTILEG         1
#define ENUM_LEGSECURITYTYPE_LEGSECURITYUNDERLYINGLEG    2
#define ENUM_LEGSIDE_BUY                                 1
#define ENUM_LEGSIDE_SELL                                2
#define ENUM_MDENTRYTYPE_TRADE                           2
#define ENUM_MDENTRYTYPE_OPENINGPRICE                    4
#define ENUM_MDENTRYTYPE_CLOSINGPRICE                    5
#define ENUM_MDENTRYTYPE_HIGHPRICE                       7
#define ENUM_MDENTRYTYPE_LOWPRICE                        8
#define ENUM_MDENTRYTYPE_TRADEVOLUME                     66
#define ENUM_MDENTRYTYPE_PREVIOUSCLOSINGPRICE            101
#define ENUM_MDENTRYTYPE_OPENINGAUCTION                  200
#define ENUM_MDENTRYTYPE_INTRADAYAUCTION                 201
#define ENUM_MDENTRYTYPE_CIRCUITBREAKERAUCTION           202
#define ENUM_MDENTRYTYPE_CLOSINGAUCTION                  203
#define ENUM_MDENTRYTYPE_IPOAUCTION                      204
#define ENUM_MDORIGINTYPE_BOOK                           0
#define ENUM_MDORIGINTYPE_OFFBOOK                        1
#define ENUM_MDREPORTEVENT_SCOPEDEFINITION               0
#define ENUM_MDUPDATEACTION_NEW                          0
#define ENUM_MDUPDATEACTION_CHANGE                       1
#define ENUM_MDUPDATEACTION_DELETE                       2
#define ENUM_MDUPDATEACTION_OVERLAY                      5
#define ENUM_MARKETCONDITION_NORMAL                      0
#define ENUM_MARKETCONDITION_STRESSED                    1
#define ENUM_MARKETDATATYPE_ORDERBOOKMAINTENANCE         1
#define ENUM_MARKETDATATYPE_ORDERBOOKEXECUTION           2
#define ENUM_MARKETDATATYPE_TRADEREVERSAL                3
#define ENUM_MARKETDATATYPE_TRADEREPORT                  4
#define ENUM_MARKETDATATYPE_AUCTIONBBO                   5
#define ENUM_MARKETDATATYPE_AUCTIONCLEARINGPRICE         6
#define ENUM_MARKETDATATYPE_CROSSTRADEANNOUNCEMENT       7
#define ENUM_MARKETDATATYPE_QUOTEREQUEST                 8
#define ENUM_MARKETDATATYPE_MARKETSEGMENTSNAPSHOT        9
#define ENUM_MARKETDATATYPE_SINGLEINSTRUMENTSNAPSHOT     10
#define ENUM_MARKETDATATYPE_ORDERBOOKSNAPSHOT            11
#define ENUM_MARKETDATATYPE_MATCHEVENT                   12
#define ENUM_MARKETDATATYPE_TOPOFBOOK                    13
#define ENUM_MARKETDATATYPE_ADDCOMPLEXINSTRUMENT         14
#define ENUM_MARKETDATATYPE_TESTRADEREPORT               15
#define ENUM_MARKETDATATYPE_HHINDEX                      16
#define ENUM_MARKETDATATYPE_ADDFLEXIBLEINSTRUMENT        17
#define ENUM_MARKETDATATYPE_ADDSCALEDSIMPLEINSTRUMENT    18
#define ENUM_MASSMARKETCONDITION_NORMAL                  0
#define ENUM_MASSMARKETCONDITION_STRESSED                1
#define ENUM_MASSSOLDOUTINDICATOR_SOLDOUT                1
#define ENUM_MATCHSUBTYPE_OPENINGAUCTION                 1
#define ENUM_MATCHSUBTYPE_CLOSINGAUCTION                 2
#define ENUM_MATCHSUBTYPE_INTRADAYAUCTION                3
#define ENUM_MATCHSUBTYPE_CIRCUITBREAKERAUCTION          4
#define ENUM_MATCHSUBTYPE_IPOAUCTION                     5
#define ENUM_MATCHTYPE_CONFIRMEDTRADEREPORT              3
#define ENUM_MATCHTYPE_CROSSAUCTION                      5
#define ENUM_MATCHTYPE_CALLAUCTION                       7
#define ENUM_MATCHTYPE_LIQUIDITYIMPROVEMENTCROSS         13
#define ENUM_MATCHTYPE_CONTINUOUSAUCTION                 14
#define LEN_MSGTYPE                                      3
#define ENUM_MSGTYPE_MARKETDATAREPORT                    "U20"
#define ENUM_MSGTYPE_HEARTBEAT                           "0  "
#define ENUM_MSGTYPE_MARKETDATAORDER                     "U21"
#define ENUM_MSGTYPE_MARKETDATATRADE                     "U22"
#define ENUM_MSGTYPE_TRADINGSESSIONSTATUS                "h  "
#define ENUM_MSGTYPE_SECURITYSTATUS                      "f  "
#define ENUM_MSGTYPE_SECURITYMASSSTATUS                  "CO "
#define ENUM_MSGTYPE_SECURITYDEFINITIONUPDATEREPORT      "BP "
#define ENUM_MSGTYPE_MARKETDATAINSTRUMENT                "U23"
#define ENUM_MULTILEGPRICEMODEL_STANDARD                 0
#define ENUM_MULTILEGPRICEMODEL_USERDEFINED              1
#define ENUM_MULTILEGREPORTINGTYPE_SINGLESECURITY        1
#define ENUM_MULTILEGREPORTINGTYPE_INDIVIDUALLEGOFAMULTILEGSECURITY 2
#define ENUM_MULTILEGREPORTINGTYPE_MULTILEGSECURITY      3
#define ENUM_NOMARKETSEGMENTS_ONE                        1
#define ENUM_OFFERORDTYPE_MARKET                         1
#define ENUM_ORDTYPE_MARKET                              1
#define LEN_PAD1                                         1
#define LEN_PAD2                                         2
#define LEN_PAD3                                         3
#define LEN_PAD4                                         4
#define LEN_PAD5                                         5
#define LEN_PAD6                                         6
#define LEN_PAD7                                         7
#define ENUM_POTENTIALSECURITYTRADINGEVENT_NONE          0
#define ENUM_POTENTIALSECURITYTRADINGEVENT_PRICEVOLATILITYAUCTIONISEXTENDED 10
#define ENUM_PREVPRICEHHIINDICATOR_NOUPDATE              -1
#define ENUM_PRODUCTCOMPLEX_SIMPLEINSTRUMENT             1
#define ENUM_PRODUCTCOMPLEX_STANDARDOPTIONSTRATEGY       2
#define ENUM_PRODUCTCOMPLEX_NONSTANDARDOPTIONSTRATEGY    3
#define ENUM_PRODUCTCOMPLEX_VOLATILITYSTRATEGY           4
#define ENUM_PRODUCTCOMPLEX_FUTURESSPREAD                5
#define ENUM_PRODUCTCOMPLEX_INTERPRODUCTSPREAD           6
#define ENUM_PRODUCTCOMPLEX_STANDARDFUTURESSTRATEGY      7
#define ENUM_PRODUCTCOMPLEX_PACKANDBUNDLE                8
#define ENUM_PRODUCTCOMPLEX_STRIP                        9
#define ENUM_PRODUCTCOMPLEX_FLEXIBLEINSTRUMENT           10
#define ENUM_PRODUCTCOMPLEX_COMMODITYSTRIP               11
#define ENUM_PRODUCTCOMPLEX_SCALEDSIMPLEINSTRUMENT       12
#define ENUM_PRODUCTCOMPLEX_NONSTANDARDVOLATILITYSTRATEGY 13
#define ENUM_PUTORCALL_PUT                               0
#define ENUM_PUTORCALL_CALL                              1
#define LEN_RELATEDSECURITYIDSOURCE                      1
#define ENUM_RELATEDSECURITYIDSOURCE_MARKETPLACE         "M"
#define LEN_SECURITYDESC                                 40
#define LEN_SECURITYIDSOURCE                             1
#define ENUM_SECURITYIDSOURCE_MARKETPLACE                "M"
#define ENUM_SECURITYMASSSTATUS_ACTIVE                   1
#define ENUM_SECURITYMASSSTATUS_INACTIVE                 2
#define ENUM_SECURITYMASSSTATUS_EXPIRED                  4
#define ENUM_SECURITYMASSSTATUS_KNOCKEDOUT               6
#define ENUM_SECURITYMASSSTATUS_KNOCKOUTREVOKED          7
#define ENUM_SECURITYMASSSTATUS_SUSPENDED                9
#define ENUM_SECURITYMASSSTATUS_PENDINGDELETION          11
#define ENUM_SECURITYMASSSTATUS_KNOCKEDOUTANDSUSPENDED   12
#define ENUM_SECURITYMASSTRADINGEVENT_PRICEVOLATILITYAUCTIONISEXTENDED 10
#define ENUM_SECURITYMASSTRADINGEVENT_PRICEVOLATILITYAUCTIONISEXTENDEDAGAIN 11
#define ENUM_SECURITYMASSTRADINGSTATUS_TRADINGHALT       2
#define ENUM_SECURITYMASSTRADINGSTATUS_MARKETIMBALANCEBUY 7
#define ENUM_SECURITYMASSTRADINGSTATUS_MARKETIMBALANCESELL 8
#define ENUM_SECURITYMASSTRADINGSTATUS_CLOSED            200
#define ENUM_SECURITYMASSTRADINGSTATUS_RESTRICTED        201
#define ENUM_SECURITYMASSTRADINGSTATUS_BOOK              202
#define ENUM_SECURITYMASSTRADINGSTATUS_CONTINUOUS        203
#define ENUM_SECURITYMASSTRADINGSTATUS_OPENINGAUCTION    204
#define ENUM_SECURITYMASSTRADINGSTATUS_OPENINGAUCTIONFREEZE 205
#define ENUM_SECURITYMASSTRADINGSTATUS_INTRADAYAUCTION   206
#define ENUM_SECURITYMASSTRADINGSTATUS_INTRADAYAUCTIONFREEZE 207
#define ENUM_SECURITYMASSTRADINGSTATUS_CIRCUITBREAKERAUCTION 208
#define ENUM_SECURITYMASSTRADINGSTATUS_CIRCUITBREAKERAUCTIONFREEZE 209
#define ENUM_SECURITYMASSTRADINGSTATUS_CLOSINGAUCTION    210
#define ENUM_SECURITYMASSTRADINGSTATUS_CLOSINGAUCTIONFREEZE 211
#define ENUM_SECURITYMASSTRADINGSTATUS_IPOAUCTION        212
#define ENUM_SECURITYMASSTRADINGSTATUS_IPOAUCTIONFREEZE  213
#define ENUM_SECURITYMASSTRADINGSTATUS_PRECALL           214
#define ENUM_SECURITYMASSTRADINGSTATUS_CALL              215
#define ENUM_SECURITYMASSTRADINGSTATUS_FREEZE            216
#define ENUM_SECURITYMASSTRADINGSTATUS_TRADEATCLOSE      217
#define ENUM_SECURITYMASSTRADINGSTATUS_CIRCUITBREAKERAUCTIONTRIGGEREDBYSTATICLIMITBREACH 220
#define ENUM_SECURITYMASSTRADINGSTATUS_CIRCUITBREAKERAUCTIONTRIGGEREDBYSTATICLIMITBREACHFREEZE 221
#define ENUM_SECURITYSTATUS_ACTIVE                       1
#define ENUM_SECURITYSTATUS_INACTIVE                     2
#define ENUM_SECURITYSTATUS_EXPIRED                      4
#define ENUM_SECURITYSTATUS_KNOCKEDOUT                   6
#define ENUM_SECURITYSTATUS_KNOCKOUTREVOKED              7
#define ENUM_SECURITYSTATUS_SUSPENDED                    9
#define ENUM_SECURITYSTATUS_PENDINGDELETION              11
#define ENUM_SECURITYSTATUS_KNOCKEDOUTANDSUSPENDED       12
#define ENUM_SECURITYTRADINGEVENT_PRICEVOLATILITYAUCTIONISEXTENDED 10
#define ENUM_SECURITYTRADINGEVENT_PRICEVOLATILITYAUCTIONISEXTENDEDAGAIN 11
#define ENUM_SECURITYTRADINGSTATUS_TRADINGHALT           2
#define ENUM_SECURITYTRADINGSTATUS_MARKETIMBALANCEBUY    7
#define ENUM_SECURITYTRADINGSTATUS_MARKETIMBALANCESELL   8
#define ENUM_SECURITYTRADINGSTATUS_CLOSED                200
#define ENUM_SECURITYTRADINGSTATUS_RESTRICTED            201
#define ENUM_SECURITYTRADINGSTATUS_BOOK                  202
#define ENUM_SECURITYTRADINGSTATUS_CONTINUOUS            203
#define ENUM_SECURITYTRADINGSTATUS_OPENINGAUCTION        204
#define ENUM_SECURITYTRADINGSTATUS_OPENINGAUCTIONFREEZE  205
#define ENUM_SECURITYTRADINGSTATUS_INTRADAYAUCTION       206
#define ENUM_SECURITYTRADINGSTATUS_INTRADAYAUCTIONFREEZE 207
#define ENUM_SECURITYTRADINGSTATUS_CIRCUITBREAKERAUCTION 208
#define ENUM_SECURITYTRADINGSTATUS_CIRCUITBREAKERAUCTIONFREEZE 209
#define ENUM_SECURITYTRADINGSTATUS_CLOSINGAUCTION        210
#define ENUM_SECURITYTRADINGSTATUS_CLOSINGAUCTIONFREEZE  211
#define ENUM_SECURITYTRADINGSTATUS_IPOAUCTION            212
#define ENUM_SECURITYTRADINGSTATUS_IPOAUCTIONFREEZE      213
#define ENUM_SECURITYTRADINGSTATUS_PRECALL               214
#define ENUM_SECURITYTRADINGSTATUS_CALL                  215
#define ENUM_SECURITYTRADINGSTATUS_FREEZE                216
#define ENUM_SECURITYTRADINGSTATUS_TRADEATCLOSE          217
#define ENUM_SECURITYTRADINGSTATUS_CIRCUITBREAKERAUCTIONTRIGGEREDBYSTATICLIMITBREACH 220
#define ENUM_SECURITYTRADINGSTATUS_CIRCUITBREAKERAUCTIONTRIGGEREDBYSTATICLIMITBREACHFREEZE 221
#define ENUM_SECURITYTYPE_OPT                            1
#define ENUM_SECURITYTYPE_FUT                            2
#define ENUM_SECURITYTYPE_MLEG                           3
#define LEN_SECURITYUPDATEACTION                         1
#define ENUM_SECURITYUPDATEACTION_ADD                    "A"
#define ENUM_SETTLMETHOD_CASH                            0
#define ENUM_SETTLMETHOD_PHYSICAL                        1
#define ENUM_SIDE_BUY                                    1
#define ENUM_SIDE_SELL                                   2
#define ENUM_SOLDOUTINDICATOR_SOLDOUT                    1
#define ENUM_TESSECURITYMASSSTATUS_ACTIVE                1
#define ENUM_TESSECURITYMASSSTATUS_INACTIVE              2
#define ENUM_TESSECURITYMASSSTATUS_EXPIRED               4
#define ENUM_TESSECURITYMASSSTATUS_SUSPENDED             9
#define ENUM_TESSECURITYSTATUS_ACTIVE                    1
#define ENUM_TESSECURITYSTATUS_INACTIVE                  2
#define ENUM_TESSECURITYSTATUS_EXPIRED                   4
#define ENUM_TESSECURITYSTATUS_SUSPENDED                 9
#define ENUM_TESTRADSESSTATUS_HALTED                     1
#define ENUM_TESTRADSESSTATUS_OPEN                       2
#define ENUM_TESTRADSESSTATUS_CLOSED                     3
#define ENUM_TESTRADSESSTATUS_PRECLOSE                   5
#define ENUM_TRADSESEVENT_TBD                            0
#define ENUM_TRADSESEVENT_STATUSCHANGE                   3
#define ENUM_TRADSESSTATUS_HALTED                        1
#define ENUM_TRADSESSTATUS_OPEN                          2
#define ENUM_TRADSESSTATUS_CLOSED                        3
#define ENUM_TRADECONDITION_IMPLIEDTRADE                 1
#define ENUM_TRADECONDITION_OUTOFSEQUENCE                107
#define ENUM_TRADECONDITION_MIDPOINTPRICE                155
#define ENUM_TRADECONDITION_TRADINGONTERMSOFISSUE        156
#define ENUM_TRADECONDITION_SPECIALAUCTION               596
#define ENUM_TRADECONDITION_TRADEATCLOSE                 624
#define ENUM_TRADECONDITION_RETAIL                       743
#define ENUM_TRADINGSESSIONID_DAY                        1
#define ENUM_TRADINGSESSIONID_MORNING                    3
#define ENUM_TRADINGSESSIONID_EVENING                    5
#define ENUM_TRADINGSESSIONID_AFTERHOURS                 6
#define ENUM_TRADINGSESSIONID_HOLIDAY                    7
#define ENUM_TRADINGSESSIONSUBID_PRETRADING              1
#define ENUM_TRADINGSESSIONSUBID_CONTINUOUS              3
#define ENUM_TRADINGSESSIONSUBID_CLOSING                 4
#define ENUM_TRADINGSESSIONSUBID_POSTTRADING             5
#define ENUM_TRADINGSESSIONSUBID_QUIESCENT               7
#define ENUM_TRDTYPE_BLOCKTRADE                          1
#define ENUM_TRDTYPE_EFP                                 2
#define ENUM_TRDTYPE_EFS                                 12
#define ENUM_TRDTYPE_OTC                                 54
#define ENUM_TRDTYPE_EXCHANGEBASISFACILITY               55
#define ENUM_TRDTYPE_VOLATRADE                           1000
#define ENUM_TRDTYPE_EFPFINTRADE                         1001
#define ENUM_TRDTYPE_EFPINDEXFUTURESTRADE                1002
#define ENUM_TRDTYPE_BLOCKTRADEATMARKET                  1004
#define ENUM_TRDTYPE_XETRAEUREXENLIGHTTRIGGEREDTRADE     1006
#define ENUM_TRDTYPE_BLOCKQTPIPTRADE                     1007
#define ENUM_TRDTYPE_DELTATRADEATMARKET                  1017

/*
 * End of DEPRECATED defines
 */

#if defined(__cplusplus) || defined(c_plusplus)
} /* close scope of 'extern "C"' declaration. */
#endif

#endif