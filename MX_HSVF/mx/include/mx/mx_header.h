#ifndef _MX_HEADER_
#define _MX_HEADER_

#include "mx_message_definitions.h"

namespace ns {

inline SequenceMeta MXPacketSequenceGetter(const PacketBufferPtr& packetBuffer) {
    char* readPtr = packetBuffer->m_buffer;
    const uint32_t bytesReceived = packetBuffer->m_bytesReceived;

    uint64_t seqNum = 0;
    uint64_t count = 0;
    while(readPtr - packetBuffer->m_buffer < bytesReceived) {
        assert(*readPtr == STX);
        if(count == 0) {
            const MsgHeader* header = reinterpret_cast<const MsgHeader*>(readPtr + sizeof(STX));
            seqNum = header->GetSeqNum();
        }

        ++count; //We have a msg
        auto it = std::find(readPtr, packetBuffer->m_buffer + bytesReceived, ETX);
        assert(*it == ETX && (it != packetBuffer->m_buffer + bytesReceived));
        readPtr = ++it;
    }

    assert(seqNum != 0 && count != 0);
    return SequenceMeta(seqNum, count);
}

inline bool MXPacketResetGetter(const PacketBufferPtr& packetBuffer) {
    char* readPtr = packetBuffer->m_buffer;
    const MsgHeader* header = reinterpret_cast<const MsgHeader*>(readPtr + sizeof(STX));
    return header->GetSeqNum() == 1;
}

}//end namespace

#endif
