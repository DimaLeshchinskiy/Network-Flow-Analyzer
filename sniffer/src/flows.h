#include<linux/if_packet.h>
#include<netinet/if_ether.h>
#include<netinet/ip.h>
#include<netinet/udp.h>	
#include<netinet/tcp.h>
#include <arpa/inet.h>

#include <string.h>
#include <stdio.h>
#include <unordered_map>

#include "hash.h"
#include "util.h"

namespace flows{
    int add(unsigned char *buffer);

    class Flow{
        public:
        // ip
        u_int32_t ip_src;
        u_int32_t ip_dst;
        u_int8_t ip_proto;
        u_int16_t port_src;
        u_int16_t port_dst;

        // tcp
        u_int8_t flags;

        //flow
        u_int32_t bytes;
        u_int32_t pkts;
        u_int64_t flow_start_milliseconds;
        u_int64_t flow_end_milliseconds;
    };
}