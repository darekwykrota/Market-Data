#ifndef _MX_RETRANSMISSION_H_
#define _MX_RETRANSMISSION_H_

#include <atomic>
#include <mutex>
#include <string>
#include <unordered_set>
#include "mx_common.h"


#define REC_ID() "MXRecovery(" << _tags.channelName << "): "
#define REC_DEBUG() TTLOG(DEBUG, 0) << REC_ID()
#define REC_INFO() TTLOG(INFO, 0) << REC_ID()
#define REC_WARN() TTLOG(WARNING, 0) << REC_ID()
#define REC_ERR() TTLOG(ERROR, 0) << REC_ID()

namespace ns {

template <typename ProcessorT>
class MXRecoveryHandler {
public:
    MXRecoveryHandler();
    bool Initialize(std::shared_ptr<Config> config, 
                    const ChannelTags& tags,
                    const PacketBufferPoolPtr_t bufferPool,
                    WorkerThreadPtr& networkThread,
                    WorkerThreadPtr& workerThread,
                    const std::string& interfaceA,
                    const std::string& interfaceB,
                    const std::string& username,
                    const std::string& password,
                    const std::string& line,
                    const int recoveryTimeout,
                    const int recoveryPageSize,
                    ProcessorT* processor);
    bool RequestGap(const uint64_t fromSequence, const uint64_t toSequence);

private:
    void _Connect();
    void _Disconnect();
    void _OnTCPStatus(TCPClient::TCPStatus status, const std::string& connectionId);
    void _OnTCPData(PacketBufferPtr ptr);
    void _ProcessData();
    void _ProcessMessage(char* data);
    void _SendLogin();
    void _SendLogout();
    void _SendRetransmissionRequest();
    void _HandleTCPError(char* msg);
    void _KickOffAbandonRecoveryTimer();
    void _OnAbandonRecoveryTimer(const boost::system::error_code & error);
    void _CancelAbandonRecoveryTimer();
    bool _IsRetransmissionComplete();


    template<typename MessageT>
    void _SendMessage(const MessageT& message);

    //Data members
    WorkerThreadPtr _workerThread;
    WorkerThreadPtr _networkThread;
    std::string _username;
    std::string _password;
    std::string _line;

    typedef std::shared_ptr<ns::TCPConnectionMgr> TCPConnectionMgrPtr;
    TCPConnectionMgrPtr _tcpConnectionMgr;
    PacketBufferPoolPtr_t _bufferPool;
    std::vector<char> _buffer;
    ChannelTags _tags;
    ProcessorT* _processor;
    uint64_t _fromSequence;
    uint64_t _toSequence;
    int _recoveryTimeout;
    int _recoveryPageSize;
    std::unique_ptr<boost::asio::deadline_timer> _abandonRecoveryTimer;
};


template <typename ProcessorT>
MXRecoveryHandler<ProcessorT>::MXRecoveryHandler()
{
}

template <typename ProcessorT>
bool MXRecoveryHandler<ProcessorT>::Initialize(std::shared_ptr<Config> configInstance,
                                                const ChannelTags& tags,
                                                const PacketBufferPoolPtr_t bufferPool,
                                                WorkerThreadPtr& networkThread, 
                                                WorkerThreadPtr& workerThread,
                                                const std::string& interfaceA, 
                                                const std::string& interfaceB,
                                                const std::string& username,
                                                const std::string& password,
                                                const std::string& line,
                                                const int recoveryTimeout,
                                                const int recoveryPageSize,
                                                ProcessorT* processor) {
    _tags = tags;
    _processor = processor;
    _workerThread = workerThread;
    _networkThread = networkThread;
    _bufferPool = bufferPool;
    assert(_workerThread && _networkThread && _bufferPool);
    _username = username;
    _password = password;
    _line = line;
    _recoveryTimeout = recoveryTimeout;
    _recoveryPageSize = recoveryPageSize;

    TCPClient::processMessageFunc_t receiveCallback = std::bind(&MXRecoveryHandler::_OnTCPData, this, std::placeholders::_1);
    TCPClient::responseFunc_t statusCallback = std::bind(&MXRecoveryHandler::_OnTCPStatus, this, std::placeholders::_1, std::placeholders::_2);

    const std::string feedName = "RecoveryFeed";
    const uint16_t timeout = 30;
    _tcpConnectionMgr = std::make_shared<TCPConnectionMgr>(_tags, 
                                                            feedName, 
                                                            _bufferPool,
                                                            _networkThread,
                                                            _workerThread, 
                                                            receiveCallback,
                                                            statusCallback,
                                                            timeout);

    _tcpConnectionMgr->InitializeConnection("", configInstance);
    if (!_tcpConnectionMgr->HaveClients()) {
        REC_ERR() << "No clients";
        return false;
    }

    const uint16_t retries = 3;
    const uint16_t waitIntervalInSeconds = 2;
    _tcpConnectionMgr->SetRetry("", retries, waitIntervalInSeconds);

    _abandonRecoveryTimer = std::make_unique<boost::asio::deadline_timer>(_workerThread->GetIOService());

    return true;
}

template <typename ProcessorT>
bool MXRecoveryHandler<ProcessorT>::RequestGap(const uint64_t fromSequence, const uint64_t toSequence) {
    assert(fromSequence <= toSequence);
    _CancelAbandonRecoveryTimer();

    //Stop any previous recovery
    _Disconnect();

    REC_INFO() << "Requesting gap from=" << fromSequence << " to=" << toSequence;
    if(fromSequence > toSequence) {
        REC_ERR() << "Error requesting gap - from=" << fromSequence << " is greater than to=" << toSequence;
        return false;
    }

    _fromSequence = fromSequence - 1;
    _toSequence = toSequence;

    _KickOffAbandonRecoveryTimer();
    _Connect();
    return true;
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_Connect() {
    REC_INFO() << "Connecting to MX retransmission";
    _tcpConnectionMgr->Connect();
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_Disconnect() {
    REC_INFO() << "Disconnecting from MX retransmission";
    _tcpConnectionMgr->Disconnect();
}

template <typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_OnTCPStatus(TCPClient::TCPStatus status, const std::string& connectionId) {
    switch (status) {
        case TCPClient::TCPStatus::CONNECTED: {
            _buffer.clear();
            REC_INFO() << "MX TCP Connected";
            _SendLogin();
        } 
        break;
        case TCPClient::TCPStatus::DISCONNECTED: {
            REC_WARN() << "MX TCP Disconnected";
        } 
        break;
        case TCPClient::TCPStatus::CONNECT_ATTEMPT_FAIL: {
            REC_WARN() << "MX TCP Connect Attempt Fail";
        } 
        break;
        case TCPClient::TCPStatus::WRITE_SUCCESS: {
            REC_INFO() << "MX TCP Write Success";
        } 
        break;
        case TCPClient::TCPStatus::WRITE_FAIL: {
            REC_WARN() << "MX TCP Write Fail";
        } 
        break;
        case TCPClient::TCPStatus::UNKNOWN: {
            REC_WARN() << "MX TCP Unknown";
        } 
        break;
        default: {
            REC_WARN() << "MX TCP Status. Unknown";
        }
    }
}

template <typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_OnTCPData(PacketBufferPtr packetBuffer) {
    char* readPtr = packetBuffer->m_buffer;
    const uint32_t bytesReceived = packetBuffer->m_bytesReceived;
    if(bytesReceived > 0) {
        _buffer.insert(std::end(_buffer), readPtr, readPtr + bytesReceived);
        _ProcessData();
    }
}

template <typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_ProcessData() {
    if(_buffer.size() < sizeof(MsgHeader)) {
        return;
    }

    char* readPtr = _buffer.data();
    assert(*readPtr == STX);

    auto it = std::find(std::begin(_buffer), std::end(_buffer), ETX);
    //Make sure we have a complete msg
    const bool isCompleteMsg = it != std::end(_buffer);

    if(!isCompleteMsg) {
        REC_DEBUG() << "Not enough data for a complete msg. Got=" << _buffer.size();
        return;
    }

    _ProcessMessage(readPtr + sizeof(STX));

    const size_t bytesUsed = std::distance(std::begin(_buffer), ++it);
    const size_t bytesLeft = _buffer.size() - bytesUsed;
    std::memmove(std::data(_buffer), std::data(_buffer) + bytesUsed, bytesLeft);
    _buffer.resize(bytesLeft);

    if (!_buffer.empty()) {
        char* ptr = _buffer.data();
        assert(*ptr == STX);

        //Can we process yet another msg?
        _ProcessData();
    }
}

template <typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_ProcessMessage(char* msg) {
    const MsgHeader* msgHeader = reinterpret_cast<const MsgHeader*>(msg);
    const std::string msgType = msgHeader->GetMsgType();
    const uint64_t seqNum = msgHeader->GetSeqNum();

    switch(consthash(msgType.c_str())) {
    case consthash("KI"): {
        REC_INFO() << "Successfully logged in";
        _SendRetransmissionRequest(); 
    }
    break;
    case consthash("RB"):
        REC_INFO() << "Retransmission of msgs starting";
        _CancelAbandonRecoveryTimer();
    break;
    case consthash("RE"): {
        if(_IsRetransmissionComplete()) {
            _processor->OnRetransmissionComplete();
            _SendLogout();
        } else {
            _SendRetransmissionRequest();
        }
    }
    break;
    case consthash("KO"): {
        REC_INFO() << "Logout acknowledged";
        _Disconnect();
    }
    break;
    case consthash("ER"):
        _HandleTCPError(msg);
    break;
    default: {
        _fromSequence = seqNum;
        _processor->OnRetransmissionMsg(msg);
    }
    }
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_SendLogin() {
    REC_INFO() << "Sending login";

    const std::string paddedUsername = std::string(16 - std::min(16, static_cast<int>(_username.size())), ' ') + _username;
    const std::string paddedPassword = std::string(16 - std::min(16, static_cast<int>(_password.size())), ' ') + _password;

    Login login;
    memcpy(login.header.seqNum, "0000000001", sizeof(login.header.seqNum));
    memcpy(login.header.msgType, "LI", sizeof(login.header.msgType));
    memcpy(login.username, paddedUsername.c_str(), sizeof(login.username));
    memcpy(login.password, paddedPassword.c_str(), sizeof(login.password));
    memcpy(login.timestamp, "000001", sizeof(login.timestamp));
    memcpy(login.protocol, "D7", sizeof(login.protocol));
    login.STX = STX;
    login.ETX = ETX;

    _SendMessage(login);
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_SendLogout() {
    REC_INFO() << "Sending logout";

    Logout logout;
    memcpy(logout.header.seqNum, "0000000001", sizeof(logout.header.seqNum));
    memcpy(logout.header.msgType, "LO", sizeof(logout.header.msgType));
    logout.STX = STX;
    logout.ETX = ETX;

    _SendMessage(logout);
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_SendRetransmissionRequest() {
    //We need to take into account page size when making requests
    const uint64_t validFromSequence = _fromSequence + 1;
    const uint64_t validToSequence = std::min((_fromSequence + _recoveryPageSize) / _recoveryPageSize * _recoveryPageSize, _toSequence);

    assert(validFromSequence <= validToSequence);

    const std::string fromSeqPadded = std::string(10 - std::min(10, static_cast<int>(std::to_string(validFromSequence).size())), '0') + std::to_string(validFromSequence);
    const std::string toSeqPadded = std::string(10 - std::min(10, static_cast<int>(std::to_string(validToSequence).size())), '0') + std::to_string(validToSequence);

    REC_INFO() << "Sending retransmission request - From: " << fromSeqPadded << ", To: " << toSeqPadded;

    RetransmissionRequest request;
    memcpy(request.header.seqNum, "0000000001", sizeof(request.header.seqNum));
    memcpy(request.header.msgType, "RT", sizeof(request.header.msgType));
    memcpy(request.line, _line.c_str(), sizeof(request.line));
    memcpy(request.startMsgNumber, fromSeqPadded.c_str(), sizeof(request.startMsgNumber));
    memcpy(request.endMsgNumber, toSeqPadded.c_str(), sizeof(request.endMsgNumber));
    request.STX = STX;
    request.ETX = ETX;

    _SendMessage(request);
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_HandleTCPError(char* msg) {
    const ErrorMessage* errMsg = reinterpret_cast<const ErrorMessage*>(msg);
    REC_ERR() << "TCP Error -"
    << " errorCode=" << errMsg->GetErrorCode() 
    << ", errorMsg=" << errMsg->GetErrorMsg();
}

template<typename ProcessorT>
template<typename MessageT>
void MXRecoveryHandler<ProcessorT>::_SendMessage(const MessageT& message) {
    REC_DEBUG() << "Sending size of msg=" << sizeof(message);
    PacketBufferPtr pBufferPtr = _bufferPool->GetFreeBuffer();
    pBufferPtr->m_bytesReceived = sizeof(message);
    pBufferPtr->m_receivedFromNetworkTimestamp_ns = 0;
    memcpy(pBufferPtr->m_buffer, &message, sizeof(message));
    _tcpConnectionMgr->Send(pBufferPtr);
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_KickOffAbandonRecoveryTimer() {
    _abandonRecoveryTimer->expires_from_now(boost::posix_time::seconds(_recoveryTimeout));
    _abandonRecoveryTimer->async_wait([=](const boost::system::error_code& ec) { _OnAbandonRecoveryTimer(ec); });
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_OnAbandonRecoveryTimer(const boost::system::error_code & error) {
    if(error) {                                                                            
        if(error != boost::asio::error::operation_aborted)
            REC_INFO() << "Abandon Recovery timer failed to execute with error= " << error.message();
        
        return;
    }

    REC_INFO() << "OnTimer - OnAbandonRecoveryTimer kicked off. Disconnecting";
    _Disconnect();
    _processor->OnRetransmissionFailed();
}

template<typename ProcessorT>
void MXRecoveryHandler<ProcessorT>::_CancelAbandonRecoveryTimer() {
    REC_INFO() << "Cancelling Abandon Recovery timer";
    boost::system::error_code ec;
    _abandonRecoveryTimer->cancel(ec);
    if (ec)
        REC_INFO() << "An error occured while stopping the Abandon Recovery timer"; 
}

template <typename ProcessorT>
bool MXRecoveryHandler<ProcessorT>::_IsRetransmissionComplete() {
    return _fromSequence == _toSequence;
}

}//end namespace

#endif
