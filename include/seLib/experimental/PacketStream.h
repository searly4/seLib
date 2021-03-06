#pragma once
/*
   Copyright 2018 by Scott Early

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.
*/

#include <stdlib.h>
#include <stdint.h>

#ifdef __cplusplus
#ifdef C_COMPAT
extern "C" {
#else
namespace seLib {
#endif
#endif

// Receiver function for PacketStream packets.
typedef void(*PacketStreamCallback)(const PacketStreamDescriptor& descriptor, const uint8_t* packetdata, void* reference);

struct PacketStreamDescriptor;


struct PacketStreamReceiver {
    PacketStreamCallback callback;
    void* reference;
};

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace seLib {

struct PacketStreamDescriptor {
    int PacketType;
    size_t PacketSize;
    int ReceiverCount;
    PacketStreamReceiver const * Receivers;

    void Notify(const void* buffer) const {
        for (int i = 0; i < ReceiverCount; i++) {
            auto t = Receivers[i];
            t.callback(*this, (const uint8_t*)buffer, t.reference);
        }
    }
};

}

#endif
