#ifndef _EOBI_PRODUCT_MANAGER_H_
#define _EOBI_PRODUCT_MANAGER_H_

#include <unordered_set>

#include "eobi_common.h"
#include "eobi_log.h"

using namespace ns;

class EOBIProductManger {
public:
    EOBIProductManger(IAdapterSend* sendApi, const ID id);
    ~EOBIProductManger();
    void OnIncrementalData(const MessageMeta& mm);
    void OnSnapshotData(const MessageMeta& mm);
    bool RequireSnapshot() const;

private:
    void _OnEOBIPacket(const PacketBufferPtr packetBuffer);
    void _OnEOBIMsg(char* msgPtr, const uint16_t templateId, const MsgSeqNumT msgSeqNum);
    void _Process(const ProductSummaryT* msg);
    void _Process(const InstrumentSummaryT* msg);
    void _Process(const SnapshotOrderT* msg);
    void _OnSnapshotComplete();
    void _ProcessEOBIBufferedMsgs();

    void _Process(const OrderAddT* msg);
    void _Process(const OrderDeleteT* msg);
    void _Process(const OrderModifyT* msg);
    void _Process(const OrderModifySamePrioT* msg);
    void _Process(const OrderMassDeleteT* msg);
    void _Process(const PartialOrderExecutionT* msg);
    void _Process(const FullOrderExecutionT* msg);
    void _Process(const ExecutionSummaryT* msg);
    template<typename OrderExecutionMsgT>
    void _ProcessOrderExecution(const OrderExecutionMsgT* msg);
    void _Process(const TradeReportT* msg);
    void _Process(const ProductStateChangeT* msg);
    void _Process(const InstrumentStateChangeT* msg);
    void _Process(const QuoteRequestT* msg);
    void _Process(const CrossRequestT* msg);
    void _Process(const AuctionBBOT* msg);
    void _Process(const AuctionClearingPriceT* msg);
    void _Process(const HeartbeatT* msg);


    //Helper methods
    MsgSeqNumT _GetMsgSeqNum(const PacketBufferPtr packetBuffer) const;
    template <typename MsgT>
    void _ProcessAndAddSecurityId(char* msgPtr);
    template<typename MsgT>
    void _AddSecurityId(const MsgT* msg);
    void _AddOrder(const SecurityIdT securityId,
                    const uint8_t side, 
                    const int64_t price, 
                    const int32_t qty, 
                    const uint64_t orderId);
    void _DeleteOrder(const SecurityIdT securityId, 
                        const uint8_t side, 
                        const uint64_t orderId);
    void _ClearOrderBook(const SecurityIdT securityId, 
                        const bool inRecovery = false);
    void _HandleInstrumentStatus(const SecurityIdT securityId, 
                                    const uint8_t securityStatus,
                                    const uint8_t securityTradingStatus,
                                    const uint8_t fastMarketIndicator,
                                    const uint64_t transactTime,
                                    const bool isSnapshot = false) const;

    void _HandleProductStatus(const uint8_t subId);
    void _HandleStatPrice(const SecurityIdT securityId,
                            const StatPriceID::Value priceId,
                            const uint64_t priceValue,
                            const bool isSnapshot = false) const;
    void _HandleTradeVolume(const SecurityIdT securityId,
                            const uint64_t volume,
                            const bool isSnapshot = false) const;
    InstrumentStatus::Value _GetInstrumentStatusFromSubID(const uint8_t subId) const;
    InstrumentStatus::Value _GetInstrumentStatus(const uint8_t securityTradingStatus, 
                                                                const uint8_t fastMarketIndicator) const;
    bool _IsSnapshotLoopValid(const ProductSummaryT* msg);
    void _OnCompletionIndicatorComplete();
    std::string _GetCurrentDescsAsStr();
    void _SendMarketEvent(MarketEvent& event) const;
    void _SendMarketEventEnd(MarketEvent& event) const;
    void _SendOnSnapshot(MarketEvent& event) const;
    void _SendSnapshotEnd() const;


private:
    //Data Members
    IAdapterSend* _sendApi = nullptr;
    ID _id = 0;
    MsgSeqNumT _lastSeqNum = 0;
    std::unordered_set<SecurityIdT> _securityIds;
    std::set<SecurityIdT> _currentDescs;
    uint64_t _bufferingSkipLogCounter = 0;
    ns::ChannelID_t _channelId;

    bool _inRecovery = false;
    MsgSeqNumT _snapshotSeqNum = NO_VALUE_UINT;
    LastMsgSeqNumT _snapshotLastMsgSeqNum = NO_VALUE_UINT;
    SecurityIdT _snapshotSecurityId = NO_VALUE_SLONG;
    std::deque<MessageMeta> _bufferedEOBIMsgs;
    

};

#endif
