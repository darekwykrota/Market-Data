#include "eobi/eobi_channel.h"
#include <fstream>
#include <time.h>
#include <chrono>

namespace ns {

EOBI_Channel::EOBI_Channel(EOBI_Adapter* adapter,
                const ns::ChannelID_t channelId,
                const ChannelTags& tags,
                const std::string& interfaceA,
                const std::string& interfaceB,
                PacketBufferPoolPtr_t packetBufferPool,
                TraceLoggerArray_t loggers)
    :      
   
      _channelId(channelId)
      , _tags(tags)
      , _interfaceA(interfaceA)
      , _interfaceB(interfaceB)
      , _bufferPool(packetBufferPool)
      , _loggers(loggers)
      , _adapter(adapter)
{
}

EOBI_Channel::~EOBI_Channel()
{
}

bool EOBI_Channel::Init(IAdapterSend* sendApi, std::shared_ptr<Config> config, WorkerThreadPtr workerThread, WorkerThreadPtr networkThread) {
    _sendApi = sendApi;
    _workerThread = workerThread;
    _networkThread = networkThread;
    assert(_sendApi && _workerThread && _networkThread);

    using std::placeholders::_1;
    using std::placeholders::_2;
    _incrementalFeed = CreateFeed("IncrementalFeed", std::bind(&EOBI_Channel::OnIncrementalFeedData, this, _1), config);
    if (!_incrementalFeed) {
        EOBI_ERR() << "channelId=" << _channelId << ", incremental feed create failed";
        return false;
    }
    _incrementalFeed->EnableArbitration(EOBIPacketSequenceGetter, ArbitrationType::Packet);
    _incrementalFeed->EnableResetLogic(EOBIPacketResetGetter);
    
    _snapshotFeed = CreateFeed("SnapshotFeed", std::bind(&EOBI_Channel::OnSnapshotFeedData, this, _1), config);
    if (!_snapshotFeed) {
        EOBI_ERR() << "channelId=" << _channelId << ", snapshot feed create failed";
        return false;
    }
    _snapshotFeed->EnableArbitration(EOBIPacketSequenceGetter, ArbitrationType::Packet);
    _snapshotFeed->EnableResetLogic(EOBIPacketResetGetter);
  
    return true;
}

MulticastFeedPtrT EOBI_Channel::CreateFeed(const std::string& feedName, MulticastReceiver::ProcessMessageFunc_t callback, std::shared_ptr<Config> config) {
    auto feedPtr = std::make_unique<MulticastFeed>(_tags, 
                                                    feedName,
                                                    _bufferPool, 
                                                    _networkThread, 
                                                    _workerThread, 
                                                    callback
                                                    );

    const auto& confMap = config->GetMapNode<std::string, sMulticastFeedInfo>(
            "Channels." + _tags.channelName + ".Feeds", "Feed", "name");
    FeedInfos_t feedInfos = confMap->PopulateCompositeNodes();

    const bool successA = feedPtr->InitializeFeed(feedInfos, MulticastFeedIdentifier__A, _interfaceA);
    const bool successB = feedPtr->InitializeFeed(feedInfos, MulticastFeedIdentifier__B, _interfaceB);

    if(successA) {
        EOBI_INFO() << "channelId=" << _channelId << ", successfully initialized " << feedName << "A";
    } else {
        EOBI_INFO() << "channelId=" << _channelId << ", failed to initialize " << feedName << "A";
    }

    if(successB) {
        EOBI_INFO() << "channelId=" << _channelId << ", successfully initialized " << feedName << "B";
    } else {
        EOBI_INFO() << "channelId=" << _channelId << ", failed to initialize " << feedName << "B";
    }

    const bool success = successA | successB;
    return success ? std::move(feedPtr)
                   : MulticastFeedPtrT{};
}

void EOBI_Channel::Start() {
    _incrementalFeed->StartFeed();
}

void EOBI_Channel::Stop() {
    _incrementalFeed->StopFeed();
}

void EOBI_Channel::OnIncrementalFeedData(const MessageMeta& mm) {
    const auto& packetBuffer = mm.pb;
    const char* readPtr = packetBuffer->m_buffer;

    assert(sizeof(PacketHeaderT) < packetBuffer->m_bytesReceived);
    const PacketHeaderT* packetHeader = reinterpret_cast<const PacketHeaderT*>(readPtr);
    const MarketSegmentIdT marketSegmentId = packetHeader->MarketSegmentID;

    auto it = _productManagers.find(marketSegmentId);
    if(std::end(_productManagers) == it) {
        EOBI_WARN() << "Failed to find product manager - marketSegmentId=" << marketSegmentId;
        return;
    }

    it->second.OnIncrementalData(mm);
    if(it->second.RequireSnapshot()) {
        _StartSnapshot(marketSegmentId);
    }
}

void EOBI_Channel::_StartSnapshot(const ID id) {
    if(_snapshotFeed && !_snapshotFeed->IsEnabled() && _snapshotIds.count(id) == 0) {
        EOBI_INFO() << "channelId=" << _channelId << ", Starting snapshot for id=" << id;
        _snapshotIds.insert(id);
        _snapshotFeed->StartFeed();
    }
}

void EOBI_Channel::OnSnapshotFeedData(const MessageMeta& mm) {
    if(!_snapshotFeed->IsEnabled()) {
        return;
    }
 
    const auto& packetBuffer = mm.pb;
    char* readPtr = packetBuffer->m_buffer;

    assert(sizeof(PacketHeaderT) < packetBuffer->m_bytesReceived);
    const PacketHeaderT* packetHeader = reinterpret_cast<const PacketHeaderT*>(readPtr);
    const MarketSegmentIdT marketSegmentId = packetHeader->MarketSegmentID;

    auto it = _snapshotIds.find(marketSegmentId);
    if(std::end(_snapshotIds) != it) {
        auto productManagerIt = _productManagers.find(*it);
        if(std::end(_productManagers) == productManagerIt) {
            EOBI_WARN() << "Failed to find product manager Id=" << *it;
            return;
        }

        productManagerIt->second.OnSnapshotData(mm);
        if(!productManagerIt->second.RequireSnapshot()) {
            _snapshotIds.erase(marketSegmentId);
        }
    }   

    if(_snapshotIds.size() == 0){
        _snapshotFeed->StopFeed();
    }
}

void EOBI_Channel::OnReplayTcpData(const char* buf, size_t len) {
    ProcessReplayData((char*) buf, len);
}

void EOBI_Channel::ProcessReplayData(char* readPtr, size_t len) {
    EOBI_DEBUG() << "channelId=" << _channelId << ", ProcessReplayData";
}

MulticastFeedPtrT EOBI_Channel::GetRealTimeFeed() const {
    return _incrementalFeed;
}

void EOBI_Channel::Post(std::function<void()> fn) {
    _workerThread->Post(fn);
}

}//end namespace
