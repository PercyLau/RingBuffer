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

    ring_buffer<packet_info> rb;
    for(int i = 0; i< 19; i++){
       packet_info p(i,"test",128,0,0);
       rb.push_back(p);
       std::cout<<"add elements... size: "<< rb.size() << ", cap: "<< rb.capacity() << std::endl;
    }

    for(int i = 0; i < 5; i++){
        auto p = rb.back();
        std::cout<<"to front pop elements...seq: "<<p.get_seq()<<", rest size: "<<rb.size()<<", cap: "<<rb.capacity()<<std::endl;
        rb.pop_front();
    }

    for(int i = 0; i < 10; i++){
        auto p = rb.front();
        std::cout<<"pop back elements...seq: "<<p.get_seq()<<", rest size: "<<rb.size()<<", cap: "<<rb.capacity()<<std::endl;
        rb.pop_back();
    }

    for(int i = 0; i< 15; i++){
       packet_info p(i+100,"test",128,0,0);
       std::cout<<"add elements seq: "<< p.get_seq() << ", buffer size: "<< rb.size() << ", cap: "<< rb.capacity() << std::endl;
       rb.push_back(p);
    }

    for(int i = 0; i < 5; i++){
        auto p = rb.back();
        std::cout<<"pop front elements...seq: "<<p.get_seq()<<", rest size: "<<rb.size()<<", cap: "<<rb.capacity()<<std::endl;
        rb.pop_front();
    }

    while(rb.capacity()>0){
        auto p = rb.back();
        std::cout<<"to pop elements...seq: "<<p.get_seq()<<", rest size: "<<rb.size()<<", cap: "<<rb.capacity()<<std::endl;
        rb.pop_back();
    }

    return 0;

}