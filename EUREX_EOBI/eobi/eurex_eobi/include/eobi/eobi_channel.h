#ifndef _EOBI_CHANNEL_H_
#define _EOBI_CHANNEL_H_

#include <algorithm>
#include <ostream>
#include <unordered_set>
#include <unordered_map>

#include "eobi_messages.h"
#include "eobi_common.h"
#include "eobi_header.h"
#include "eobi_log.h"
#include "eobi_product_manager.h"
namespace ns {

class EOBI_Adapter;
class EOBI_Channel {
public:
    EOBI_Channel(EOBI_Adapter* adapter,
                const ns::ChannelID_t channelId,
                const ChannelTags& tags,
                const std::string& interfaceA,
                const std::string& interfaceB,
                PacketBufferPoolPtr_t packetBufferPool,
                TraceLoggerArray_t loggers);

    ~EOBI_Channel();
    bool Init(IAdapterSend* sendApi, std::shared_ptr<Config> config, WorkerThreadPtr workerThread, WorkerThreadPtr networkThread);
    void Start();
    void Stop();
    void Post(std::function<void()> fn);

public:
    MulticastFeedPtrT CreateFeed(const std::string& feedName, MulticastReceiver::ProcessMessageFunc_t callback, std::shared_ptr<Config> config);
    void OnIncrementalFeedData(const MessageMeta& mm);
    void OnSnapshotFeedData(const MessageMeta& mm);
    void OnReplayTcpData(const char* buf, size_t len);
    void ProcessReplayData(char* readPtr, size_t len);
    void _StartSnapshot(const ID id);

    //Data Members
    IAdapterSend* _sendApi = nullptr;
    MulticastFeedPtrT _incrementalFeed;
    MulticastFeedPtrT _snapshotFeed;
    WorkerThreadPtr _workerThread;
    WorkerThreadPtr _networkThread;
    std::map<MarketSegmentIdT, EOBIProductManger> _productManagers;
    std::unordered_set<ID> _snapshotIds;
   
    ChannelID_t _channelId;
    ChannelTags _tags;
    const std::string _interfaceA;
    const std::string _interfaceB;
    PacketBufferPoolPtr_t _bufferPool;
    MulticastFeedPtrT GetRealTimeFeed() const;
    TraceLoggerArray_t _loggers;
    EOBI_Adapter* _adapter = nullptr;
    
}; //end class definition

typedef std::shared_ptr<EOBI_Channel> EOBI_ChannelPtrT;

} //end namespace

#endif

