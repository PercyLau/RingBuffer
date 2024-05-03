#ifndef QOS_RINGBUFFER_H
#define QOS_RINGBUFFER_H
#include <limits>

template<typename T> class ring_buffer {

using value_type = T;
using pointer = value_type *;
using reference = value_type &;
using self_pointer = ring_buffer const*;
using errno_t = long;
using const_pointer = value_type const *;
using const_reference = value_type const &;

public:
    class iterator{
        // iterator traits
        using difference_type = std::ptrdiff_t;
        using iterator_category = std::random_access_iterator_tag;
        using self = iterator;
        public:
            explicit iterator():
                ptr_(const_cast<pointer>(nullptr)),
                container_bind_(const_cast<ring_buffer::self_pointer>(nullptr)){
                return;
            }

            iterator(const pointer ptr, ring_buffer::self_pointer bind = const_cast<ring_buffer::self_pointer>(nullptr)):
                ptr_(ptr),
                container_bind_(bind){    
                return;
            }

            iterator(const iterator& right_hand) {
                this->ptr = right_hand.ptr_;
                this->container_bind_ = right_hand.container_bind_;
                return;
            }

            void operator=(const iterator right_hand) {
                this->ptr = right_hand.ptr_;
                this->container_bind_ = right_hand.container_bind_;
                return;
            }

            bool operator==(const iterator right_hand) const{
                return this->ptr_ == right_hand.ptr_ && this->container_bind_ == right_hand.container_bind_;
            }

            bool operator != (const iterator right_hand) const{
                return !(*(this)==right_hand);
            }

            self& operator++(){
                if(!container_bind_ || !container_bind_->exist(this->ptr_)){
                    return (*this);
                }
                return increase_();
            }

            self operator++(int){
                self copy(*this);
                ++(*this);
                return copy;
            }

            self& operator--(){
                if(!container_bind_ || !container_bind_->exist(this->ptr_)){
                    return (*this);
                }
                return decrease_();
            }

            self operator--(int){
                self copy(*this);
                ++(*this);
                return copy;
            }
        
        private:
            self& increase_(){
                if(container_bind_ && (*this) != this->container_bind_.end()){
                     ++(this->ptr_);
                }
                return (*this);
            }

            self& decrease_(){
                if(container_bind_ && (*this) == this->container_bind_.begin()){
                    --(this->ptr);
                }
                return (*this);
            }

        protected:
            ring_buffer::self_pointer  container_bind_;
            const_pointer ptr_;
    };

    protected:
        class local_ptr{
            public:
                pointer ptr_;
                size_t index_;
                size_t counter_;

                local_ptr():
                    ptr_(nullptr),
                    index_(0),
                    counter_(0){
                }

                local_ptr(pointer ptr, size_t index, size_t counter):
                    ptr_(ptr),
                    index_(index),
                    counter_(counter){
                }

                local_ptr(const local_ptr& rh){
                    ptr_ = rh.ptr_;
                    index_ = rh.index_;
                    counter_ = rh.counter_;
                }

                void operator=(const local_ptr& rh){
                    ptr_ = rh.ptr_;
                    index_ = rh.index_;
                    counter_ = rh.counter_;
                }

                bool operator==(const local_ptr& rh){
                    return index_ == rh.index_ && counter_ == rh.counter_;
                }

                bool operator>(const local_ptr& rh){
                    return counter_ > rh.counter_;
                }

                bool operator>=(const local_ptr& rh){
                    if(*this == rh){
                        return true;
                    }
                    return *this > rh;
                }

                bool operator<(const local_ptr& rh){
                    return counter_ < rh.counter_;
                }

                bool operator<=(const local_ptr& rh){
                    if(*this == rh){
                        return true;
                    }
                    return *this < rh;
                }
        };

        size_t budget(){
            size_t free_num = size_ - capacity();
            return free_num;
        }

        size_t size_; // buffer size
        local_ptr latest_;
        local_ptr last_;
        pointer buffer_;

    public:
        explicit ring_buffer(const size_t s = 8){
                size_ = s;
                buffer_ = new value_type[size_+1];
                latest_ = {buffer_,0,0};
                last_ = {buffer_,0,0};
        }

        ~ ring_buffer(){
            delete[] buffer_;
        }

        static const size_t MAX_BUFFER_SIZE;
        static const size_t MIN_BUFFER_SIZE;
        static const errno_t SUCCESS;

        size_t size() {
            return size_;
        }

        size_t capacity(){
            size_t capacity = latest_.counter_ - last_.counter_;
            if(capacity > MAX_BUFFER_SIZE) {
                throw("ring buffer stores too many elements");
            }
            return capacity;
        }

        void reset(){
            size_ = 128;
            buffer_ = new value_type[size_+1];
            if(buffer_ == nullptr){
                throw("mallocing buffer fails");
            }
            last_ = local_ptr(buffer_,0,last_.counter_);
            latest_ = local_ptr(buffer_,0,latest_.counter_);
            return;
        }

        bool exist(const_pointer in) const{
            // for(size_t i = 0; i < capacity(); i++){
            //     if(in == last_.ptr_ + i){
            //         return true;
            //     }
            // }
            // return false;
            if(empty()){
                return false;
            }

            if(latest_.index_ > last_.index_){
                // =: valid element
                // ring buffer_>> ------last======>latest------[buffer_ + size_] >>>buffer ptr+size_t
                return in < latest_.ptr_ && in >= last_.ptr_;
            }

            if(latest_.index_ < last_.index_){
                // =: valid element
                // ring buffer_>> ======>latest-------last======>[buffer_ + size_] >>>buffer ptr+size_t
                return (in >= buffer_ && in < latest_.ptr_ )||(in >=last_.ptr_ && in < buffer_ + size_);
            }
        }


        const iterator begin(){
            return iterator(last_.ptr_,this);
        }


        const iterator end(){
            return iterator(latest_.ptr_,this);
        }

        bool empty(){
            return latest_ == last_;
        }

        errno_t resize(const size_t target_size){
            if(target_size > MAX_BUFFER_SIZE){
                throw("no available memory when increase ring buffer");
            }

            if(target_size < MIN_BUFFER_SIZE){
                throw("ring buffer size becomes smaller than minimum value");
            }

            if(target_size < capacity()){
                throw("ring buffer resize() fails when target size is smaller than the number of stored elements");
            }

            T* new_buffer = new T[target_size];

            for(size_t i = 0; i < capacity(); i++){
                size_t old_index = (last_.counter_ + i) % size_;
                new_buffer[i] = buffer_[old_index];
            }
            
            size_ = target_size;
            latest_.index_ = capacity();
            last_.index_ = 0;
            
            delete[] buffer_;

            buffer_ = new_buffer;

            return SUCCESS;
        }

        errno_t pop_front(){
            if(size() == 0){
                throw("empty inner buffer when insert into ring buffer");
            }
            if(last_ < latest_){
                last_.counter_++;
                last_.index_ = (last_.counter_) % size_;
                last_.ptr_ = buffer_ + last_.index_;
                return SUCCESS;
            }else{
                return -1;
            }
        }

        errno_t pop_back(){
            if(size() == 0){
                throw("empty inner buffer when insert into ring buffer");
            }

            if(last_ < latest_){
                latest_.counter_--;
                latest_.index_ = latest_.counter_ % size_;
                latest_.ptr_ = buffer_ + latest_.index_;
                return SUCCESS;
            }
            return -1;
        }

        reference front(errno_t& status){   
            if(empty()){
                return buffer_[-1];
            }
            status = SUCCESS;
            return buffer_[last_.index_];
        }

        const_reference front(){   
            if(empty()){
                return buffer_[-1];
            }
            return buffer_[last_.index_];
        }

        void push_back(const T& in){
            if(budget()==0){
                if(resize(2*size_) != SUCCESS){
                    return;
                };
            }
            buffer_[latest_.index_] = in;
            latest_.counter_++;
            latest_.index_ = latest_.counter_ % size_;
            latest_.ptr_ = buffer_ + latest_.index_;
            return ;
        }

        reference back(errno_t& status){
            if(empty()){
                return buffer_[-1];
            }
            status = SUCCESS;
            return buffer_[latest_.index_ - 1];
        }

        const_reference back(){
            if(empty()){
                return buffer_[-1];
            }
            return buffer_[latest_.index_ - 1];
        }

        const T& at(const size_t index) const{
            if(index > capacity()){
                throw("out of range");
            }

            size_t _pos = (last_.index_ + index) % size_;
            return buffer_[_pos];
        }

        const T& visit(size_t counter) const{
            if(counter >= latest_.counter_){
                throw("visit out of range");
            }

            size_t _pos = counter % size_;

            if(!(latest_.index_ > last_.index_)&&(_pos >= last_.index_ && _pos < latest_.index_)){
                    throw("visit out of range");
            }else{
                if (!((_pos >= last_.index_ ) || (_pos < latest_.index_))){
                    throw("visit out of range");
                }
            }
            return buffer_[_pos];
        }
};

template<typename T> const size_t ring_buffer<T>::MAX_BUFFER_SIZE = std::numeric_limits<size_t>::max()/2 - 1;
template<typename T> const size_t ring_buffer<T>::MIN_BUFFER_SIZE = 8;
template<typename T> const ring_buffer<T>::errno_t ring_buffer<T>::SUCCESS = 0;
#endif