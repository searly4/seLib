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

#include <stdint.h>
#include <strings.h>


namespace seLib {
namespace Serializable {

// Class for accessing byte for byte equivalent types in serialized data buffers.
// This class uses byte-wise copy functions instead of type casts to avoid
// potential alignement issues, but may create temporary copies in the process.
template <typename T>
struct SerializedType final {
    inline T operator*() const {
        T value;
        memcpy(&value, this, sizeof(T));
        return value;
    }

    inline SerializedType<T>& Set(const T& value) {
        memcpy(this, &value, sizeof(T));
        return *this;
    }
};

// todo: not ready yet
template <typename T>
struct SerializedLittleEndian final {
    inline const T& operator*() const {
        T value = 0;
        for (int i = 0; i < sizeof(T); i++) {
            int byteposition = i;
            value |= (T)(((uint8_t*)this)[byteposition]) << (8 * i);
        }
        return *(T*)this;
    }
    inline SerializedLittleEndian<T>& Set(const T& value) {
        for (int i = 0; i < sizeof(T); i++) {
            int byteposition = i;
            ((uint8_t*)this)[byteposition] = (uint8_t)((value >> (8 * i)) & 0xFFu);
        }
        return *this;
    }
};

// todo: not ready yet
template <typename T>
struct SerializedBigEndian final {
    inline const T& operator*() const {
        T value = 0;
        for (int i = 0; i < sizeof(T); i++) {
            int byteposition = sizeof(T) - i;
            value |= (T)(((uint8_t*)this)[byteposition]) << (8 * i);
        }
        return *(T*)this;
    }
    inline SerializedLittleEndian<T>& Set(const T& value) {
        for (int i = 0; i < sizeof(T); i++) {
            int byteposition = sizeof(T) - i;
            ((uint8_t*)this)[byteposition] = (uint8_t)((value >> (8 * i)) & 0xFFu);
        }
        return *this;
    }
};

}
}
