#include "eobi/eobi_product_manager.h"

EOBIProductManger::EOBIProductManger(IAdapterSend* sendApi, const ID id) 
    : _sendApi(sendApi)
    , _id(id)
{
}

EOBIProductManger::~EOBIProductManger() {
}

void EOBIProductManger::OnSnapshotData(const MessageMeta& mm) {
    if(!_inRecovery) {
        return;
    }

    const auto& packetBuffer = mm.pb;
    char* readPtr = packetBuffer->m_buffer;
    const PacketHeaderT* packetHeader = reinterpret_cast<const PacketHeaderT*>(readPtr);
    const MarketSegmentIdT marketSegmentId = packetHeader->MarketSegmentID;

    assert(_id == marketSegmentId);
    
    bool exitSnapshot = false;
    readPtr += sizeof(PacketHeaderT);
    while(!exitSnapshot && readPtr - packetBuffer->m_buffer < packetBuffer->m_bytesReceived) {
        const MessageHeaderCompT* header = reinterpret_cast<const MessageHeaderCompT*>(readPtr);
        const uint16_t templateId = header->TemplateID;

        EOBI_INFO() << "Snapshot -"
        << ", Id=" << _id 
        << ", TemplateId=" << templateId 
        << ", MsgSeqNum=" << header->MsgSeqNum 
        ;
        
        switch(templateId) {
        case TID_PRODUCTSUMMARY: {
            if(IsValid(_snapshotLastMsgSeqNum)) {
                exitSnapshot = true;
                break;
            }

            const ProductSummaryT* msg = reinterpret_cast<const ProductSummaryT*>(readPtr);      
            if(_IsSnapshotLoopValid(msg)) {
                _Process(msg);
            }
        }
        break;
        case TID_INSTRUMENTSUMMARY: {
            if(IsValid(_snapshotLastMsgSeqNum)) {
                const InstrumentSummaryT* msg = reinterpret_cast<const InstrumentSummaryT*>(readPtr);         
                _Process(msg);
            }
        }
        break;
        case TID_SNAPSHOTORDER: {
            if(IsValid(_snapshotLastMsgSeqNum) && IsValid(_snapshotSecurityId)) {
                const SnapshotOrderT* msg = reinterpret_cast<const SnapshotOrderT*>(readPtr);
                _Process(msg);
            }
        }
        break;
        default: {
            assert(!"Processing snapshot - Unhandled templateId!");
            EOBI_WARN() << "Processing snapshot - Id=" << _id << ", Unhandled templateId=" << templateId;
        }
        break;

        }//end switch

        readPtr += header->BodyLen;
    }//end while loop

    if(exitSnapshot) {
        EOBI_INFO() << "Snapshot - exiting";
        _OnSnapshotComplete();
    }
}

void EOBIProductManger::_Process(const ProductSummaryT* msg) {
    EOBI_INFO() << "ProductSummaryT - mapping Id=" << _id << " to lastMsgSeqNum=" << msg->LastMsgSeqNumProcessed;
    _snapshotLastMsgSeqNum = msg->LastMsgSeqNumProcessed;
}

void EOBIProductManger::_Process(const InstrumentSummaryT* msg) {
    EOBI_INFO() << "InstrumentSummaryT - securityId=" << msg->SecurityID 
    << ", lastMsgSeqNum=" << _snapshotLastMsgSeqNum 
    << ", entries=" << +msg->NoMDEntries
    << ", securityStatus=" << +msg->SecurityStatus
    << ", securityTradingStatus=" << +msg->SecurityTradingStatus
    << ", fastMarketIndicator=" << +msg->FastMarketIndicator
    ;

    const bool isSnapshot = true;
    _snapshotSecurityId = msg->SecurityID;
    
    const uint64_t now = ns::GetNowEpoch(std::chrono::nanoseconds());
    _HandleInstrumentStatus(msg->SecurityID, 
                            msg->SecurityStatus,
                            msg->SecurityTradingStatus,
                            msg->FastMarketIndicator,
                            now,
                            isSnapshot);

    const int statEntriesCount = msg->NoMDEntries;
    for(int i = 0; i < statEntriesCount; ++i) {
        auto currentEntry = msg->MDInstrumentEntryGrp[i];

        StatPriceID::Value priceId;
        const uint8_t entryType = currentEntry.MDEntryType;
        switch(entryType) {
        case ENUM_MDENTRYTYPE_LOWPRICE: priceId = StatPriceID::Low; break;
        case ENUM_MDENTRYTYPE_HIGHPRICE: priceId = StatPriceID::High; break;
        case ENUM_MDENTRYTYPE_OPENINGPRICE: priceId = StatPriceID::Open; break;
        case ENUM_MDENTRYTYPE_CLOSINGPRICE: priceId = StatPriceID::Close; break;
        case ENUM_MDENTRYTYPE_TRADEVOLUME: {
            _HandleTradeVolume(msg->SecurityID, 
                                currentEntry.MDEntrySize,
                                isSnapshot);
            continue;
        }
        default: {
            EOBI_INFO() << "Unhandled stat price=" << +entryType;
            assert(!"Unhandled stat price");
        }
        }//end switch

        _HandleStatPrice(msg->SecurityID, 
                        priceId, 
                        currentEntry.MDEntryPx,
                        isSnapshot);
    } 
}

void EOBIProductManger::_Process(const SnapshotOrderT* msg) {
    EOBI_INFO() << "SnapshotOrderT - securityId=" << _snapshotSecurityId
    << ", side=" << GetSideAsString(msg->OrderDetails.Side)
    << ", price=" << msg->OrderDetails.Price
    << ", qty=" << msg->OrderDetails.DisplayQty
    << ", orderId=" << msg->OrderDetails.TrdRegTSTimePriority
    << ", priority=" << msg->OrderDetails.TrdRegTSTimePriority
    << ", lastMsgSeqNum=" << _snapshotLastMsgSeqNum
    ;

    MarketEvent event;
    event.type = MarketEventType::OrderBook;
    event.entry.order_book.action = MarketUpdateAction::New;
    event.indesc = _snapshotSecurityId;
    event.entry.order_book.side = GetSide(msg->OrderDetails.Side);
    event.entry.order_book.price = msg->OrderDetails.Price;
    event.entry.order_book.quantity = msg->OrderDetails.DisplayQty;
    event.entry.order_book.orderId = msg->OrderDetails.TrdRegTSTimePriority;
    event.entry.order_book.priority = msg->OrderDetails.TrdRegTSTimePriority;

    _SendOnSnapshot(event);
}

void EOBIProductManger::_OnSnapshotComplete() {
    _ProcessEOBIBufferedMsgs();
    _snapshotLastMsgSeqNum = NO_VALUE_UINT;
    _snapshotSecurityId = NO_VALUE_SLONG;
    _snapshotSeqNum = NO_VALUE_UINT;
    _SendSnapshotEnd();
    _currentDescs.clear();
    _inRecovery = false;
    EOBI_INFO() << "Snapshot - complete";
}

void EOBIProductManger::_ProcessEOBIBufferedMsgs() {
    EOBI_INFO() << "Snapshot - beginning processing buffered msgs, size=" << _bufferedEOBIMsgs.size() 
    << ", inRecovery=" << _inRecovery;

    while(!_bufferedEOBIMsgs.empty()) {
        const auto& currentPacket = _bufferedEOBIMsgs.front();
        const auto& packetBuffer = currentPacket.pb;

        char* readPtr = packetBuffer->m_buffer;
        const PacketHeaderT* packetHeader = reinterpret_cast<const PacketHeaderT*>(readPtr);
        const MarketSegmentIdT marketSegmentId = packetHeader->MarketSegmentID;
        assert(_id == marketSegmentId);
       
        readPtr += sizeof(PacketHeaderT);
        while(readPtr - packetBuffer->m_buffer < packetBuffer->m_bytesReceived) {
            const MessageHeaderCompT* header = reinterpret_cast<const MessageHeaderCompT*>(readPtr);
            const uint32_t msgSeqNum = header->MsgSeqNum;

            if(msgSeqNum > _snapshotLastMsgSeqNum) {
                EOBI_INFO() << "Processing buffered msg - Id=" << _id 
                << ", TemplateId=" << header->TemplateID 
                << ", MsgSeqNum=" << msgSeqNum
                << ", SnapshotLMSN=" << _snapshotLastMsgSeqNum
                ;

                _OnEOBIMsg(readPtr, header->TemplateID, msgSeqNum);
            } else {
                EOBI_INFO() << "Stale msg - Id=" << _id 
                << ", TemplateId=" << header->TemplateID 
                << ", MsgSeqNum=" << msgSeqNum
                << ", SnapshotLMSN=" << _snapshotLastMsgSeqNum
                ;
            }

            readPtr += header->BodyLen; //Move to the next msg
        }

        _bufferedEOBIMsgs.pop_front();
    }

    assert(_bufferedEOBIMsgs.size() == 0);
    EOBI_INFO() << "Snapshot - finished processing buffered msgs, size=" << _bufferedEOBIMsgs.size() 
    << ", inRecovery=" << _inRecovery;
}


void EOBIProductManger::OnIncrementalData(const MessageMeta& mm) {
    const auto& packetBuffer = mm.pb;
    const MsgSeqNumT msgSeqNum = _GetMsgSeqNum(packetBuffer);

    const int sequenceDiff = msgSeqNum - _lastSeqNum;
    if(sequenceDiff > 1 || _inRecovery) {

        if(_bufferingSkipLogCounter % 100 == 0){
            EOBI_INFO() << "Gap detected -"
            << " Id=" << _id
            << ", lastSeqNum=" << _lastSeqNum
            << ", currentSeqNum=" << msgSeqNum
            << ", diff=" << sequenceDiff
            << ", inSnapshot=" << _inRecovery
            << ". Buffering";
        }
        ++_bufferingSkipLogCounter;

        _bufferedEOBIMsgs.push_back(mm);

        if(!_inRecovery) {
            _inRecovery = true;
            _snapshotSeqNum = msgSeqNum;
        }
        
        return;
    }

    _OnEOBIPacket(packetBuffer);
}

MsgSeqNumT EOBIProductManger::_GetMsgSeqNum(const PacketBufferPtr packetBuffer) const {
    char* readPtr = packetBuffer->m_buffer;
    readPtr += sizeof(PacketHeaderT);

    const MessageHeaderCompT* header = reinterpret_cast<const MessageHeaderCompT*>(readPtr);
    return header->MsgSeqNum;
}

void EOBIProductManger::_OnEOBIPacket(const PacketBufferPtr packetBuffer) {
    char* readPtr = packetBuffer->m_buffer;
    const PacketHeaderT* packetHeader = reinterpret_cast<const PacketHeaderT*>(readPtr);
    const MarketSegmentIdT marketSegmentId = packetHeader->MarketSegmentID;
    assert(_id == marketSegmentId);

    readPtr += sizeof(PacketHeaderT);
    while(readPtr - packetBuffer->m_buffer < packetBuffer->m_bytesReceived) {
        const MessageHeaderCompT* header = reinterpret_cast<const MessageHeaderCompT*>(readPtr);
        const uint16_t templateId = header->TemplateID;
        const MsgSeqNumT msgSeqNum = header->MsgSeqNum;

        EOBI_INFO() << "Incremental msg - Id=" << _id 
        << ", TemplateId=" << templateId 
        << ", MsgSeqNum=" << msgSeqNum 
        << ", CompletionIndicator=" << +packetHeader->CompletionIndicator
        ;
        
        _OnEOBIMsg(readPtr, templateId, msgSeqNum);
        readPtr += header->BodyLen;
    }

    if(packetHeader->CompletionIndicator == ENUM_COMPLETION_INDICATOR_COMPLETE) {
        _OnCompletionIndicatorComplete();
    }
}

void EOBIProductManger::_OnEOBIMsg(char* msgPtr, const uint16_t templateId, const MsgSeqNumT msgSeqNum) {

    _lastSeqNum = msgSeqNum;

    switch(templateId) {
    case TID_ORDER_ADD: {
        _ProcessAndAddSecurityId<OrderAddT>(msgPtr);
    }
    break;
    case TID_ORDER_DELETE: {
        _ProcessAndAddSecurityId<OrderDeleteT>(msgPtr);        
    }
    break;
    case TID_ORDER_MODIFY: {
        _ProcessAndAddSecurityId<OrderModifyT>(msgPtr); 
    }
    break;
    case TID_ORDER_MODIFY_SAME_PRIO: {
        _ProcessAndAddSecurityId<OrderModifySamePrioT>(msgPtr); 
    }
    break;
    case TID_ORDER_MASS_DELETE: {
        _ProcessAndAddSecurityId<OrderMassDeleteT>(msgPtr);
    }
    break;
    case TID_TRADE_REPORT: {
        _ProcessAndAddSecurityId<TradeReportT>(msgPtr);
    }
    break;
    case TID_FULL_ORDER_EXECUTION: {
        _ProcessAndAddSecurityId<FullOrderExecutionT>(msgPtr);
    }
    break;
    case TID_PARTIAL_ORDER_EXECUTION: {
        _ProcessAndAddSecurityId<PartialOrderExecutionT>(msgPtr);
    }
    break;
    case TID_EXECUTION_SUMMARY: {
        _ProcessAndAddSecurityId<ExecutionSummaryT>(msgPtr);
    }
    break;
    case TID_INSTRUMENT_STATE_CHANGE: {
        _ProcessAndAddSecurityId<InstrumentStateChangeT>(msgPtr);
    }
    break;
    case TID_QUOTE_REQUEST: {
        _ProcessAndAddSecurityId<QuoteRequestT>(msgPtr);
    }
    break;
    case TID_CROSS_REQUEST: {
        _ProcessAndAddSecurityId<CrossRequestT>(msgPtr);
    }
    break;
    case TID_AUCTION_BBO: {
        _ProcessAndAddSecurityId<AuctionBBOT>(msgPtr);
    }
    break;
    case TID_AUCTION_CLEARING_PRICE: {
        _ProcessAndAddSecurityId<AuctionClearingPriceT>(msgPtr);
    }
    break;
    case TID_PRODUCT_STATE_CHANGE: {
        const ProductStateChangeT* msg = reinterpret_cast<const ProductStateChangeT*>(msgPtr);
        _Process(msg);
    }
    break;
    case TID_HEARTBEAT: {
        const HeartbeatT* msg = reinterpret_cast<const HeartbeatT*>(msgPtr);
        _Process(msg);
    }
    break;

    default: {

    }
    break;
    }//end switch
}

template <typename MsgT>
void EOBIProductManger::_ProcessAndAddSecurityId(char* msgPtr) {
    const MsgT* msg = reinterpret_cast<const MsgT*>(msgPtr);
    _Process(msg);
    _AddSecurityId(msg);
}

template<typename MsgT>
void EOBIProductManger::_AddSecurityId(const MsgT* msg) {
    _currentDescs.insert(msg->SecurityID);
}

void EOBIProductManger::_Process(const OrderAddT* msg) {
    EOBI_INFO() << "OrderAddT - securityId=" << msg->SecurityID
    << ", side=" << GetSideAsString(msg->OrderDetails.Side)
    << ", price=" << msg->OrderDetails.Price
    << ", qty=" << msg->OrderDetails.DisplayQty
    << ", orderId=" << msg->OrderDetails.TrdRegTSTimePriority
    << ", priority=" << msg->OrderDetails.TrdRegTSTimePriority
    ;

    _AddOrder(msg->SecurityID, 
              msg->OrderDetails.Side, 
              msg->OrderDetails.Price, 
              msg->OrderDetails.DisplayQty, 
              msg->OrderDetails.TrdRegTSTimePriority);
}

void EOBIProductManger::_Process(const OrderDeleteT* msg) {
    EOBI_INFO() << "OrderDeleteT - securityId=" << msg->SecurityID
    << ", side=" << GetSideAsString(msg->OrderDetails.Side)
    << ", price=" << msg->OrderDetails.Price
    << ", qty=" << msg->OrderDetails.DisplayQty
    << ", orderId=" << msg->OrderDetails.TrdRegTSTimePriority
    << ", priority=" << msg->OrderDetails.TrdRegTSTimePriority
    ;

    _DeleteOrder(msg->SecurityID, 
                msg->OrderDetails.Side, 
                msg->OrderDetails.TrdRegTSTimePriority);
}

void EOBIProductManger::_Process(const OrderModifyT* msg) {
    EOBI_INFO() << "OrderModifyT - securityId=" << msg->SecurityID
    << ", previousOrderId=" << msg->TrdRegTSPrevTimePriority
    << ", prevPrice=" << msg->PrevPrice
    << ", prevQty=" << msg->PrevDisplayQty
    << ", newSide=" << GetSideAsString(msg->OrderDetails.Side)
    << ", newPrice=" << msg->OrderDetails.Price
    << ", newQty=" << msg->OrderDetails.DisplayQty
    << ", newOrderId=" << msg->OrderDetails.TrdRegTSTimePriority
    << ", newPriority=" << msg->OrderDetails.TrdRegTSTimePriority
    ;

    _DeleteOrder(msg->SecurityID, 
                 msg->OrderDetails.Side, 
                 msg->TrdRegTSPrevTimePriority);
    _AddOrder(msg->SecurityID, 
              msg->OrderDetails.Side, 
              msg->OrderDetails.Price, 
              msg->OrderDetails.DisplayQty, 
              msg->OrderDetails.TrdRegTSTimePriority);
}

void EOBIProductManger::_Process(const OrderModifySamePrioT* msg) {
    EOBI_INFO() << "OrderModifySamePrioT - securityId=" << msg->SecurityID
    << ", side=" << GetSideAsString(msg->OrderDetails.Side)
    << ", price=" << msg->OrderDetails.Price
    << ", qty=" << msg->OrderDetails.DisplayQty
    << ", orderId=" << msg->OrderDetails.TrdRegTSTimePriority
    << ", priority=" << msg->OrderDetails.TrdRegTSTimePriority
    ;

    MarketEvent event;
    event.type = MarketEventType::OrderBook;
    event.entry.order_book.action = MarketUpdateAction::Change;
    event.indesc = msg->SecurityID;
    event.entry.order_book.side = GetSide(msg->OrderDetails.Side);
    event.entry.order_book.price = msg->OrderDetails.Price;
    event.entry.order_book.quantity = msg->OrderDetails.DisplayQty;
    event.entry.order_book.orderId = msg->OrderDetails.TrdRegTSTimePriority;
    event.entry.order_book.priority = msg->OrderDetails.TrdRegTSTimePriority;
    _SendMarketEvent(event);
}

void EOBIProductManger::_Process(const OrderMassDeleteT* msg) {
    EOBI_INFO() << "OrderMassDeleteT - securityId=" << msg->SecurityID;
    _ClearOrderBook(msg->SecurityID);
}

//Just log
void EOBIProductManger::_Process(const TradeReportT* msg) {
    EOBI_INFO() << "TradeReportT - securityId=" << msg->SecurityID
    << ", price=" << msg->LastPx
    << ", qty=" << msg->LastQty
    << ", tradeMatchId=" << msg->TrdMatchID
    << ", matchType=" << +msg->MatchType
    << ", matchSubType=" << +msg->MatchSubType
    << ", tradeCondition=" << msg->TradeCondition
    << ", algorithmicTradeIndicator=" << +msg->AlgorithmicTradeIndicator
    ;
}

template<typename OrderExecutionMsgT>
void EOBIProductManger::_ProcessOrderExecution(const OrderExecutionMsgT* msg) {
    EOBI_INFO() << "OrderExecutionMsgT - securityId=" << msg->SecurityID
    << ", side=" << GetSideAsString(msg->Side)
    << ", price=" << msg->Price
    << ", qty=" << msg->LastQty
    << ", orderId=" << msg->TrdRegTSTimePriority
    << ", priority=" << msg->TrdRegTSTimePriority
    ;

    MarketEvent event;
    event.type = MarketEventType::OrderBook;
    event.entry.order_book.action = MarketUpdateAction::Execute;
    event.indesc = msg->SecurityID;
    event.entry.order_book.side = GetSide(msg->Side);
    event.entry.order_book.price = msg->Price;
    event.entry.order_book.quantity = msg->LastQty;
    event.entry.order_book.orderId = msg->TrdRegTSTimePriority;
    event.entry.order_book.priority = msg->TrdRegTSTimePriority;
    _SendMarketEvent(event);
}

void EOBIProductManger::_Process(const PartialOrderExecutionT* msg) {
    _ProcessOrderExecution(msg);
}
void EOBIProductManger::_Process(const FullOrderExecutionT* msg) {
    _ProcessOrderExecution(msg);
}

void EOBIProductManger::_Process(const ExecutionSummaryT* msg) {
    EOBI_INFO() << "ExecutionSummaryT - securityId=" << msg->SecurityID
    << ", agressorSide=" << GetSideAsString(msg->AggressorSide)
    << ", price=" << msg->LastPx
    << ", qty=" << msg->LastQty
    << ", execId=" << msg->ExecID
    << ", tradeCondition=" << +msg->TradeCondition
   //<< ", tradingHHIIndicator=" << +msg->TradingHHIIndicator
    ;

    MarketEvent event;
    event.type = MarketEventType::Trade;
    event.indesc = msg->SecurityID;
    event.entry.trade.orderId = msg->ExecID;
    event.entry.trade.status = TradeStatus::Regular;
    event.entry.trade.qualifier = msg->TradeCondition == 1
                                ? TradeQualifier::Value::ImpliedTrade 
                                : TradeQualifier::Value::Regular;
    event.entry.trade.side = GetHitOrTake(msg->AggressorSide);
    event.entry.trade.price = msg->LastPx;
    event.entry.trade.quantity = msg->LastQty;
    event.entry.trade.tsTrade = msg->ExecID;
    event.entry.trade.tsExchangeTransact = 0;
    event.entry.trade.bidCounterPartyId = 0;
    event.entry.trade.askCounterPartyId = 0;

    _SendMarketEvent(event);
}

void EOBIProductManger::_Process(const ProductStateChangeT* msg) {
    EOBI_INFO() << "ProductStateChangeT - Id=" << _id
    << ", tradingSessionSubID=" << msg->TradingSessionSubID
    << ", tradSesStatus=" << msg->TradSesStatus
    << ", marketCondition=" << msg->MarketCondition
    << ", fastMarketIndicator=" << msg->FastMarketIndicator
    << ", tesTradSesStatus=" << msg->TESTradSesStatus
    ;

    _HandleProductStatus(msg->TradingSessionSubID);
}

void EOBIProductManger::_Process(const InstrumentStateChangeT* msg) {
    EOBI_INFO() << "InstrumentStateChangeT - securityId=" << msg->SecurityID
    << ", securityStatus=" << msg->SecurityStatus
    << ", securityTradingStatus=" << msg->SecurityTradingStatus
    << ", fastMarketIndicator=" << msg->FastMarketIndicator
    //<< ", ttStatus=" << status
    << ", marketCondition=" << msg->MarketCondition
    << ", highPrice=" << msg->HighPx
    << ", lowPrice=" << msg->LowPx
    ;

    _HandleInstrumentStatus(msg->SecurityID, 
                            msg->SecurityStatus,
                            msg->SecurityTradingStatus,
                            msg->FastMarketIndicator,
                            msg->TransactTime);
}

void EOBIProductManger::_Process(const QuoteRequestT* msg) {
    EOBI_INFO() << "QuoteRequestT - securityId=" << msg->SecurityID
    << ", side=" << GetSideAsString(msg->Side)
    << ", qty=" << msg->LastQty
    ;

    RFQ_Side::Value side;
    switch(msg->Side) {
    case 1: side = RFQ_Side::Buy; break;
    case 2: side = RFQ_Side::Sell; break;
    default:
        assert(!"QuoteRequestT - unhandled msg side");
        side = RFQ_Side::Unknown;
    }

    MarketEvent event;
    event.type = MarketEventType::QuoteRequest;
    event.entry.quote_request.type = RFQ_QuoteType::Tradable;
    event.indesc = msg->SecurityID;
    event.entry.quote_request.side = side;
    event.entry.quote_request.price = ADAPTER_INVALID_INT;
    event.entry.quote_request.quantity = msg->LastQty;
    event.entry.quote_request.tsExchangeTransact = ns::GetNowEpoch(std::chrono::nanoseconds());
    _SendMarketEvent(event);
}

void EOBIProductManger::_Process(const CrossRequestT* msg) {
    EOBI_INFO() << "CrossRequestT - securityId=" << msg->SecurityID
    << ", side=" << GetSideAsString(msg->Side)
    << ", price=" << msg->LastPx
    << ", qty=" << msg->LastQty
    ;

    MarketEvent event;
    event.type = MarketEventType::QuoteRequest;
    event.entry.quote_request.type = RFQ_QuoteType::CrossTradeRequest;
    event.indesc = msg->SecurityID;
    event.entry.quote_request.side = RFQ_Side::Cross;
    event.entry.quote_request.price = msg->LastPx;
    event.entry.quote_request.quantity = msg->LastQty;
    event.entry.quote_request.tsExchangeTransact = ns::GetNowEpoch(std::chrono::nanoseconds());
    _SendMarketEvent(event);
}

void EOBIProductManger::_Process(const AuctionBBOT* msg) {
    //TODO:
}
void EOBIProductManger::_Process(const AuctionClearingPriceT* msg) {
    //TODO:
}

void EOBIProductManger::_Process(const HeartbeatT* msg) {
    const MsgSeqNumT lastMsgSeqNumProcessed = msg->LastMsgSeqNumProcessed;
    EOBI_INFO() << "Got heartbeat - LastMsgSeqNumProcessed=" << lastMsgSeqNumProcessed << ", current=" << _lastSeqNum;

    const bool gapExists = lastMsgSeqNumProcessed > _lastSeqNum;
    if(gapExists) {
        EOBI_INFO() << "Heartbeat - Gap detected -"
        << ", Id=" << _id
        << ", lastSeqNum=" << _lastSeqNum
        << ", currentSeqNum=" << lastMsgSeqNumProcessed
        << ", diff=" << (lastMsgSeqNumProcessed - _lastSeqNum)
        << ", inSnapshot=" << _inRecovery
        ;
        if(!_inRecovery) {
            _inRecovery = true;
            _snapshotSeqNum = lastMsgSeqNumProcessed;
        }        
    }
}



//Helper methods
void EOBIProductManger::_AddOrder(const SecurityIdT securityId, 
                        const uint8_t side, 
                        const int64_t price, 
                        const int32_t qty, 
                        const uint64_t orderId) {
    MarketEvent event;
    event.type = MarketEventType::OrderBook;
    event.entry.order_book.action = MarketUpdateAction::New;
    event.indesc = securityId;
    event.entry.order_book.side = GetSide(side);
    event.entry.order_book.price = price;
    event.entry.order_book.quantity = qty;
    event.entry.order_book.orderId = orderId;
    event.entry.order_book.priority = orderId;
    _SendMarketEvent(event);
}

void EOBIProductManger::_DeleteOrder(const SecurityIdT securityId, 
                            const uint8_t side, 
                            const uint64_t orderId) {
    MarketEvent event;
    event.type = MarketEventType::OrderBook;
    event.entry.order_book.action = MarketUpdateAction::Delete;
    event.indesc = securityId;
    event.entry.order_book.side = GetSide(side);
    event.entry.order_book.orderId = orderId;
    _SendMarketEvent(event);
}

void EOBIProductManger::_ClearOrderBook(const SecurityIdT securityId,
                                        const bool inRecovery) {
    EOBI_INFO() << (inRecovery ? "In recovery - " : "") << "Clearing order book for securityId=" << securityId;

    MarketEvent event;
    event.type = ns::MarketEventType::BookReset;
    event.indesc = securityId;
    event.channelId = _channelId;
    event.tsServerRecv = event.tsExchangeSend = ns::GetNowEpoch(std::chrono::nanoseconds());
    _SendMarketEvent(event);
}

void EOBIProductManger::_HandleInstrumentStatus(const SecurityIdT securityId, 
                                        const uint8_t securityStatus,
                                        const uint8_t securityTradingStatus,
                                        const uint8_t fastMarketIndicator,
                                        const uint64_t transactTime,
                                        const bool isSnapshot) const {
    MarketEvent event;
    event.type = MarketEventType::Status;
    event.indesc = securityId;
    event.channelId = _channelId;
    event.tsExchangeSend = transactTime;
    event.tsServerRecv = ns::GetNowEpoch(std::chrono::nanoseconds());

    InstrumentStatus::Value status;
    if(securityStatus == ENUM_SECURITYSTATUS_EXPIRED) {
        event.entry.status.val = InstrumentStatus::Expired;
    } else {
        event.entry.status.val = _GetInstrumentStatus(securityTradingStatus, fastMarketIndicator);
    }

    if(isSnapshot)
        _SendOnSnapshot(event);
    else
        _SendMarketEvent(event);
}

void EOBIProductManger::_HandleProductStatus(const uint8_t subId) {
    MarketEvent event;
    event.type = MarketEventType::Status;
    event.channelId = _channelId;
    event.tsExchangeSend = ns::GetNowEpoch(std::chrono::nanoseconds()); //TODO: get it from the msg
    event.tsServerRecv = ns::GetNowEpoch(std::chrono::nanoseconds());
    event.entry.status.val = _GetInstrumentStatusFromSubID(subId);
  
    for(const SecurityIdT securityId: _securityIds) {
        event.indesc = securityId;
        _SendMarketEvent(event);
        _currentDescs.insert(securityId);
    }
}

void EOBIProductManger::_HandleStatPrice(const SecurityIdT securityId,
                                const StatPriceID::Value priceId,
                                const uint64_t priceValue, 
                                const bool isSnapshot) const {
    MarketEvent event;
    event.type = MarketEventType::StatPrice;
    event.entry.stat_price.action = MarketUpdateAction::New;
    event.indesc = securityId;
    event.entry.stat_price.id = priceId;
    event.entry.stat_price.val = priceValue;
    event.tsExchangeSend = event.tsServerRecv = ns::GetNowEpoch(std::chrono::nanoseconds());

    if(isSnapshot)
        _SendOnSnapshot(event);
    else
        _SendMarketEvent(event);
}

void EOBIProductManger::_HandleTradeVolume(const SecurityIdT securityId,
                                    const uint64_t volume,
                                    const bool isSnapshot) const {
    MarketEvent event;
    event.type = MarketEventType::StatQty;
    event.entry.stat_qty.action = MarketUpdateAction::New;
    event.indesc = securityId;
    event.entry.stat_qty.id = StatQtyID::Volume;
    event.entry.stat_qty.val = volume;
    event.tsExchangeSend = event.tsServerRecv = ns::GetNowEpoch(std::chrono::nanoseconds());

    if(isSnapshot)
        _SendOnSnapshot(event);
    else
        _SendMarketEvent(event);
}

InstrumentStatus::Value EOBIProductManger::_GetInstrumentStatusFromSubID(const uint8_t subId) const {
    switch (subId) {
    case ENUM_TRADINGSESSIONSUBID_PRETRADING: 
        return InstrumentStatus::PreTrading;
    case ENUM_TRADINGSESSIONSUBID_POSTTRADING: 
        return InstrumentStatus::PostTrading;
    default: 
        return InstrumentStatus::Unknown;
    }
}

InstrumentStatus::Value EOBIProductManger::_GetInstrumentStatus(const uint8_t securityTradingStatus, const uint8_t fastMarketIndicator) const {
    switch (securityTradingStatus) {
        case ENUM_SECURITYTRADINGSTATUS_CLOSED:
        case ENUM_SECURITYTRADINGSTATUS_RESTRICTED:
            return InstrumentStatus::Closed;
        case ENUM_SECURITYTRADINGSTATUS_BOOK:
            return InstrumentStatus::PreTrading;
        case ENUM_SECURITYTRADINGSTATUS_CONTINUOUS:
            return fastMarketIndicator == 1 ? InstrumentStatus::FastMarket : InstrumentStatus::Open;
        case ENUM_SECURITYTRADINGSTATUS_OPENINGAUCTION:
            return InstrumentStatus::PreOpen;
        case ENUM_SECURITYTRADINGSTATUS_INTRADAYAUCTION:
        case ENUM_SECURITYTRADINGSTATUS_CIRCUITBREAKERAUCTION:
        case ENUM_SECURITYTRADINGSTATUS_CLOSINGAUCTION:
            return InstrumentStatus::Auction;
        case ENUM_SECURITYTRADINGSTATUS_OPENINGAUCTIONFREEZE:
        case ENUM_SECURITYTRADINGSTATUS_INTRADAYAUCTIONFREEZE:
        case ENUM_SECURITYTRADINGSTATUS_CIRCUITBREAKERAUCTIONFREEZE:
        case ENUM_SECURITYTRADINGSTATUS_CLOSINGAUCTIONFREEZE:
        case ENUM_SECURITYTRADINGSTATUS_TRADINGHALT:
            return InstrumentStatus::Freeze;
        default:
            return InstrumentStatus::Unknown;
    }
}

bool EOBIProductManger::_IsSnapshotLoopValid(const ProductSummaryT* msg) {
    assert(_snapshotSeqNum != NO_VALUE_UINT);
    const MsgSeqNumT lastMsgSeqNumProcessed = msg->LastMsgSeqNumProcessed;
    return lastMsgSeqNumProcessed >= (_snapshotSeqNum - 1);
}

void EOBIProductManger::_OnCompletionIndicatorComplete() {
    EOBI_INFO() << "Sending end for indescs=" << _GetCurrentDescsAsStr();

    MarketEvent event;
    event.channelId = _channelId;
    event.type = MarketEventType::EventEnd;
    for(const auto& desc: _currentDescs) {
        event.indesc = desc;
        _SendMarketEvent(event);
    }
    
    _currentDescs.clear();
}

std::string EOBIProductManger::_GetCurrentDescsAsStr() {
    std::stringstream ss;
    for(const auto& entry: _currentDescs) {
        ss << std::to_string(entry) << ", ";
    }
    return ss.str();
}

bool EOBIProductManger::RequireSnapshot() const {
    return _inRecovery;
}

inline void EOBIProductManger::_SendMarketEvent(MarketEvent& event) const {
    _sendApi->OnIncremental(&event);
}

inline void EOBIProductManger::_SendMarketEventEnd(MarketEvent& event) const {
    auto previousType = event.type;
    event.type = MarketEventType::EventEnd;
    _sendApi->OnIncremental(&event); 
    event.type = previousType;
}

inline void EOBIProductManger::_SendOnSnapshot(MarketEvent& event) const {
    _sendApi->OnSnapshot(&event);
}

void EOBIProductManger::_SendSnapshotEnd() const {
    EOBI_INFO() << "Sending snapshot end for " << _securityIds.size() << " descs, inSnapshot=" << _inRecovery;

    MarketEvent event;
    event.channelId = _channelId;
    event.type = MarketEventType::EventEnd;

    for(const auto& secId: _securityIds) {
        event.indesc = secId;
        _SendOnSnapshot(event);
    }
}
