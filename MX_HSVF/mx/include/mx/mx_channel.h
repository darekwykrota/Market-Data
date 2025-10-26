#ifndef _MX_CHANNEL_H_
#define _MX_CHANNEL_H_

#include "mx_common.h"
#include "mx_message_definitions.h"
#include "mx_header.h"
#include "mx_price_indicator_markers.h"
#include "mx_recovery_handler.h"
#include "mx_outright_info.h"
#include "mx_orderbook.h"

#include <algorithm>
#include <ostream>
#include <unordered_set>
#include <unordered_map>


namespace ns {

class MX_Adapter;

class MX_Channel {
public:
    MX_Channel(const ns::ChannelID_t channelId,
                const ChannelTags& tags,
                const std::string& interfaceA,
                const std::string& interfaceB,
                PacketBufferPoolPtr_t packetBufferPool,
                TraceLoggerArray_t loggers);

    ~MX_Channel();
    bool Init(IAdapterSend* sendApi, 
            std::shared_ptr<Config> config, 
            WorkerThreadPtr workerThread, 
            WorkerThreadPtr networkThread,
            const std::string& recoveryUsername,
            const std::string& recoveryPassword,
            const std::string& recoveryLine,
            const int recoveryTimeout,
            const int recoveryPageSize);
    void Start();
    void Stop();
    void Post(std::function<void()> fn);

    void OnRealtimeFeedData(const MessageMeta& mm);
    void OnRetransmissionMsg(char* data);
    void OnRetransmissionComplete();
    void OnRetransmissionFailed();
    MulticastFeedPtrT GetRealTimeFeed() const;
    
protected:
    MulticastFeedPtrT CreateFeed(const std::string& feedName, MulticastReceiver::ProcessMessageFunc_t callback, std::shared_ptr<Config> configg);
    virtual void OnReplayTcpData(const char* buf, size_t len);
    void ProcessReplayData(char* readPtr, size_t len);

private:
    void _OnRealtimePacket(const PacketBufferPtr packetBuffer);
    void _OnRealTimeMsg(char* msg, bool isReplay = false);
    void _ProcessBufferedMsgs(); 

    template<typename MsgT>
    Descriptor_t _GetIndesc(const MsgT* msg);

    template<typename MsgT>
    void _InitMarketEvent(const MsgT* msg, MarketEvent& event);

    template<typename InstrumentKeysMsgT>
    void _CacheGroupInfo(const InstrumentKeysMsgT* msg);

    template<typename MarketDepthMsgT>
    void _ProcessMarketDepthMsg(const MarketDepthMsgT* msg);
    void _Process(const FutureMarketDepth* msg);
    void _Process(const OptionMarketDepth* msg);
    void _Process(const FutureOptionsMarketDepth* msg);
    void _Process(const StrategyMarketDepth* msg);

    void _PopulateMarketEventOnMarketDepth(MarketEvent& event,
                                            const MarketBookSide bookSide,
                                            const MarketUpdateAction updateAction,
                                            const int32_t bidSize, 
                                            const int64_t bidPrice, 
                                            const int32_t bidOrdersNum,
                                            const int depthLevel);
    void _PopulateMarketEventOnMarketDepth(MarketEvent& event,
                                            const MarketBookSide bookSide,
                                            const MarketUpdateAction updateAction,
                                            const int level);
    template<typename MarketDepthMsgT>
    void _HandleStatusUpdate(const MarketDepthMsgT* msg);
    template<typename MarketDepthMsgT>
    void _HandleTheoreticalOpeningUpdate(const MarketDepthMsgT* msg);

    template<typename SummaryMsgT>
    void _ProcessSummaryMsg(const SummaryMsgT* msg);
    template<typename SummaryMsgT>
    void _HandleSettlementUpdate(const SummaryMsgT* msg, MarketEvent& event);
    void _Process(const FuturesSummary* msg);
    void _Process(const OptionSummary* msg);
    void _Process(const FutureOptionsSummary* msg);
    void _Process(const StrategySummary* msg);

    template<typename TradeMsgT>
    void _ProcessTradeMsg(const TradeMsgT* msg, const bool handleIndicativeSettle = false);
    void _Process(const FuturesTrade* msg);
    void _Process(const OptionTrade* msg);
    void _Process(const FutureOptionsTrade* msg);
    void _Process(const StrategyTrade* msg);

    template<typename InstrumentKeysMsgT>
    bool _OnInstrumentKeysMsg(const InstrumentKeysMsgT* msg, InstrumentDefinition& defn, bool& usesTickTable, std::string& tickTableName, const bool isOption = false);
    template<typename InstrumentKeysMsgT>
    void _CompleteInstrumentSetup(const InstrumentKeysMsgT* msg, const InstrumentDefinition& defn);
   
    void _Process(const FuturesInstrumentKeys* msg);
    void _Process(const OptionInstrumentKeys* msg);
    void _Process(const FutureOptionsInstrumentKeys* msg);
    void _Process(const StrategyInstrumentKeys* msg);
    
    void _PostInstrumentDefinition(Descriptor_t indesc,
                                MarketBookType bookType,
                                MarketBookType impliedBookType,
                                MarketUpdateAction action,
                                const InstrumentDefinition& defn);
    template<typename InstrumentKeysMsgT>
    void _LogInstDefn(const InstrumentKeysMsgT* msg, 
                    const InstrumentDefinition& defn, 
                    const std::string& type, 
                    const bool usesTickTable, 
                    const std::string& tickTableName) const;
                    
    template<typename RFQMsgT>
    void _ProcessRFQMsg(const RFQMsgT* msg);
    void _Process(const OptionRequestForQuote* msg);
    void _Process(const FutureOptionsRequestForQuote* msg);
    void _Process(const FuturesRequestForQuote* msg);
    void _Process(const StrategyRequestForQuote* msg);

    void _Process(const GroupStatus* msg);
    void _Process(const GroupStatusStrategies* msg);
    void _UpdateGroupStatus(const std::unordered_map<std::string, std::vector<std::string>>& map, 
                            const std::string& group, 
                            const char status, 
                            const std::string& log);
    void _CacheInstrumentStatus(const std::string& identifier, const char status);
    void _CacheInstrumentDefn(const std::string& identifier, const InstrumentDefinition& defn);

    void _Process(const FutureDeliverables* msg);
    void _Process(const TickTable* msg);
    void _Process(const StartOfDay* msg);
    void _Process(const EndOfTransmission* msg);
    void _Process(const Heartbeat* msg);

    //Empty implementations
    void _Process(const BeginningOfOptionsSummary* msg);
    void _Process(const BeginningOfFutureOptionsSummary* msg);
    void _Process(const BeginningOfFuturesSummary* msg);
    void _Process(const BeginningOfStrategySummary* msg);
    void _Process(const EndOfSales* msg);

   
    
    //Helper methods
    template<typename TradeMsgT>
    long long _GetTradeTime(const TradeMsgT* msg);
    long long _GetNumMilliSecondsSinceMidnightUTC(int hours, int minutes, int seconds, int milliseconds) const;
    std::string _GetMetaDataAsStr(const InstrumentDefinition& defn) const;

    void _GoStable() const;
    MarketBookSide _GetLegSideFromFI(const char fractionIndicator) const;
    bool _LegsAvailable(const std::vector<std::string>& legs) const;
    CurrencyCode::Value _ToCurrencyCode(const std::string& currency) const;
    InstrumentStatus::Value _GetStatus(const char status) const;
    void _CompleteRecovery();
    void _ResetBook(const Descriptor_t indesc) const;
    bool _IsStartupRetransmission() const;

    //CDD processing
    template<typename InstrumentKeysMsgT>
    void _HandleCDDField(const InstrumentKeysMsgT* msg, InstrumentDefinition& defn);
    template<typename InstrumentKeysMsgT>
    void _DecodeExpiry(const InstrumentKeysMsgT* msg, 
                        const std::string& productSymbol, 
                        unsigned short& contractYear,
                        unsigned short& contractMonth, 
                        unsigned short& contractDay, 
                        unsigned short& termMonth, 
                        unsigned short& termYear) const;
    int _GetCurrentYear() const;
    int _DecodeYear(const int year) const;

    void _SendMarketEvent(MarketEvent& event) const;
    void _SendMarketEventEnd(MarketEvent& event) const;
    void _SendEndForChannel();

    void _SanityCheck(const PacketBufferPtr packetBuffer);
    uint16_t _GetMessageCount(const PacketBufferPtr& packetBuffer, std::string& types) const;

  

    //Data members
    IAdapterSend* _sendApi = nullptr;
    MulticastFeedPtrT _realtimeFeed;
    WorkerThreadPtr _workerThread;
    WorkerThreadPtr _networkThread;

    std::deque<MessageMeta> _bufferedRealtimeMsgs;
    std::unordered_map<std::string, int> _decimals;
    std::unordered_map<std::string, char> _cachedStatusMarkerPerInstrument;
    std::unordered_map<std::string, std::vector<std::string>> _outrightGroupToDescs, _strategyGroupToDescs;
    std::unordered_map<std::string, OutrightInfo> _outrights;
    MXOrderbooks _orderbooks;
    //ID to Ticktable
    std::unordered_map<std::string, TickTable_t> _tickTables;
    std::set<Descriptor_t> _securityIds;
    //Identifier to instr defn
    std::unordered_map<std::string, InstrumentDefinition> _instruments;
    std::unordered_set<uint64_t> _recoverySequenceNumbers;

    uint64_t _lastRealtimeSequence = 0; //StartOfDay is always with 1
    bool _inRecovery = false;
    uint32_t _bufferingSkipLogCounter = 0;
   
    MXRecoveryHandler<MX_Channel> _mxRecoveryHandler;
    uint64_t _fromSeq = 0;
    uint64_t _toSeq = 0;
    std::string _recoveryUsername;
    std::string _recoveryPassword;
    std::string _recoveryLine;
    int _recoveryTimeout;
    int _recoveryPageSize;

    const ChannelID_t _channelId;
    ChannelTags _tags;
    const std::string _interfaceA;
    const std::string _interfaceB;
    PacketBufferPoolPtr_t _bufferPool;
    TraceLoggerArray_t _loggers;

    
}; //end class definition

typedef std::shared_ptr<MX_Channel> MX_ChannelPtrT;

} //end namespace

#endif

