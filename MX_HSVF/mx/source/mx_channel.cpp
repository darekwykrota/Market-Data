#include "mx/mx_channel.h"
#include "mx/mx_common.h"
#include <fstream>
#include <time.h>
#include <chrono>

namespace ns {

MX_Channel::MX_Channel(const ns::ChannelID_t channelId,
                const ChannelTags& tags,
                const std::string& interfaceA,
                const std::string& interfaceB,
                PacketBufferPoolPtr_t packetBufferPool,
                TraceLoggerArray_t loggers)
    : _channelId(channelId)
    , _tags(tags)
    , _interfaceA(interfaceA)
    , _interfaceB(interfaceB)
    , _bufferPool(packetBufferPool)
    , _loggers(loggers)
{
}

MX_Channel::~MX_Channel()
{
}

bool MX_Channel::Init(IAdapterSend* sendApi, 
                      std::shared_ptr<Config> config, 
                      WorkerThreadPtr workerThread, 
                      WorkerThreadPtr networkThread, 
                      const std::string& recoveryUsername,
                      const std::string& recoveryPassword,
                      const std::string& recoveryLine,
                      const int recoveryTimeout,
                      const int recoveryPageSize) {
    _sendApi = sendApi;
    _workerThread = workerThread;
    _networkThread = networkThread;
    _recoveryUsername = recoveryUsername;
    _recoveryPassword = recoveryPassword;
    _recoveryLine = recoveryLine;
    _recoveryTimeout = recoveryTimeout;
    _recoveryPageSize = recoveryPageSize;
    assert(_sendApi && _workerThread && _networkThread && _recoveryLine.size() == 2);


    using std::placeholders::_1;
    using std::placeholders::_2;
    _realtimeFeed = CreateFeed("RealtimeFeed", std::bind(&MX_Channel::OnRealtimeFeedData, this, _1), config);
    if(!_realtimeFeed) {
        MX_ERR() << "channelId=" << _channelId << ", realtime feed create failed";
        return false;
    }
    _realtimeFeed->EnableArbitration(MXPacketSequenceGetter, ArbitrationType::Message);
    _realtimeFeed->EnableResetLogic(MXPacketResetGetter);

    if(_recoveryUsername.empty() && _recoveryPassword.empty()) {
        MX_INFO() << "channelId=" << _channelId << ", Both recovery username and password are empty. This is expected";
    } else {
        MX_INFO() << "channelId=" << _channelId << ", Recovery username=" << _recoveryUsername << ", recovery password=" << _recoveryPassword;
    }

    if(_recoveryLine.size() != 2) {
        MX_ERR() << "channelId=" << _channelId << ", Recovery line " << _recoveryLine << " is not of length 2";
        return false;
    }

    const bool ret = _mxRecoveryHandler.Initialize(config, 
                                                    _tags, 
                                                    _bufferPool,
                                                    _networkThread, 
                                                    _workerThread,
                                                    _interfaceA, 
                                                    _interfaceB,
                                                    _recoveryUsername,
                                                    _recoveryPassword,
                                                    _recoveryLine,
                                                    _recoveryTimeout,
                                                    _recoveryPageSize,
                                                    this);

    if(!ret) {
        MX_ERR() << "channelId=" << _channelId << ", MX TCP Recovery handler initialization failed";
        return false;
    }

    return true;
}

MulticastFeedPtrT MX_Channel::CreateFeed(const std::string& feedName, 
                                        MulticastReceiver::ProcessMessageFunc_t callback, 
                                        std::shared_ptr<Config> config) {

    auto feedPtr = std::make_unique<MulticastFeed>(_tags, 
                                                    feedName,
                                                    _bufferPool, 
                                                    _networkThread, 
                                                    _workerThread, 
                                                    callback);

    const auto& confMap = config->GetMapNode<std::string, sMulticastFeedInfo>("Channels." + _tags.channelName + ".Feeds", "Feed", "name");
    FeedInfos_t feedInfos = confMap->PopulateCompositeNodes();

    const bool successA = feedPtr->InitializeFeed(feedInfos, MulticastFeedIdentifier__A, _interfaceA);
    const bool successB = feedPtr->InitializeFeed(feedInfos, MulticastFeedIdentifier__B, _interfaceB);

    if(successA) {
        MX_INFO() << "channelId=" << _channelId << ", successfully initialized " << feedName << "A";
    } else {
        MX_INFO() << "channelId=" << _channelId << ", failed to initialize " << feedName << "A";
    }

    if(successB) {
        MX_INFO() << "channelId=" << _channelId << ", successfully initialized " << feedName << "B";
    } else {
        MX_INFO() << "channelId=" << _channelId << ", failed to initialize " << feedName << "B";
    }

    const bool success = successA | successB;
    return success ? std::move(feedPtr)
                    : MulticastFeedPtrT{};
                    ;
}

void MX_Channel::Start() {
    MX_INFO() << "channelId=" << _channelId << ", starting channel";
    _realtimeFeed->StartFeed();
}

void MX_Channel::Stop() {
    MX_INFO() << "channelId=" << _channelId << ", stopping channel";
    if (_realtimeFeed)
        _realtimeFeed->ShutdownFeed();
}

void MX_Channel::OnRealtimeFeedData(const MessageMeta& mm) {
    const auto& packetBuffer = mm.pb;
    char* readPtr = packetBuffer->m_buffer;
    _SanityCheck(packetBuffer);

    const MsgHeader* header = reinterpret_cast<const MsgHeader*>(readPtr + sizeof(STX));
    const uint64_t seqNum = header->GetSeqNum();
    const int sequenceDiff = seqNum - _lastRealtimeSequence;

    if( (sequenceDiff > 1) || _inRecovery ) {
        if(_bufferingSkipLogCounter % 200 == 0){
            MX_INFO() << "channelId=" << _channelId
            << ", lastSeqNo=" << _lastRealtimeSequence
            << ", currentSeqNo=" << seqNum
            << ", gap=" << sequenceDiff
            << ", inRecovery=" << _inRecovery
            << ". Buffering"
            ;
        }
        ++_bufferingSkipLogCounter;

        _bufferedRealtimeMsgs.push_back(mm);
        
        if(!_inRecovery) {
            _inRecovery = true;
            _recoverySequenceNumbers.clear();
            _bufferingSkipLogCounter = 0;
            _fromSeq = _lastRealtimeSequence + 1;
            _toSeq = seqNum - 1;

            MX_INFO() << "channelId=" << _channelId
            << ", Requesting gap replay from=" << _fromSeq
            << ", to=" << _toSeq
            << ", lastSeqNo=" << _lastRealtimeSequence
            << ", currentSeqNo=" << seqNum
            ;

            _mxRecoveryHandler.RequestGap(_fromSeq, _toSeq);
        }

        return;
    }

    _OnRealtimePacket(packetBuffer);
}

void MX_Channel::_OnRealtimePacket(const PacketBufferPtr packetBuffer) {
    char* readPtr = packetBuffer->m_buffer;
    const uint32_t bytesReceived = packetBuffer->m_bytesReceived;

    while(readPtr - packetBuffer->m_buffer < bytesReceived) {
        assert(*readPtr == STX);

        _OnRealTimeMsg(readPtr + sizeof(STX));

        auto it = std::find(readPtr, packetBuffer->m_buffer + bytesReceived, ETX);
        assert(*it == ETX && (it != packetBuffer->m_buffer + bytesReceived));

        readPtr = ++it;
    }

    MX_VALIDATE_PACKET_READ(readPtr, packetBuffer);
}

void MX_Channel::_SanityCheck(const PacketBufferPtr packetBuffer) {
    //Note: Remove before merging - just for testing
    std::string msgTypes;
    int count = _GetMessageCount(packetBuffer, msgTypes);

    if(count > 1 && msgTypes.find("V") != std::string::npos) {
        assert(!"SanityCheck() - heartbeat in a packet with other msgs");
        MX_WARN() << "channelId=" << _channelId
            << ", Warning - msgs=" << msgTypes 
            << ", count=" << count;
    }
}

void MX_Channel::_OnRealTimeMsg(char* msgPtr, bool inRecovery) {
    const MsgHeader* header = reinterpret_cast<const MsgHeader*>(msgPtr);
    std::string msgType = header->GetMsgType();
    if(!msgType.empty() && msgType.back() == ' ') {
        msgType.pop_back();
    }

    _lastRealtimeSequence = header->GetSeqNum();

    switch(consthash(msgType.c_str())) {
    case consthash("H"): {
        const OptionMarketDepth* msg = reinterpret_cast<const OptionMarketDepth*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("HF"): {
        const FutureMarketDepth* msg = reinterpret_cast<const FutureMarketDepth*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("HB"): {
        const FutureOptionsMarketDepth* msg = reinterpret_cast<const FutureOptionsMarketDepth*>(msgPtr);
        _Process(msg);
    }
    break;
    case consthash("HS"): {
        const StrategyMarketDepth* msg = reinterpret_cast<const StrategyMarketDepth*>(msgPtr);
        _Process(msg);
    }
    break;

    case consthash("N"): {
        const OptionSummary* msg = reinterpret_cast<const OptionSummary*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("NB"): {
        const FutureOptionsSummary* msg = reinterpret_cast<const FutureOptionsSummary*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("NF"): {
        const FuturesSummary* msg = reinterpret_cast<const FuturesSummary*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("NS"): {
        const StrategySummary* msg = reinterpret_cast<const StrategySummary*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("C"): {
        const OptionTrade* msg = reinterpret_cast<const OptionTrade*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("CB"): {
        const FutureOptionsTrade* msg = reinterpret_cast<const FutureOptionsTrade*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("CF"): {
        const FuturesTrade* msg = reinterpret_cast<const FuturesTrade*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("CS"): {
        const StrategyTrade* msg = reinterpret_cast<const StrategyTrade*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("GS"): {
        const GroupStatusStrategies* msg = reinterpret_cast<const GroupStatusStrategies*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("GR"): {
        const GroupStatus* msg = reinterpret_cast<const GroupStatus*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("V"): {
        const Heartbeat* msg = reinterpret_cast<const Heartbeat*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("J"): {
        const OptionInstrumentKeys* msg = reinterpret_cast<const OptionInstrumentKeys*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("JB"): {
        const FutureOptionsInstrumentKeys* msg = reinterpret_cast<const FutureOptionsInstrumentKeys*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("JF"): {
        const FuturesInstrumentKeys* msg = reinterpret_cast<const FuturesInstrumentKeys*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("JS"): {
        const StrategyInstrumentKeys* msg = reinterpret_cast<const StrategyInstrumentKeys*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("Q"): {
        const BeginningOfOptionsSummary* msg = reinterpret_cast<const BeginningOfOptionsSummary*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("QB"): {
        const BeginningOfFutureOptionsSummary* msg = reinterpret_cast<const BeginningOfFutureOptionsSummary*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("QF"): {
        const BeginningOfFuturesSummary* msg = reinterpret_cast<const BeginningOfFuturesSummary*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("QS"): {
        const BeginningOfStrategySummary* msg = reinterpret_cast<const BeginningOfStrategySummary*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("D"): {
        const OptionRequestForQuote* msg = reinterpret_cast<const OptionRequestForQuote*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("DF"): {
        const FuturesRequestForQuote* msg = reinterpret_cast<const FuturesRequestForQuote*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("DB"): {
        const FutureOptionsRequestForQuote* msg = reinterpret_cast<const FutureOptionsRequestForQuote*>(msgPtr);
        _Process(msg);
    }
    break;
    case consthash("DS"): {
        const StrategyRequestForQuote* msg = reinterpret_cast<const StrategyRequestForQuote*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("TT"): {
        const TickTable* msg = reinterpret_cast<const TickTable*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("KF"): {
        const FutureDeliverables* msg = reinterpret_cast<const FutureDeliverables*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("SD"): {
        const StartOfDay* msg = reinterpret_cast<const StartOfDay*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("U"): {
        const EndOfTransmission* msg = reinterpret_cast<const EndOfTransmission*>(msgPtr);
        _Process(msg);
    } 
    break;
    case consthash("S"): {
        const EndOfSales* msg = reinterpret_cast<const EndOfSales*>(msgPtr);
        _Process(msg);
    } 
    break;
   
    default: {
        MX_WARN() << "channelId=" << _channelId << ", Unhandled msg=" << msgType;
    } 
    break;

    }//end switch

} //end _OnRealTimeMsg

void MX_Channel::OnRetransmissionMsg(char* data) {
    const MsgHeader* header = reinterpret_cast<const MsgHeader*>(data);
    const uint64_t seqNum = header->GetSeqNum();
    const std::string msgType = header->GetMsgType();
    MX_DEBUG() << "channelId=" << _channelId << ", Retransmission msg - seq=" << seqNum << ", msg=" << msgType;

    assert(_fromSeq <= seqNum && seqNum <= _toSeq);

    //Sanity check - making sure no duplicates. Delete before going to prod
    const bool newSeqNum = _recoverySequenceNumbers.count(seqNum) == 0;
    assert(newSeqNum);
    _recoverySequenceNumbers.insert(seqNum);

    _OnRealTimeMsg(data, true);
}

void MX_Channel::OnRetransmissionComplete() {
    MX_INFO() << "channelId=" << _channelId << ", Retransmission complete"
    << " - from=" << _fromSeq 
    << ", to=" << _toSeq 
    << ", isStartupRetransmission=" << _IsStartupRetransmission();
    _CompleteRecovery();
}

void MX_Channel::OnRetransmissionFailed() {
    MX_WARN() << "channelId=" << _channelId << ", Retransmission failed!";
    _CompleteRecovery();
}

void MX_Channel::_CompleteRecovery() {
    _GoStable();
    _ProcessBufferedMsgs();
    _SendEndForChannel();
    _inRecovery = false;
}

void MX_Channel::_SendEndForChannel() {
    MX_INFO() << "channelId=" << _channelId << ", Sending snapshot end for " << _securityIds.size() << " instruments, inRecovery=" << _inRecovery;

    MarketEvent event;
    event.channelId = _channelId;
    event.type = MarketEventType::EventEnd;

    for(const auto& entry: _securityIds) {
        event.indesc = entry;
        if(_IsStartupRetransmission())
            _sendApi->OnSnapshot(&event);
        else
            _sendApi->OnIncremental(&event);
    }
}

void MX_Channel::_ProcessBufferedMsgs() {
    MX_INFO() << "channelId=" << _channelId << ", processing buffered msgs - size=" << _bufferedRealtimeMsgs.size() << ", inRecovery=" << _inRecovery;
    
    while(!_bufferedRealtimeMsgs.empty()) {
        const auto& currentPacket = _bufferedRealtimeMsgs.front();
        const auto& packetBuffer = currentPacket.pb;

        _OnRealtimePacket(packetBuffer);
        _bufferedRealtimeMsgs.pop_front();
    }

    MX_INFO() << "channelId=" << _channelId << ", finished processing buffered msgs - size=" << _bufferedRealtimeMsgs.size() << ", inRecovery=" << _inRecovery;
}



template<typename MsgT>
Descriptor_t MX_Channel::_GetIndesc(const MsgT* msg) {
    return consthash(msg->GetIdentifier().c_str());
}

template<typename MsgT>
void MX_Channel::_InitMarketEvent(const MsgT* msg, MarketEvent& event) {
    event.indesc = _GetIndesc(msg);
    event.channelId = _channelId;
    const uint64_t seqNum = msg->msgHeader.GetSeqNum();
    event.messageSequence = event.packetSequence = seqNum;
    event.tsServerRecv = event.tsExchangeSend = ns::GetNowEpoch(std::chrono::nanoseconds());
}

template<typename InstrumentKeysMsgT>
void MX_Channel::_CacheGroupInfo(const InstrumentKeysMsgT* msg) {
    _outrightGroupToDescs[msg->GetRootSymbol()].push_back(msg->GetIdentifier());
}

template<>
void MX_Channel::_CacheGroupInfo(const StrategyInstrumentKeys* msg) {
   _strategyGroupToDescs[msg->GetGroup()].push_back(msg->GetIdentifier());
}


template<typename MarketDepthMsgT>
void MX_Channel::_ProcessMarketDepthMsg(const MarketDepthMsgT* msg) {
    MX_DEBUG() << "channelId=" << _channelId << ", MarketDepthMsg - " << msg->ToString();
    static const int IMPLIED_LEVEL = 0;

    const int levels = msg->GetLevelsNum();
    assert(levels > 0);

    MarketEvent event;
    _InitMarketEvent(msg, event);
    event.type = ns::MarketEventType::LevelBook;

    //Bids
    for(int i = 0; i < levels; ++i) {
        auto& currentLevel = msg->depthLevels[i];
        const int currentDepthLevel = currentLevel.GetLevel();
        /** As per doc:
            "Level of market depth 1 to 5 and A (Implied)"
        */
        if(currentLevel.level == 'A') {
            const int32_t bidSize = currentLevel.GetBidSize();
            if(bidSize != 0) {
                const int32_t bidOrdersNum = currentLevel.GetBidOrdersNum();
                int64_t bidPrice = currentLevel.GetBidPrice();
                AdjustPrice(msg->GetIdentifier(), currentLevel.GetBidPriceFI(), bidPrice, _decimals);
                _PopulateMarketEventOnMarketDepth(event, MarketBookSide::ImpliedBid, MarketUpdateAction::NewOrChange, bidSize, bidPrice, bidOrdersNum, IMPLIED_LEVEL);
                _SendMarketEvent(event);
            } else {
                _PopulateMarketEventOnMarketDepth(event, MarketBookSide::ImpliedBid, MarketUpdateAction::Delete, IMPLIED_LEVEL);
                _SendMarketEvent(event);
            }
        } else {
            const int32_t bidSize = currentLevel.GetBidSize();
            if(bidSize != 0) {    
                const int32_t bidOrdersNum = currentLevel.GetBidOrdersNum();
                int64_t bidPrice = currentLevel.GetBidPrice();
                AdjustPrice(msg->GetIdentifier(), currentLevel.GetBidPriceFI(), bidPrice, _decimals); 
                _PopulateMarketEventOnMarketDepth(event, MarketBookSide::Bid, MarketUpdateAction::NewOrChange, bidSize, bidPrice, bidOrdersNum, currentDepthLevel);
                _SendMarketEvent(event);
                _orderbooks.NewOrChange(msg->GetIdentifier(), MarketBookSide::Bid, currentDepthLevel, bidPrice, bidSize);
            } else if(bidSize == 0) {
                _PopulateMarketEventOnMarketDepth(event, MarketBookSide::Bid, MarketUpdateAction::DeleteFrom, currentDepthLevel);
                _SendMarketEvent(event);
                _orderbooks.DeleteFrom(msg->GetIdentifier(), MarketBookSide::Bid, currentDepthLevel);
            }
        }
    } //end bids

    //Asks
    for(int i = 0; i < levels; ++i) {
        auto& currentLevel = msg->depthLevels[i];
        const int currentDepthLevel = currentLevel.GetLevel();
        if(currentLevel.level == 'A') {
            const int32_t askSize = currentLevel.GetAskSize();
            if(askSize != 0) {
                const int32_t askOrdersNum = currentLevel.GetBidOrdersNum();
                int64_t askPrice = currentLevel.GetAskPrice();
                AdjustPrice(msg->GetIdentifier(), currentLevel.GetAskPriceFI(), askPrice, _decimals);
                _PopulateMarketEventOnMarketDepth(event, MarketBookSide::ImpliedAsk, MarketUpdateAction::NewOrChange, askSize, askPrice, askOrdersNum, IMPLIED_LEVEL);
                _SendMarketEvent(event);
            } else {
                _PopulateMarketEventOnMarketDepth(event, MarketBookSide::ImpliedAsk, MarketUpdateAction::Delete, IMPLIED_LEVEL);
                _SendMarketEvent(event);
            }
        } else {
            const int32_t askSize = currentLevel.GetAskSize();
            if(askSize != 0) {
                const int32_t askOrdersNum = currentLevel.GetAskOrdersNum();
                int64_t askPrice = currentLevel.GetAskPrice();
                AdjustPrice(msg->GetIdentifier(), currentLevel.GetAskPriceFI(), askPrice, _decimals);
                _PopulateMarketEventOnMarketDepth(event, MarketBookSide::Ask, MarketUpdateAction::NewOrChange, askSize, askPrice, askOrdersNum, currentDepthLevel);
                _SendMarketEvent(event);
                _orderbooks.NewOrChange(msg->GetIdentifier(), MarketBookSide::Ask, currentDepthLevel, askPrice, askSize);
            } else if(askSize == 0) {
                _PopulateMarketEventOnMarketDepth(event, MarketBookSide::Ask, MarketUpdateAction::DeleteFrom, currentDepthLevel);
                _SendMarketEvent(event);
                _orderbooks.DeleteFrom(msg->GetIdentifier(), MarketBookSide::Ask, currentDepthLevel);
            }
        }
    } //end asks


    _HandleStatusUpdate(msg);

    const char status = msg->GetStatusMarker();
    if(status != StatusMarker::ContinuousTrading) {
        //Why are we doing this?? Talk to team
        _HandleTheoreticalOpeningUpdate(msg);
    }

    _SendMarketEventEnd(event);
}

void MX_Channel::_PopulateMarketEventOnMarketDepth(MarketEvent& event,
                                                    const MarketBookSide bookSide,
                                                    const MarketUpdateAction updateAction,
                                                    const int32_t size, 
                                                    const int64_t price, 
                                                    const int32_t ordersNum,
                                                    const int level) {           
    event.entry.level_book.side = bookSide;
    event.entry.level_book.action = updateAction;
    event.entry.level_book.level = level;
    event.entry.level_book.quantity = size;
    event.entry.level_book.price = price;
    event.entry.level_book.numOrders = ordersNum;
}

void MX_Channel::_PopulateMarketEventOnMarketDepth(MarketEvent& event,
                                                    const MarketBookSide bookSide,
                                                    const MarketUpdateAction updateAction,
                                                    const int level) {
    event.entry.level_book.side = bookSide;
    event.entry.level_book.action = updateAction;
    event.entry.level_book.level = level;
}

template<typename MarketDepthMsgT>
void MX_Channel::_HandleStatusUpdate(const MarketDepthMsgT* msg) {
    const char statusMarker = msg->GetStatusMarker();
    const std::string identifier = msg->GetIdentifier();

    auto it = _cachedStatusMarkerPerInstrument.find(identifier);
    if(std::end(_cachedStatusMarkerPerInstrument) == it || statusMarker != it->second) {
        _CacheInstrumentStatus(identifier, statusMarker);

        MarketEvent event;
        _InitMarketEvent(msg, event);
        const auto ttStatus = _GetStatus(statusMarker);
        event.type = MarketEventType::Status;
        event.entry.status.val = ttStatus;
        _SendMarketEvent(event);

        MX_INFO() << "channelId=" << _channelId << ", Status update for identifier=" << identifier << ", mxStatus=" << +statusMarker << ", ttStatus=" << ttStatus;
    }
}

template<typename MarketDepthMsgT>
void MX_Channel::_HandleTheoreticalOpeningUpdate(const MarketDepthMsgT* msg) {
    auto info = _orderbooks.TopBidEqualsTopAsk(msg->GetIdentifier());
    const bool isNewUpdate = std::get<0>(info); 
    const int64_t price = std::get<1>(info);
    const int32_t qty = std::get<2>(info);  

    MX_DEBUG() << "channelId=" << _channelId << ", TheoreticalOpening - securityId=" << msg->GetIdentifier() 
    << ", newUpdate=" << isNewUpdate
    << ", price=" << price
    << ", qty=" << qty
    ;

    MarketEvent event;
    _InitMarketEvent(msg, event);
    event.type = MarketEventType::StatPrice;
    event.entry.stat_price.action = MarketUpdateAction::New;
    event.entry.stat_price.id = StatPriceID::IndOpenPrc;
    event.entry.stat_price.val = price;

    if(!isNewUpdate) {
        event.entry.stat_price.action = MarketUpdateAction::Delete;
    }
    _SendMarketEvent(event);

    event.type = MarketEventType::StatQty;
    event.entry.stat_qty.action = MarketUpdateAction::New;
    event.entry.stat_qty.id = StatQtyID::IndicativeOpenQty;
    event.entry.stat_qty.val = qty;

    if(!isNewUpdate) {
        event.entry.stat_qty.action = MarketUpdateAction::Delete;
    }
    _SendMarketEvent(event);
}

void MX_Channel::_Process(const FutureMarketDepth* msg) {
    _ProcessMarketDepthMsg(msg);
}
void MX_Channel::_Process(const OptionMarketDepth* msg) {
    _ProcessMarketDepthMsg(msg);
}
void MX_Channel::_Process(const FutureOptionsMarketDepth* msg) {
    _ProcessMarketDepthMsg(msg);
}
void MX_Channel::_Process(const StrategyMarketDepth* msg) {
    _ProcessMarketDepthMsg(msg);
}

template<typename SummaryMsgT>
void MX_Channel::_ProcessSummaryMsg(const SummaryMsgT* msg) {
    MX_DEBUG() << "channelId=" << _channelId << ", SummaryMsg - " << msg->ToString();

    const std::string identifier = msg->GetIdentifier();
    int64_t highPrice = msg->GetHighPrice();
    int64_t lowPrice = msg->GetLowPrice();
    int64_t openPrice = msg->GetOpenPrice();

    AdjustPrice(identifier, msg->GetHighPriceFractionIndicator(), highPrice, _decimals);
    AdjustPrice(identifier, msg->GetLowPriceFractionIndicator(), lowPrice, _decimals);
    AdjustPrice(identifier, msg->GetOpenPriceFractionIndicator(), openPrice, _decimals);

    const int64_t volume = msg->GetVolume();
    const char reason = msg->GetReason();

    MX_DEBUG() << "channelId=" << _channelId << ", Summary msg - details:"
    << " identifier=" << identifier
    << ", high=" << highPrice
    << ", low=" << lowPrice
    << ", open=" << openPrice
    << ", volume=" << volume
    << ", reason=" << reason
    ;
 

    MarketEvent event;
    _InitMarketEvent(msg, event);
    event.type = MarketEventType::StatPrice;
    event.entry.stat_price.action = MarketUpdateAction::New;

    //Handle High, Low, Open, Settle, Total Volume
    if(highPrice != 0) {
        event.entry.stat_price.id = StatPriceID::High;
        event.entry.stat_price.val = highPrice;
        _SendMarketEvent(event);
    }

    if(lowPrice != 0) {
        event.entry.stat_price.id = StatPriceID::Low;
        event.entry.stat_price.val = lowPrice;
        _SendMarketEvent(event);
    }

    if(openPrice != 0) {
        event.entry.stat_price.id = StatPriceID::Open;
        event.entry.stat_price.val = openPrice;
        _SendMarketEvent(event);
    }
    
    if(volume != 0) {
        event.type = MarketEventType::StatQty;
        event.entry.stat_qty.id = StatQtyID::Volume;
        event.entry.stat_qty.action = MarketUpdateAction::New;
        event.entry.stat_qty.val = volume;
        _SendMarketEvent(event);
    }

    _HandleSettlementUpdate(msg, event);
    _SendMarketEventEnd(event);
}

template<typename SummaryMsgT>
void MX_Channel::_HandleSettlementUpdate(const SummaryMsgT* msg, MarketEvent& event) {
    const std::string identifier = msg->GetIdentifier();
    int64_t settlementPrice = msg->GetSettlementPrice();
    int64_t previousSettlementPrice = msg->GetPreviousSettlementPrice();

    AdjustPrice(identifier, msg->GetSettlemenetPriceFractionIndicator(), settlementPrice, _decimals);
    AdjustPrice(identifier, msg->GetPreviousSettlementPriceFI(), previousSettlementPrice, _decimals);

    const char reason = msg->GetReason();

    MX_DEBUG() << "channelId=" << _channelId << ", Summary msg - details:"
    << " identifier=" << identifier
    << ", settle=" << settlementPrice
    << ", previousSettle=" << previousSettlementPrice
    << ", reason=" << reason
    ;

    switch(reason) {
    case ReasonMarker::EndOfDay: {
        if(settlementPrice != 0) {
            event.type = MarketEventType::StatPrice;
            event.entry.stat_price.id = StatPriceID::Settle;
            event.entry.stat_price.val = settlementPrice;
            _SendMarketEvent(event);

            event.type = MarketEventType::StatTime;
            event.entry.stat_time.id = StatTimeID::SettleTime;
            event.entry.stat_time.action = MarketUpdateAction::New;
            event.entry.stat_time.val = GenerateNowSettleTime();
            _SendMarketEvent(event);
        }
    }
    break;
    default: {
        if(previousSettlementPrice != 0) {
            event.type = MarketEventType::StatPrice;
            event.entry.stat_price.id = StatPriceID::Settle;
            event.entry.stat_price.action = MarketUpdateAction::New;
            event.entry.stat_price.val = previousSettlementPrice;
            _SendMarketEvent(event);

            event.type = MarketEventType::StatTime;
            event.entry.stat_time.id = StatTimeID::SettleTime;
            event.entry.stat_time.action = MarketUpdateAction::New;
            event.entry.stat_time.val = GenerateNowSettleTime();
            _SendMarketEvent(event);
        }
    }
    break;
    }
}

template<>
void MX_Channel::_HandleSettlementUpdate(const StrategySummary* msg, MarketEvent& event) {
    //noop
}

void MX_Channel::_Process(const FuturesSummary* msg) {
    _ProcessSummaryMsg(msg);
}
void MX_Channel::_Process(const StrategySummary* msg) {
    _ProcessSummaryMsg(msg);
}
void MX_Channel::_Process(const OptionSummary* msg) {
    _ProcessSummaryMsg(msg);
}
void MX_Channel::_Process(const FutureOptionsSummary* msg) {
    _ProcessSummaryMsg(msg);
}

template<typename TradeMsgT>
void MX_Channel::_ProcessTradeMsg(const TradeMsgT* msg, const bool handleIndicativeSettle) {
    MX_DEBUG() << "channelId=" << _channelId << ", TradeMsg - " << msg->ToString();

    const std::string identifier = msg->GetIdentifier();
    int64_t tradePrice = msg->GetTradePrice();
    AdjustPrice(identifier, msg->GetTradePriceFractionIndicator(), tradePrice, _decimals);
    
    const int64_t tradeQty = msg->GetVolume();
    const char priceIndicatorMarker = msg->GetPriceIndicatorMarker();
    
    MX_DEBUG() << "channelId=" << _channelId << ", Trade msg - details:"
    << ", identifier=" << identifier
    << ", price=" << tradePrice
    << ", qty=" << tradeQty
    << ", marker=" << priceIndicatorMarker
    << ", handleIndSettle=" << std::boolalpha << handleIndicativeSettle
    ;

    if(tradeQty <= 0) {
        if(priceIndicatorMarker != PriceIndicatorMarker::ReferencePrice) {
            MX_ERR() << "channelId=" << _channelId << ", Invalid trade qty received, identifier=" << identifier << ", qty=" << tradeQty;
        } else {
            if(handleIndicativeSettle) {
                MarketEvent event;
                _InitMarketEvent(msg, event);
                event.type = MarketEventType::StatPrice;
                event.entry.stat_price.id = StatPriceID::IndSettle;
                event.entry.stat_price.action = MarketUpdateAction::New;
                event.entry.stat_price.val = tradePrice;
                _SendMarketEvent(event);

                event.type = MarketEventType::StatTime;
                event.entry.stat_time.id = StatTimeID::SettleTime;
                event.entry.stat_time.action = MarketUpdateAction::New;
                event.entry.stat_time.val = GenerateNowSettleTime();
                _SendMarketEvent(event);
                _SendMarketEventEnd(event);
            }
        }
        
        //As per old design - since this was not a real trade and just a settlement price, not processing further
        return;
    }

    MarketEvent event;
    _InitMarketEvent(msg, event);
    event.type = MarketEventType::Trade;
    event.entry.trade.orderId = 0;
    event.entry.trade.status = TradeStatus::Regular;
    event.entry.trade.qualifier = TradeQualifier::Value::Regular;
    event.entry.trade.side = HitOrTake::Unknown;
    event.entry.trade.price = tradePrice;
    event.entry.trade.quantity = tradeQty;
    event.entry.trade.tsTrade = _GetTradeTime(msg);
    event.entry.trade.tsExchangeTransact = 0;
    event.entry.trade.bidCounterPartyId = 0;
    event.entry.trade.askCounterPartyId = 0;

    switch(priceIndicatorMarker) {
    case PriceIndicatorMarker::Crossed:
    case PriceIndicatorMarker::Committed: 
        event.entry.trade.type = TradeType::Value::OTC_Guaranteed_Cross;
        break;
    case PriceIndicatorMarker::BlockTrade:
        event.entry.trade.type = TradeType::Value::OTC_Block_Trade;
        break;
    case PriceIndicatorMarker::EFRReporting: 
        event.entry.trade.type = TradeType::Value::OTC_Exchange_For_Swap;
        break;
    case PriceIndicatorMarker::EFPReporting:
        event.entry.trade.type = TradeType::Value::OTC_Exchange_for_Physical___Equity_Index;
        break;
    case PriceIndicatorMarker::StrategyReporting: {
        //Just LOG
    }
    break;
    default:
        event.entry.trade.type = TradeType::Value::Normal;
        break;
    }

    _SendMarketEvent(event);
    _SendMarketEventEnd(event);
}

void MX_Channel::_Process(const FuturesTrade* msg) {
    _ProcessTradeMsg(msg, true);
}
void MX_Channel::_Process(const FutureOptionsTrade* msg) {
    _ProcessTradeMsg(msg, true);
}
void MX_Channel::_Process(const OptionTrade* msg) {
    _ProcessTradeMsg(msg);
}
void MX_Channel::_Process(const StrategyTrade* msg) {
    _ProcessTradeMsg(msg);
}

template<typename InstrumentKeysMsgT>
bool MX_Channel::_OnInstrumentKeysMsg(const InstrumentKeysMsgT* msg, InstrumentDefinition& defn, bool& usesTickTable, std::string& tickTableName, const bool isOption) {
    defn.seriesKey = defn.instrumentName = msg->GetInstrumentName();
    defn.productSymbol = GetString(msg->GetRootSymbol());
    defn.exchangeTicker = TrimRight(msg->GetInstrumentExternalCode());
    defn.securityId = msg->GetGroup() + msg->GetInstrument();
    defn.securityExchange = tt::messaging::order::enums::TT_SECURITY_EXCHANGE_ID_MX;

    const std::string tickIncrementAsStr = GetString(msg->GetTickIncrement());
    double tickSize = 0.0;
    int64_t tickIncrement = 0;
    if(tickIncrementAsStr.size() > 2 && tickIncrementAsStr.substr(0, 2) != "TT") {
        tickIncrement = std::stoll(tickIncrementAsStr);
        const char tickIncrementFI = msg->GetTickIncrementFI();
        tickSize = GetPrice(tickIncrement, tickIncrementFI);
    } else {
        //use tick table
        usesTickTable = true;
        tickTableName = tickIncrementAsStr;
        auto it = _tickTables.find(tickTableName);

        if(it == std::end(_tickTables)) {
            assert(!"OnInstrumentKeysMsg() - tick table not found");
            MX_WARN() << "Tick table " << tickTableName << " was not found";
            return false;
        }

        defn.tickTable = it->second;
        const int64_t tickSizeNumerator = it->second[0].tickSizeNumerator;
        const int64_t tickSizeDenominator = it->second[0].tickSizeDenominator;
        tickIncrement = tickSizeNumerator;
        tickSize = (double)tickSizeNumerator / tickSizeDenominator;
    }

    assert(tickSize != 0.0 && tickIncrement != 0);

    defn.tickSizeNumerator = tickIncrement;
    const int decimals = ns::GetDecimals(tickSize / defn.tickSizeNumerator, 14, std::numeric_limits<double>::epsilon());
    defn.wireFormat.priceFactor = decimals;
    _decimals.insert({msg->GetIdentifier(), decimals});

    const double tickValue = GetPrice(msg->GetTickValue(), msg->GetTickValueFI());
    if(tickValue > 1.0) {
        defn.tickValueNumerator = tickSize * tickValue * ExpandWireFactor(decimals);
    } else {
        const int64_t contractSize = msg->GetContractSize();
        defn.tickValueNumerator = tickSize * contractSize * ExpandWireFactor(decimals);
    }

    const int priceDisplayDecimals = ns::GetDecimals(tickSize, 14, std::numeric_limits<double>::epsilon()) + 1;
    defn.priceDisplayDecimals = priceDisplayDecimals;
    defn.priceDisplayType = PriceDisplayTypes::DECIMAL;

    const std::string currency = msg->GetCurrency();
    defn.currencyCode = _ToCurrencyCode(currency);
    //From the spec
    defn.marketDepth = 5;
    defn.impliedDepth = 1;
    defn.isImplieds = true;

    OutrightInfo outrightInfo(defn.productSymbol, defn.tickValueNumerator, defn.wireFormat.priceFactor, currency, defn.securityId, isOption);
    _outrights.insert({defn.securityId, outrightInfo});
    _CacheGroupInfo(msg);

    _orderbooks.CreateOrGetOrderbook(msg->GetIdentifier());

    defn.syntheticFlags = ((SyntheticFlags_t)SyntheticFlag::GenerateHigh |
                           (SyntheticFlags_t)SyntheticFlag::GenerateLow |
                           (SyntheticFlags_t)SyntheticFlag::GenerateOpen |
                           (SyntheticFlags_t)SyntheticFlag::GenerateTotalTradedQuantity
                          );
    defn.persistenceFlags = ((PersistenceFlags_t)PersistenceFlag::PersistHigh |
                             (PersistenceFlags_t)PersistenceFlag::PersistLow |
                             (PersistenceFlags_t)PersistenceFlag::PersistOpen |
                             (PersistenceFlags_t)PersistenceFlag::PersistTotalTradedQuantity |
                             (PersistenceFlags_t)PersistenceFlag::PersistSettlement
                            );
    
    return true;
}

template<typename InstrumentKeysMsgT>
void MX_Channel::_CompleteInstrumentSetup(const InstrumentKeysMsgT* msg, const InstrumentDefinition& defn) {
    Descriptor_t indesc = _GetIndesc(msg);
    const bool newInstrument = _securityIds.count(indesc) == 0;
    _securityIds.insert(indesc);
   
    _PostInstrumentDefinition(indesc, MarketBookType::Level, MarketBookType::Level, MarketUpdateAction::New, defn);
    
    if(_inRecovery && _IsStartupRetransmission()) {
        _ResetBook(indesc);
        return;
    }

    if(newInstrument) {
        MarketEvent event;
        event.type = MarketEventType::EventEnd;
        event.indesc = indesc;
        event.channelId = _channelId;
        _sendApi->OnSnapshot(&event);
    } else {
        MX_INFO() << "channelId=" << _channelId << ", Existing instrument - name=" << defn.instrumentName;
    }
}

void MX_Channel::_Process(const FuturesInstrumentKeys* msg) {
    MX_INFO() << msg->ToString();

    InstrumentDefinition defn;
    defn.productType = ProductType::FUTURE;

    bool usesTickTable = false;
    std::string tickTableName = "";
    const bool ret = _OnInstrumentKeysMsg(msg, defn, usesTickTable, tickTableName);
    if(!ret) {
        assert(!"FuturesInstrumentKeys - failed to process defn");
        MX_WARN() << "Failed to process instrument defn - securityId=" << defn.securityId << ", instrName=" << defn.instrumentName;
        return;
    }
    _HandleCDDField(msg, defn);
    
    const std::string maturityDateAsString = msg->GetExpiryDate();
    const std::string ltd = msg->GetLastTradingDate();
    DateTime maturityDate = GetExpiryDate(maturityDateAsString);
    defn.expiration = maturityDate;
    DateTime ltdDate = GetLTD(ltd);
    defn.lastTradeDt = defn.expiryDate = ltdDate;

    _CacheInstrumentDefn(msg->GetIdentifier(), defn);
    _CompleteInstrumentSetup(msg, defn);
    _LogInstDefn(msg, defn, "FuturesInstrumentKeys", usesTickTable, tickTableName);
}

void MX_Channel::_Process(const OptionInstrumentKeys* msg) {
    MX_INFO() << msg->ToString();

    InstrumentDefinition defn;
    defn.productType = ProductType::OPTION;

    bool usesTickTable = false;
    std::string tickTableName = "";
    const bool ret = _OnInstrumentKeysMsg(msg, defn, usesTickTable, tickTableName, true);
    if(!ret) {
        assert(!"OptionInstrumentKeys - failed to process defn");
        MX_WARN() << "Failed to process instrument defn - securityId=" << defn.securityId << ", instrName=" << defn.instrumentName;
        return;
    }
    _HandleCDDField(msg, defn);

    const std::string lastTradingDate = msg->GetLastTradingDate();
    DateTime ltd = GetLTD(lastTradingDate);
    defn.expiration = defn.lastTradeDt = defn.expiryDate = ltd;

    _CacheInstrumentDefn(msg->GetIdentifier(), defn);
    _CompleteInstrumentSetup(msg, defn);
    _LogInstDefn(msg, defn, "OptionInstrumentKeys", usesTickTable, tickTableName);
}

void MX_Channel::_Process(const FutureOptionsInstrumentKeys* msg) {
    MX_INFO() << msg->ToString();
   
    InstrumentDefinition defn;
    defn.productType = ProductType::OPTION;

    bool usesTickTable = false;
    std::string tickTableName = "";
    const bool ret = _OnInstrumentKeysMsg(msg, defn, usesTickTable, tickTableName, true);
    if(!ret) {
        assert(!"FutureOptionsInstrumentKeys - failed to process defn");
        MX_WARN() << "Failed to process instrument defn - securityId=" << defn.securityId << ", instrName=" << defn.instrumentName;
        return;
    }
    _HandleCDDField(msg, defn);
  
    const std::string expiryDate = msg->GetExpiryDate();
    const std::string maturityDateStr = msg->GetSymbolYear() + GetMonthNumber(msg->GetSymbolMonth()) + msg->GetExpiryDay();
    DateTime ltdDate = GetExpiryDate(expiryDate);
    defn.lastTradeDt = defn.expiryDate = ltdDate;
    DateTime maturityDate = GetExpiryDate(maturityDateStr);
    defn.expiration = maturityDate;

    _CacheInstrumentDefn(msg->GetIdentifier(), defn);
    _CompleteInstrumentSetup(msg, defn);
    _LogInstDefn(msg, defn, "FutureOptionsInstrumentKeys", usesTickTable, tickTableName);
}

void MX_Channel::_Process(const StrategyInstrumentKeys* msg) {
    MX_INFO() << msg->ToString();

    const std::string instName = msg->GetInstrumentName();
    std::vector<std::string> legIdentifiers = msg->GetLegIdentifiers();
    if(!_LegsAvailable(legIdentifiers)) {
        assert(!"Processing strategy - not all legs available");
        MX_WARN() << "channelId=" << _channelId << ", Not all legs available for strategy symbol=" << instName;
        return;
    }

    InstrumentDefinition defn;
    defn.seriesKey = defn.instrumentName = defn.exchangeTicker = instName;

    bool usesTickTable = false;
    std::string tickTableName = "";
    const std::string tickIncrementAsStr = GetString(msg->GetTickIncrement());
    double tickSize = 0.0;
    int64_t tickIncrement = 0;
    if(tickIncrementAsStr.size() > 2 && tickIncrementAsStr.substr(0, 2) != "TT") {
        tickIncrement = std::stoll(tickIncrementAsStr);
        const char tickIncrementFI = msg->GetTickIncrementFI();
        tickSize = GetPrice(tickIncrement, tickIncrementFI);
    } else {
        //use tick table
        usesTickTable = true;
        tickTableName = tickIncrementAsStr;
        auto it = _tickTables.find(tickTableName);

        if(it == std::end(_tickTables)) {
            assert(!"Processing StrategyInstrumentKeys - tick table not found");
            MX_WARN() << "Processing StrategyInstrumentKeys - Tick table " << tickTableName << " was not found, strategy symbol=" << instName;
            return;
        }

        defn.tickTable = it->second;
        const int64_t tickSizeNumerator = it->second[0].tickSizeNumerator;
        const int64_t tickSizeDenominator = it->second[0].tickSizeDenominator;
        tickIncrement = tickSizeNumerator;
        tickSize = (double)tickSizeNumerator / tickSizeDenominator;
    }

    assert(tickSize != 0.0 && tickIncrement != 0);

    defn.tickSizeNumerator = tickIncrement;
    const int decimals = ns::GetDecimals(tickSize / defn.tickSizeNumerator, 14, std::numeric_limits<double>::epsilon());
    defn.wireFormat.priceFactor = decimals;
    _decimals.insert({msg->GetIdentifier(), decimals});


    //Handle Legs
    std::set<int64_t> tickValues;
    bool isOptionLeg = false;
    const int legsCount = msg->GetLegsNum();
    for(int i = 0; i < legsCount; ++i) {
        auto& currentLeg = msg->legs[i];
        const double legRatio = GetPrice(currentLeg.GetLegRatio(), currentLeg.GetLegRatioFI());

        //Sanity check. To be fixed later. Not enough hours in a day
        //Just bomb out for now if it happens
        if (std::floor(legRatio) != legRatio) {
            assert(!"Processing StrategyInstrumentKeys - leg ration contains fractional part");
            MX_WARN() << "Processing StrategyInstrumentKeys - leg ration contains fractional part, strategy symbol=" << instName;
            return;
        }

        LegInfo legInfo;
        legInfo.securityId = currentLeg.GetIdentifier();
        legInfo.side = _GetLegSideFromFI(currentLeg.GetLegRatioFI());
        legInfo.ratioQtyNumerator = legRatio;
        legInfo.ratioQtyDenominator = 1;
        defn.legList.push_back(legInfo);

        auto it = _outrights.find(currentLeg.GetIdentifier());
        if(std::end(_outrights) != it) {
            defn.productSymbol = it->second.productSymbol;
            defn.currencyCode = _ToCurrencyCode(it->second.currency);
            tickValues.insert(it->second.tickValueNumerator);
            isOptionLeg = isOptionLeg || it->second.isOption;
        }
    }

    defn.productType = isOptionLeg ? ProductType::OPTION_STRATEGIES : ProductType::SPREAD;
    if(tickValues.empty()) {
        assert(!"Processing StrategyInstrumentKeys - tick values set empty");
        MX_WARN() << "Processing StrategyInstrumentKeys - tick values set empty, strategy symbol=" << instName;
        return;
    }

    defn.tickValueNumerator = *tickValues.begin();
    const int priceDisplayDecimals = ns::GetDecimals(tickSize, 14, std::numeric_limits<double>::epsilon()) + 1;
    defn.priceDisplayDecimals = priceDisplayDecimals;
    defn.priceDisplayType = PriceDisplayTypes::DECIMAL;

    //From the spec
    defn.marketDepth = 5;
    defn.impliedDepth = 1;
    defn.isImplieds = true;

    const std::string lastTradingDate = msg->GetLastTradingDate();
    DateTime ltd = GetLTD(lastTradingDate);
    defn.expiration = defn.lastTradeDt = defn.expiryDate = ltd;

    _CacheGroupInfo(msg);

    _CacheInstrumentDefn(msg->GetIdentifier(), defn);
    _CompleteInstrumentSetup(msg, defn);
    _LogInstDefn(msg, defn, "StrategyInstrumentKeys", usesTickTable, tickTableName);
}

void MX_Channel::_PostInstrumentDefinition(Descriptor_t indesc,
                                            MarketBookType bookType,
                                            MarketBookType impliedBookType,
                                            MarketUpdateAction action,
                                            const InstrumentDefinition& defn) {
       
    InstrumentDefinition instrDefn(defn);
    _sendApi->OnInstrumentDefinition(indesc, 
                                    _channelId, 
                                    bookType,
                                    impliedBookType, 
                                    action, 
                                    &instrDefn);    
}

template<typename InstrumentKeysMsgT>
void MX_Channel::_LogInstDefn(const InstrumentKeysMsgT* msg, 
                            const InstrumentDefinition& defn, 
                            const std::string& type, 
                            const bool usesTickTable, 
                            const std::string& tickTableName) const {
    MX_INFO() << "channelId=" << _channelId << ", " << type << " details - identifier=" << msg->GetIdentifier()
    << delimiter << "symbol=" << defn.productSymbol
    << delimiter << "securityId=" << defn.securityId
    << delimiter << "inst_name=" << defn.instrumentName
    << delimiter << "seriesKey=" << defn.seriesKey
    << delimiter << "exchangeTicker=" << defn.exchangeTicker
    << delimiter << "currency=" << defn.currencyCode
    << delimiter << "productType=" << defn.productType
    << delimiter << "marketDepth=" << defn.marketDepth
    << delimiter << "ts=" << defn.TickSize()
    << delimiter << "ts_numerator=" << defn.tickSizeNumerator
    << delimiter << "tv=" << defn.TickValue()
    << delimiter << "pv=" << defn.PointValue()
    << delimiter << "maturityDate=" << defn.expiration
    << delimiter << "ltd=" << defn.lastTradeDt 
    << delimiter << "expiryDate=" << defn.expiryDate
    << delimiter << "pdd=" << defn.priceDisplayDecimals
    << delimiter << "dt=" << defn.priceDisplayType
    << delimiter << "priceFactor=" << defn.wireFormat.priceFactor
    << delimiter << "usesTickTable=" << std::boolalpha << usesTickTable
    << delimiter << "tickTableName=" << tickTableName
    << delimiter << "metaData=" << _GetMetaDataAsStr(defn)
    ;
}

template<typename RFQMsgT>
void MX_Channel::_ProcessRFQMsg(const RFQMsgT* msg) {
    MX_DEBUG() << "channelId=" << _channelId << ", RFQMsg - " << msg->ToString();

    RFQ_Side::Value side;
    switch(msg->GetRequestedMarketSide()) {
    case 'B': side = RFQ_Side::Buy; break;
    case 'S': side = RFQ_Side::Sell; break;
    case '2': side = RFQ_Side::BuySell; break;
    default:
        assert(!"ProcessRFQMsg() - unhandled market side");
        side = RFQ_Side::Unknown;
    }

    MarketEvent event;
    _InitMarketEvent(msg, event);
    event.indesc = _GetIndesc(msg);
    event.type = MarketEventType::QuoteRequest;
    event.entry.quote_request.type = RFQ_QuoteType::Tradable;
    event.entry.quote_request.side = side;
    event.entry.quote_request.price = ADAPTER_INVALID_INT;
    const int64_t qty =  msg->GetRequestedSize();
    event.entry.quote_request.quantity = qty;
    event.entry.quote_request.tsExchangeTransact = ns::GetNowEpoch(std::chrono::nanoseconds());

    _SendMarketEvent(event);
    _SendMarketEventEnd(event);
}

void MX_Channel::_Process(const OptionRequestForQuote* msg) {
    _ProcessRFQMsg(msg);
}
void MX_Channel::_Process(const FutureOptionsRequestForQuote* msg) {
    _ProcessRFQMsg(msg);
}
void MX_Channel::_Process(const FuturesRequestForQuote* msg) {
    _ProcessRFQMsg(msg);
}
void MX_Channel::_Process(const StrategyRequestForQuote* msg) {
    _ProcessRFQMsg(msg);
}

void MX_Channel::_Process(const GroupStatus* msg) {
    MX_INFO() << msg->ToString();

    const std::string outrightGroup = msg->GetRootSymbol();
    const char status = msg->GetGroupStatus();
    _UpdateGroupStatus(_outrightGroupToDescs, outrightGroup, status, "outright group");
}

void MX_Channel::_Process(const GroupStatusStrategies* msg) {
    MX_INFO() << "channelId=" << _channelId << ", GroupStatusStrategies=" << msg->ToString();

    const std::string strategyGroup = msg->GetInstrumentGroup();
    const char status = msg->GetGroupStatus();
    _UpdateGroupStatus(_strategyGroupToDescs, strategyGroup, status, "strategy group");
}

void MX_Channel::_UpdateGroupStatus(const std::unordered_map<std::string, std::vector<std::string>>& map, 
                            const std::string& group, 
                            const char status, 
                            const std::string& log) {
    auto it = map.find(group);
    if(std::end(map) == it) {
        return;
    }

    MarketEvent event;
    event.type = MarketEventType::Status;
    event.channelId = _channelId;
    //event.packetSequence = event.messageSequence = msg->msgHeader.GetSeqNum();
    event.tsServerRecv = event.tsExchangeSend = ns::GetNowEpoch(std::chrono::nanoseconds());
    const auto ttStatus = _GetStatus(status);
    event.entry.status.val = ttStatus;

    const std::vector<std::string>& identifiers = it->second;
    for(const std::string& identifier: identifiers) {
        MX_DEBUG() << log << "=" << group 
        << ", identifier=" << identifier 
        << ", status=" << status 
        << ", ttstatus=" << ttStatus;

        _CacheInstrumentStatus(identifier, status);
        event.indesc = consthash(identifier.c_str());
        _SendMarketEvent(event);
        _SendMarketEventEnd(event);
    }    
}

void MX_Channel::_CacheInstrumentStatus(const std::string& identifier, const char status) {
    _cachedStatusMarkerPerInstrument[identifier] = status;
}

void MX_Channel::_CacheInstrumentDefn(const std::string& identifier, const InstrumentDefinition& defn) {
    _instruments[identifier] = defn;
}

void MX_Channel::_Process(const FutureDeliverables* msg) {
    MX_INFO() << "channelId=" << _channelId << ", FutureDeliverablesMsg - " << msg->ToString();
    const std::string identifier = msg->GetIdentifier();
    const int numBonds = msg->GetBondsNum();

    auto it = _instruments.find(identifier);
    if(std::end(_instruments) == it){
        assert(!"Processing FutureDeliverables - identifier not found");
        MX_WARN() << "channelId=" << _channelId << ", Processing FutureDeliverables - identifier=" << identifier << " not found";
        return;
    }

    InstrumentDefinition defn = it->second;

    MX_INFO() << "channelId=" << _channelId << ", FutureDeliverables -"
    << ", identifier=" << identifier
    << ", numBonds=" << numBonds
    ;
    for(int i = 0; i < numBonds; ++i) {
        auto& currentBond = msg->bonds[i];

        defn.productType = ProductType::TBOND;
        const std::string newExchangeTicker = defn.seriesKey;
        defn.exchangeTicker = newExchangeTicker;
 
        const std::string maturityDate = currentBond.GetMaturityDate();
        const int year = std::stoi(maturityDate.substr(0, 4));
		const int month = std::stoi(maturityDate.substr(4, 2));
		const int day = std::stoi(maturityDate.substr(6, 2));
        DateTime ltdDate = GetLTD(maturityDate);
        defn.lastTradeDt = defn.expiryDate = ltdDate;

        const std::string symbol = TrimRight(msg->GetRootSymbol());
        const std::string aliasMonth = GetMonth(msg->GetSymbolMonth());
		const int aliasYear = std::stoi(msg->GetSymbolYear()); //TODO: check if we need tt_pcf_ns::ConvertToInteger

        std::stringstream alias;
		alias << symbol 
        << " " 
        << aliasMonth 
        << aliasYear 
        << " " 
        << std::to_string(year) 
        << "/" 
        << std::setfill('0') 
        << std::setw(2) 
        << std::to_string(month) 
        << "/" << std::setfill('0') 
        << std::setw(2) 
        << std::to_string(day)
        ;

		defn.alias = alias.str();
        defn.productSymbol = symbol;
        const std::string seriesKey = identifier.substr(0, 3) + identifier.substr(6, 3);
        const std::string name = seriesKey + maturityDate + "B";
        defn.securityId = defn.seriesKey = defn.contractInfo = name;

        const double couponRate = GetPrice(currentBond.GetCoupon(), currentBond.GetCouponFI());
        const double conversionFactor = GetPrice(currentBond.GetConversionFactor(), currentBond.GetConversionFactorFI());
        const int64_t outstandingBondValue = currentBond.GetOutstandingBondValue();
        defn.instrumentJSONData.insert({"couponRate", std::to_string(couponRate)});
        defn.instrumentJSONData.insert({"conversionFactor", std::to_string(conversionFactor)});
        defn.instrumentJSONData.insert({"outstandingBondValue", std::to_string(outstandingBondValue)});

        _CompleteInstrumentSetup(msg, defn);

        MX_INFO() << "channelId=" << _channelId << ", FutureDeliverables details -"
        << " id=" << identifier
        << ", securityId=" << defn.securityId
		<< ", seriesKey=" << defn.seriesKey
        << ", symbol=" << defn.productSymbol
		<< ", alias=" << defn.alias
        << ", aliasMonth=" << aliasMonth
        << ", aliasYear=" << aliasYear
		<< ", maturityDay=" << maturityDate
        << ", year=" << year
        << ", month=" << month
        << ", day=" << day
		<< ", exchangeTicker=" << defn.exchangeTicker
        << ", contractInfo=" << defn.contractInfo
		<< ", couponRate=" << defn.instrumentJSONData["couponRate"]
		<< ", outstandingBondValue=" << defn.instrumentJSONData["outstandingBondValue"]
		<< ", conversionFactor=" << defn.instrumentJSONData["conversionFactor"]
        ;

    }
}

/** MX is sending tick table with lower bound whereas we define them with upper bound
    so we need to flip them to match
*/
void MX_Channel::_Process(const TickTable* msg) {
    MX_INFO() << "channelId=" << _channelId << ", TickTableMsg - " << msg->ToString();

    const std::string name = msg->GetTTName();
    const std::string shortName = "TT=" + msg->GetTTShortName();
    const int num = msg->GetEntriesNum();
    const std::vector<TTEntry> entries = msg->GetTTEntries();

    TickTable_t ttTickTable;
    for(auto it = std::rbegin(entries); it != std::rend(entries); ) {
        int64_t upperBound = INT64_MAX;

        const int64_t tickIncrement = it->GetTickPrice();
        const char tickIncrementFI = it->GetTickPriceFractionIndicator();
        double tickSize = GetPrice(tickIncrement, tickIncrementFI);
        const int decimals = ns::GetDecimals(tickSize / tickIncrement, 14, std::numeric_limits<double>::epsilon());
  
        ++it;
        if(it != std::rend(entries)) {
            upperBound = it->GetMinPrice();
        }

        MX_INFO() << "channelId=" << _channelId << ", TickTable details -"
        << " shortName=" << shortName
        << ", tickIncrement=" << tickIncrement
        << ", tickIncrementFI=" << std::string(1, tickIncrementFI)
        << ", tickDenom=" << ExpandWireFactor(decimals)
        << ", tickSize=" << tickSize
        << ", upperBound=" << upperBound
        ;
     
        TickTableEntry tickTableEntry(tickIncrement, ExpandWireFactor(decimals), upperBound);
        ttTickTable.push_back(tickTableEntry);
    }

    _tickTables[shortName] = ttTickTable;
}

void MX_Channel::_Process(const StartOfDay* msg) {
    MX_INFO() << "channelId=" << _channelId << ", StartOfDay=" << msg->GetBusinessDate() << ", inRecovery=" << _inRecovery;

    if(_inRecovery)
        return;
    
    _GoStable();
}

void MX_Channel::_Process(const EndOfTransmission* msg) {
    MX_INFO() << "channelId=" << _channelId << ", EndOfTransmission - " << msg->ToString();
}

void MX_Channel::_Process(const Heartbeat* msg) {
    MX_DEBUG() << "channelId=" << _channelId << ", Got heartbeat=" << msg->ToString();
}

//Empty implementations
void MX_Channel::_Process(const BeginningOfOptionsSummary* msg)
{
}
void MX_Channel::_Process(const BeginningOfFutureOptionsSummary* msg)
{
}
void MX_Channel::_Process(const BeginningOfFuturesSummary* msg)
{
}
void MX_Channel::_Process(const BeginningOfStrategySummary* msg)
{
}
void MX_Channel::_Process(const EndOfSales* msg)
{
}


//Helper methods
template<typename TradeMsgT>
long long MX_Channel::_GetTradeTime(const TradeMsgT* msg) {
    const int hours = std::stoi(msg->msgHeader.GetTimestamp().substr(8, 2));
    const int minutes = std::stoi(msg->msgHeader.GetTimestamp().substr(10, 2));
    const int seconds = std::stoi(msg->msgHeader.GetTimestamp().substr(12, 2));
    const int milliseconds = std::stoi(msg->msgHeader.GetTimestamp().substr(14, 3));

    assert(!    (hours < 0 || 
                hours >= 24 || 
                minutes < 0 || 
                minutes >= 60 || 
                seconds < 0 || 
                seconds >= 60 || 
                milliseconds < 0 || 
                milliseconds > 999) 
            );

    using days = std::chrono::duration<int, std::ratio<86400>>;
    const std::chrono::milliseconds lastMidnight = std::chrono::time_point_cast<days>(std::chrono::system_clock::now()).time_since_epoch();
    const long long timestamp = (lastMidnight.count() + _GetNumMilliSecondsSinceMidnightUTC(hours, minutes, seconds, milliseconds)) * 1000;
    return timestamp;
}

long long MX_Channel::_GetNumMilliSecondsSinceMidnightUTC(int h, int m, int s, int ms) const {
    using namespace std::chrono;
    
    const milliseconds total_milliseconds = duration_cast<milliseconds>(
        hours(h) + minutes(m) + seconds(s) + milliseconds(ms)
    );
    
    return total_milliseconds.count();
}

std::string MX_Channel::_GetMetaDataAsStr(const InstrumentDefinition& defn) const {
    std::string metaData;
    for(const auto& e: defn.instrumentJSONData) {
        metaData += "[" + e.first + ":" + e.second + "], ";
    }
    return metaData;
}

void MX_Channel::_GoStable() const {
    MX_INFO() << "channelId=" << _channelId << ", Going stable for channelId=" << _channelId;
    _sendApi->OnChannelStatus(_channelId, ChannelStatus::Stable);
}

MarketBookSide MX_Channel::_GetLegSideFromFI(const char fractionIndicator) const {
    return fractionIndicator >= 'A' && fractionIndicator <= 'G' ? MarketBookSide::Ask : MarketBookSide::Bid;
}

bool MX_Channel::_LegsAvailable(const std::vector<std::string>& legs) const {
    for(const auto& leg: legs) {
        auto it = _outrights.find(leg);
        if(std::end(_outrights) == it) {
            return false;
        }
    }
    return true;
}

CurrencyCode::Value MX_Channel::_ToCurrencyCode(const std::string& currency) const {
    if( currency == "BRL" )             return CurrencyCode::BRL;
    else if( currency == "GBP" )        return CurrencyCode::GBP;
    else if( currency == "EUR" )        return CurrencyCode::EUR;
    else if( currency == "USD" )        return CurrencyCode::USD;
    else if( currency == "ARS" )        return CurrencyCode::ARS;
    else if( currency == "MXN" )        return CurrencyCode::MXN;
    else if( currency == "SEK" )        return CurrencyCode::SEK;
    else if( currency == "TRY" )        return CurrencyCode::TRY;
    else if( currency == "ZAR" )        return CurrencyCode::ZAR;
    else if( currency == "JPY" )        return CurrencyCode::JPY;
    else if( currency == "CAD" )        return CurrencyCode::CAD;
    else if( currency == "CLP" )        return CurrencyCode::CLP;
    else if( currency == "CHF" )        return CurrencyCode::CHF;
    else if( currency == "CNY" )        return CurrencyCode::CNY;
    else if( currency == "NOK" )        return CurrencyCode::NOK;
    else if( currency == "RUB" )        return CurrencyCode::RUB;
    else {
        assert(!"ToCurrencyCode() - unhandled currency");
        return CurrencyCode::Unknown;   
    }
}

InstrumentStatus::Value MX_Channel::_GetStatus(const char status) const {
    switch(status) {
    case StatusMarker::PreOpening:                  return InstrumentStatus::PreOpen;
    case StatusMarker::Opening:                     return InstrumentStatus::Auction;
    case StatusMarker::ContinuousTrading:           return InstrumentStatus::Open;
    case StatusMarker::Forbidden:                   return InstrumentStatus::Closed;
    case StatusMarker::InterventionBeforeOpening:   return InstrumentStatus::PreTrading;
    case StatusMarker::HaltedTrading:               return InstrumentStatus::Freeze;
    case StatusMarker::Reserved:                    return InstrumentStatus::Reserve;
    case StatusMarker::Suspended:                   return InstrumentStatus::PreOpen;
    case StatusMarker::SurveillanceIntervention:    return InstrumentStatus::PostTrading;
    case StatusMarker::EndOfDayInquiries:           return InstrumentStatus::Closed;
    case StatusMarker::IfNotUsed:                   return InstrumentStatus::Open;
    default: {
        assert(!"GetStatus() - unhandled status");
        return InstrumentStatus::Unknown;
    }
    }    
}

void MX_Channel::_ResetBook(const Descriptor_t indesc) const {
    MarketEvent event;
    event.channelId = _channelId;
    event.indesc = indesc;
    event.type = MarketEventType::BookReset;
    _sendApi->OnSnapshot(&event); 
}

bool MX_Channel::_IsStartupRetransmission() const {
    return _fromSeq == 1;
}


//CDD processing
template<typename InstrumentKeysMsgT>
void MX_Channel::_HandleCDDField(const InstrumentKeysMsgT* msg, InstrumentDefinition& defn) {
    static std::set<std::string> quarterlies = {"SXA", 
                                                "SXB", 
                                                "SXH", 
                                                "SXF", 
                                                "SXY", 
                                                "SXM", 
                                                "SXU", 
                                                "SXK", 
                                                "SCF", 
                                                "BSF"
                                               };

    unsigned short contractYear, contractMonth, contractDay, termMonth = 0, termYear = 0;
    _DecodeExpiry(msg, defn.productSymbol, contractYear, contractMonth, contractDay, termMonth, termYear);

    if(contractMonth > 0 && contractMonth < 13) {
        uint8_t quarter = GetQuarter(contractMonth);

        if(quarterlies.count(defn.productSymbol) > 0) {
            defn.termType = SeriesTermType::QUARTER;
            defn.instrumentJSONData.insert({"cdd", GetCddString(contractYear, contractMonth, contractDay, 0 , 0, quarter)});
        } else if(defn.productSymbol == "CRA") {
            defn.termType = SeriesTermType::MONTH;
            defn.instrumentJSONData.insert({"cdd", GetCddString(termYear, termMonth, contractDay)});
        } else {
            defn.termType = SeriesTermType::MONTH;
            defn.instrumentJSONData.insert({"cdd", GetCddString(contractYear, contractMonth, contractDay)});
        }
    }
}

template<typename InstrumentKeysMsgT>
void MX_Channel::_DecodeExpiry(const InstrumentKeysMsgT* msg, 
                    const std::string& productSymbol, 
                    unsigned short& contractYear,
                    unsigned short& contractMonth, 
                    unsigned short& contractDay, 
                    unsigned short& termMonth, 
                    unsigned short& termYear) const {

    const std::string symbolYear = msg->GetSymbolYear();
    contractYear = _DecodeYear(std::stoi(symbolYear));
    contractMonth = DecodeMonth(msg->GetSymbolMonth());
    contractDay = std::stoi(msg->GetExpiryDay());

    if(productSymbol == "CRA") {
        using namespace boost::gregorian;
        date d(contractYear, contractMonth, contractDay);
        d += months(3);
        greg_month mo(d.month());

        nth_day_of_the_week_in_month ndm(nth_day_of_the_week_in_month::third, Wednesday, mo);
        date nd = ndm.get_date(d.year());
        nd -= days(1);

        termMonth = contractMonth;
        termYear = contractYear;
        contractYear = nd.year();
        contractMonth = nd.month();
        contractDay = nd.day();
    }
}

template<>
void MX_Channel::_DecodeExpiry(const OptionInstrumentKeys* msg,
                    const std::string& productSymbol, 
                    unsigned short& contractYear,
                    unsigned short& contractMonth, 
                    unsigned short& contractDay, 
                    unsigned short& termMonth, 
                    unsigned short& termYear) const {

    const int decodedOptionMonth = DecodeOptionMonth(msg->GetExpiryMonth());
    contractYear = std::stoi(msg->GetExpiryYear()) + 2000;
    contractMonth = decodedOptionMonth;
    contractDay = std::stoi(msg->GetExpiryDay());
}

template<>
void MX_Channel::_DecodeExpiry(const StrategyInstrumentKeys* msg,
                    const std::string& productSymbol, 
                    unsigned short& contractYear,
                    unsigned short& contractMonth, 
                    unsigned short& contractDay, 
                    unsigned short& termMonth, 
                    unsigned short& termYear) const {

    std::string expiryYear = msg->GetExpiryYear();
    contractYear = _DecodeYear(std::stoi(expiryYear));
    const std::string marketFlowIndicator = msg->GetMarketFlowIndicator();
    contractMonth = 'W' == marketFlowIndicator[0] ? DecodeStrategyMonth(msg->GetExpiryMonth()) : DecodeMonth(msg->GetExpiryMonth());
    contractDay = std::stoi(msg->GetExpiryDay());
}

int MX_Channel::_GetCurrentYear() const {
    auto now = std::chrono::system_clock::now();
    std::time_t currentTime = std::chrono::system_clock::to_time_t(now);
    std::tm* localTime = std::localtime(&currentTime);
    int currentYear = localTime->tm_year + 1900;
    return currentYear;
}

int MX_Channel::_DecodeYear(const int year) const {
    int currentYear = _GetCurrentYear();
    int remainderOfYear = currentYear % 100;
    int ret = year + currentYear - remainderOfYear;
    return ret;
}


inline void MX_Channel::_SendMarketEvent(MarketEvent& event) const {
    _sendApi->OnIncremental(&event);
}

inline void MX_Channel::_SendMarketEventEnd(MarketEvent& event) const{
    if(_inRecovery)
        return;

    auto previousType = event.type;
    event.type = MarketEventType::EventEnd;
    _sendApi->OnIncremental(&event); 
    event.type = previousType;
}

uint16_t MX_Channel::_GetMessageCount(const PacketBufferPtr& packetBuffer, std::string& types) const {
    char* readPtr = packetBuffer->m_buffer;
    const uint32_t bytesReceived = packetBuffer->m_bytesReceived;

    uint16_t count = 0;
    while(readPtr - packetBuffer->m_buffer < bytesReceived) {
        assert(*readPtr == STX);
        const MsgHeader* header = reinterpret_cast<const MsgHeader*>(readPtr + sizeof(STX));

        std::string msgType = header->GetMsgType();
        if(!msgType.empty() && msgType.back() == ' ') {
            msgType.pop_back();
        }

        types += msgType + "(" + std::to_string(header->GetSeqNum()) + ")" + ",";

        ++count; //We have a msg
        auto it = std::find(readPtr, packetBuffer->m_buffer + bytesReceived, ETX);
        assert(*it == ETX && (it != packetBuffer->m_buffer + bytesReceived));
        readPtr = ++it;
    }

    assert(count != 0);
    return count;
}


void MX_Channel::OnReplayTcpData(const char* buf, size_t len) {
    ProcessReplayData((char*) buf, len);
}

void MX_Channel::ProcessReplayData(char* readPtr, size_t len) {
    MX_DEBUG() << "channelId=" << _channelId << ", ProcessReplayData";
}

MulticastFeedPtrT MX_Channel::GetRealTimeFeed() const {
    return _realtimeFeed;
}

void MX_Channel::Post(std::function<void()> fn) {
    _workerThread->Post(fn);
}

}//end namespace

