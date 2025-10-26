#ifndef _EOBI_HEADER_H_
#define _EOBI_HEADER_H_

#include "eobi_messages.h"

namespace ns {

inline uint64_t EOBIPacketSequenceGetter(const PacketBufferPtr& packetBuffer) {
    char* readPtr = packetBuffer->m_buffer;
    const PacketHeaderT* packetHeader = reinterpret_cast<const PacketHeaderT*>(readPtr);
    return packetHeader->ApplSeqNum;
}

inline bool EOBIPacketResetGetter(const PacketBufferPtr& packetBuffer) {
    return EOBIPacketSequenceGetter(packetBuffer) == 1;
}

}//end namespace

#endif
