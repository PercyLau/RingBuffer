#ifndef QOS_RINGBUFFER_H
#define QOS_RINGBUFFER_H

#include <algorithm>
#include <limits>
#include "optional-lite/include/nonstd/optional.hpp"

using ErrNo = signed long;
template<class T> class RingBuffer {

    class Iterator{
        public:
            explicit Iterator(const size_t c = 0, const size_t i = 0){
                counter_ = c;
                index_ = i;
            }
            size_t counter_;
            size_t index_;

            bool operator > (const Iterator& right_hand) const{
                if(this->counter_ > right_hand.counter_ && this->counter_ - right_hand.counter_ <= MAX_BUFFER_SIZE){
                    return true;
                }

                if(this->counter_ < right_hand.counter_ && right_hand.counter_ - this->counter_ > MAX_BUFFER_SIZE){
                    return true;
                }

                return false;
            }

            bool operator >= (const Iterator& right_hand) const{
                if(this->counter_ >= right_hand.counter_ && this->counter_ - right_hand.counter_ <= MAX_BUFFER_SIZE){
                    return true;
                }

                if(this->counter_ < right_hand.counter_ && right_hand.counter_ - this->counter_ > MAX_BUFFER_SIZE){
                    return true;
                }

                return false;
            }  

            bool operator < (const Iterator& right_hand) const{
                if(this->counter_ < right_hand.counter_ && right_hand.counter_ - this->counter_ <= MAX_BUFFER_SIZE){
                    return true;
                }

                if(this->counter_ > right_hand.counter_ && this->counter_ - right_hand.counter_ > MAX_BUFFER_SIZE){
                    return true;
                }

                return false;
            }

            bool operator <= (const Iterator& right_hand) const{
                if(this->counter_ <= right_hand.counter_ && right_hand.counter_ - this->counter_ <= MAX_BUFFER_SIZE){
                    return true;
                }

                if(this->counter_ > right_hand.counter_ && this->counter_ - right_hand.counter_ > MAX_BUFFER_SIZE){
                    return true;
                }

                return false;
            } 
    };

    public:

        explicit RingBuffer(const size_t s = 8)
            : size_(s)
            , front_(0,0)
            , back_(0,0) {
                buffer_ = new T[size_];
            }

        ~ RingBuffer(){
            delete[] buffer_;
        }

        static const size_t MAX_BUFFER_SIZE;
        static const size_t MIN_BUFFER_SIZE;
        static const ErrNo SUCCESS;

        ErrNo Reset(){
            size_ = 128;
            front_ = Iterator(0,0);
            back_ = Iterator(0,0);
            buffer_ = new(std::nothrow) T[size_];
            if(buffer_ == nullptr){
                return -1;
            }
            return SUCCESS;
        }

        ErrNo Resize(const size_t target_size){
            if(target_size < Capacity()){
                return -1;
            }

            if(target_size > MAX_BUFFER_SIZE || target_size < MIN_BUFFER_SIZE){
                return -2;
            }

            T* new_buffer = new(std::nothrow) T[target_size];

            if(new_buffer == nullptr){
                return -3;
            }

            for(size_t i = 0; i < Capacity(); i++){
                size_t old_index = (back_.counter_ + i) % size_;
                new_buffer[i] = buffer_[old_index];
            }
            
            size_ = target_size;
            front_.index_ = Capacity();
            back_.index_ = 0;
            
            delete[] buffer_;

            buffer_ = new_buffer;

            return SUCCESS;
        }

        ErrNo PushFront(const T in_chunk){
            if(Budget() == 0){
                if(Resize(2 * size_) != SUCCESS){
                    return -1;
                }
            }

            buffer_[front_.index_] = in_chunk;
            front_.counter_ ++;
            front_.index_ = (++front_.index_) % size_;

            return SUCCESS;
        }

        ErrNo PopBack(){
            if(Size() == 0){
                return -1;
            }

            back_.counter_++;
            back_.index_ = (++back_.index_) % size_;

            return SUCCESS;
        }

        const T& Front(ErrNo& status){
            status = SUCCESS;
            if(buffer_ == nullptr || Capacity() == 0 || front_ < back_ || front_.index_ >= size_){
                status = -1;
            }
            return buffer_[front_.index_ - 1];
        }

        const T& Back(ErrNo& status){
            if(buffer_ == nullptr || Capacity() == 0 || back_.index_ >= size_){
                status = -1;
            }
            return buffer_[back_.index_];
        }

        const T& Get(size_t counter, ErrNo& status) {
            status = SUCCESS;
            if(counter >= front_.counter_){
                status = -1;
            }

            if(front_.counter_ % size_ > back_.counter_ % size_){
                size_t index = counter % size_;
                if(index >= back_.index_ && index < front_.index_){
                    return buffer_[index];
                }else{
                    status = -2;
                }
                
            }else{
                size_t index = counter % size_;
                if((index >= back_.index_ && index < size_) || (index < front_.index_)){
                    return buffer_[index];
                }else{
                    status = -2;
                }
            }
            return buffer_[0];
        }


        size_t Size() {
            return size_;
        }

        size_t Capacity(){
            size_t capacity = front_.counter_ - back_.counter_;
            if(capacity > MAX_BUFFER_SIZE) {
                Reset();
                return 0;
            }
            return capacity;
        }

    protected:
        size_t Budget(){
            size_t free_num = size_ - Capacity();
            return free_num;
        }

        size_t size_; // buffer size
        Iterator front_;
        Iterator back_;
        T* buffer_;

};

template <typename T> const size_t RingBuffer<T>::MAX_BUFFER_SIZE =  std::numeric_limits<size_t>::max()/2 - 1;
template <typename T> const size_t RingBuffer<T>::MIN_BUFFER_SIZE = 8;
template <typename T> const ErrNo RingBuffer<T>::SUCCESS = 0;

#endif