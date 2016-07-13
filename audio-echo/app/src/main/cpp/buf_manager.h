/*
 * Copyright 2015 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef NATIVE_AUDIO_BUF_MANAGER_H
#define NATIVE_AUDIO_BUF_MANAGER_H
#include <sys/types.h>
#include <SLES/OpenSLES.h>
#include <atomic>
#include <cassert>
#include <memory>
#include <limits>

#ifndef CACHE_ALIGN
#define CACHE_ALIGN 64
#endif

/*
 * ProducerConsumerQueue, borrowed from Ian NiLewis
 */
template <typename T>
class ProducerConsumerQueue {
public:
    explicit ProducerConsumerQueue(int size)
            : ProducerConsumerQueue(size, new T[size]) {}


    explicit ProducerConsumerQueue(int size, T* buffer)
            : size_(size), buffer_(buffer) {

        // This is necessary because we depend on twos-complement wraparound
        // to take care of overflow conditions.
        assert(size < std::numeric_limits<int>::max());
    }

    bool push(const T& item) {
        return push([&](T* ptr) -> bool {*ptr = item; return true; });
    }

    // get() is idempotent between calls to commit().
    T*getWriteablePtr() {
        T* result = nullptr;


        bool check  __attribute__((unused));//= false;

        check = push([&](T* head)-> bool {
            result = head;
            return false; // don't increment
        });

        // if there's no space, result should not have been set, and vice versa
        assert(check == (result != nullptr));

        return result;
    }

    bool commitWriteablePtr(T *ptr) {
        bool result = push([&](T* head)-> bool {
            // this writer func does nothing, because we assume that the caller
            // has already written to *ptr after acquiring it from a call to get().
            // So just double-check that ptr is actually at the write head, and
            // return true to indicate that it's safe to advance.

            // if this isn't the same pointer we got from a call to get(), then
            // something has gone terribly wrong. Either there was an intervening
            // call to push() or commit(), or the pointer is spurious.
            assert(ptr == head);
            return true;
        });
        return result;
    }

    // writer() can return false, which indicates that the caller
    // of push() changed its mind while writing (e.g. ran out of bytes)
    template<typename F>
    bool push(const F& writer) {
        bool result = false;
        int readptr = read_.load(std::memory_order_acquire);
        int writeptr = write_.load(std::memory_order_relaxed);

        // note that while readptr and writeptr will eventually
        // wrap around, taking their difference is still valid as
        // long as size_ < MAXINT.
        int space = size_ - (int)(writeptr - readptr);
        if (space >= 1) {
            result = true;

            // writer
            if (writer(buffer_.get() + (writeptr % size_))) {
                ++writeptr;
                write_.store(writeptr, std::memory_order_release);
            }
        }
        return result;
    }
    // front out the queue, but not pop-out
    bool front(T* out_item) {
        return front([&](T* ptr)-> bool {*out_item = *ptr; return true;});
    }

    void pop(void) {
        int readptr = read_.load(std::memory_order_relaxed);
        ++readptr;
        read_.store(readptr, std::memory_order_release);
    }

    template<typename F>
    bool front(const F& reader) {
        bool result = false;

        int writeptr = write_.load(std::memory_order_acquire);
        int readptr = read_.load(std::memory_order_relaxed);

        // As above, wraparound is ok
        int available = (int)(writeptr - readptr);
        if (available >= 1) {
            result = true;
            reader(buffer_.get() + (readptr % size_));
        }

        return result;
    }
    uint32_t size(void) {
        int writeptr = write_.load(std::memory_order_acquire);
        int readptr = read_.load(std::memory_order_relaxed);

        return (uint32_t)(writeptr - readptr);
    }

private:
    int size_;
    std::unique_ptr<T> buffer_;

    // forcing cache line alignment to eliminate false sharing of the
    // frequently-updated read and write pointers. The object is to never
    // let these get into the "shared" state where they'd cause a cache miss
    // for every write.
    alignas(CACHE_ALIGN) std::atomic<int> read_ { 0 };
    alignas(CACHE_ALIGN) std::atomic<int> write_ { 0 };
};

struct sample_buf {
    uint8_t    *buf_;       // audio sample container
    uint32_t    cap_;       // buffer capacity in byte
    uint32_t    size_;      // audio sample size (n buf) in byte
};

using AudioQueue = ProducerConsumerQueue<sample_buf*>;

__inline__ void releaseSampleBufs(sample_buf* bufs, uint32_t& count) {
    if(!bufs || !count) {
        return;
    }
    for(uint32_t i=0; i<count; i++) {
        if(bufs[i].buf_) delete [] bufs[i].buf_;
    }
    delete [] bufs;
}
__inline__ sample_buf *allocateSampleBufs(uint32_t count, uint32_t sizeInByte){
    if (count <= 0 || sizeInByte <= 0) {
        return nullptr;
    }
    sample_buf* bufs = new sample_buf[count];
    assert(bufs);
    memset(bufs, 0, sizeof(sample_buf) * count);

    uint32_t allocSize = (sizeInByte + 3) & ~3;   // padding to 4 bytes aligned
    uint32_t i ;
    for(i =0; i < count; i++) {
        bufs[i].buf_ = new uint8_t [allocSize];
        if(bufs[i].buf_ == nullptr) {
            LOGW("====Requesting %d buffers, allocated %d in %s",count, i,  __FUNCTION__);
            break;
        }
        bufs[i].cap_ = sizeInByte;
        bufs[i].size_ = 0;        //0 data in it
    }
    if(i < 2) {
        releaseSampleBufs(bufs, i);
        bufs = nullptr;
    }
    count = i;
    return bufs;
}

#endif //NATIVE_AUDIO_BUF_MANAGER_H
