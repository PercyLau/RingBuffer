#include<iostream>
#include "ring_buffer.hpp"

int main(){

    class packet_info{
        public:
            packet_info():
                seq(0),
                type("rtp"),
                size(1024),
                send_time(1),
                ack_time(2)
            {
            }

            packet_info(int n, const char* t, int s, int st, int at):
                seq(n),
                type(t),
                size(s),
                send_time(st),
                ack_time(at)
            {
            }

            unsigned long get_seq(){
                return seq;
            }

            unsigned int get_size(){
                return size;
            }

            const char* get_type(){
                return type;
            }

        private:
            unsigned long seq;
            const char* type;
            unsigned int size;
            unsigned long long send_time;
            unsigned long long ack_time;
    };

    RingBuffer<packet_info> rb;
    for(int i = 0; i< 19; i++){
       packet_info p(i,"test",128,0,0);
       rb.PushFront(p);
       std::cout<<"add elements... size: "<< rb.Size() << "cap: "<< rb.Capacity() << std::endl;
    }

    for(int i = 0; i < 5; i++){
        ErrNo s;
        auto p = rb.Back(s);
        std::cout<<"to pop elements...seq: "<<p.get_seq()<<" rest size: "<<rb.Size()<<" cap: "<<rb.Capacity()<<std::endl;
        rb.PopBack();
    }

    for(int i = 0; i < 10; i++){
        ErrNo s;
        auto p = rb.Back(s);
        std::cout<<"pop elements...seq: "<<p.get_seq()<<"rest size: "<<rb.Size()<<"cap: "<<rb.Capacity()<<std::endl;
        rb.PopBack();
    }

    for(int i = 0; i< 15; i++){
       packet_info p(i+100,"test",128,0,0);
       std::cout<<"add elements seq: "<< p.get_seq() << "buffer size: "<< rb.Size() << "cap: "<< rb.Capacity() << std::endl;
       rb.PushFront(p);
    }

    for(int i = 0; i < 5; i++){
        ErrNo s;
        auto p = rb.Back(s);
        std::cout<<"pop elements...seq: "<<p.get_seq()<<"rest size: "<<rb.Size()<<"cap: "<<rb.Capacity()<<std::endl;
        rb.PopBack();
    }

    while(rb.Capacity()>0){
        ErrNo s;
        auto p = rb.Back(s);
        std::cout<<"to pop elements...seq: "<<p.get_seq()<<"rest size: "<<rb.Size()<<"cap: "<<rb.Capacity()<<std::endl;
        rb.PopBack();
    }

    return 0;

}