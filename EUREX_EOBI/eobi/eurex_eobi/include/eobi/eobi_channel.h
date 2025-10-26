#ifndef _EOBI_CHANNEL_H_
#define _EOBI_CHANNEL_H_

#include <algorithm>
#include <ostream>
#include <unordered_set>
#include <unordered_map>

#include "adapter_interface/adapter_api.h"
#include "adapter_interface/instrument_definition.h"
#include "adapter_util/mcast_feed.h"
#include "adapter_util/mcast_receiver.h"
#include "adapter_util/packet_buffer_pool.h"
#include "adapter_util/replay.h"
#include "adapter_util/time.h"
#include "adapter_util/worker_thread.h"
#include "adapter_util/sequence_message_manager.h"
#include "adapter_util/tcp_feed_info.h"
#include "adapter_util/trace_logger.h"
#include "adapter_util/trading_date_store.h"
#include "logger/logger.h"

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

