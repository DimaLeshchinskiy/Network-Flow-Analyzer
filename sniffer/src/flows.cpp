#include "flows.h"

int tcp_header(unsigned char *buffer){
    struct tcphdr *tcp = (struct tcphdr*)(buffer);

    printf("\nTCP Header\n");
    printf("\t|-Source Port          : %u\n",ntohs(tcp->source));
    printf("\t|-Destination Port     : %u\n",ntohs(tcp->dest));
    printf("\t|-Sequence Number      : %u\n",ntohl(tcp->seq));
    printf("\t|-Acknowledge Number   : %u\n",ntohl(tcp->ack_seq));
    printf("\t|-Header Length        : %d DWORDS or %d BYTES\n" ,(unsigned int)tcp->doff,(unsigned int)tcp->doff*4);
    printf("\t|----------Flags-----------\n");
    printf("\t\t|-Urgent Flag          : %d\n",(unsigned int)tcp->urg);
    printf("\t\t|-Acknowledgement Flag : %d\n",(unsigned int)tcp->ack);
    printf("\t\t|-Push Flag            : %d\n",(unsigned int)tcp->psh);
    printf("\t\t|-Reset Flag           : %d\n",(unsigned int)tcp->rst);
    printf("\t\t|-Synchronise Flag     : %d\n",(unsigned int)tcp->syn);
    printf("\t\t|-Finish Flag          : %d\n",(unsigned int)tcp->fin);
    printf("\t|-Window size          : %d\n",ntohs(tcp->window));
    printf("\t|-Checksum             : %d\n",ntohs(tcp->check));
    printf("\t|-Urgent Pointer       : %d\n",tcp->urg_ptr);

    return 1;
}

int udp_header(unsigned char *buffer){
	struct udphdr *udp = (struct udphdr*)(buffer);

    printf("\t|-Source Port    	: %d\n" , ntohs(udp->source));
    printf("\t|-Destination Port	: %d\n" , ntohs(udp->dest));
    printf("\t|-UDP Length      	: %d\n" , ntohs(udp->len));
    printf("\t|-UDP Checksum   	: %d\n" , ntohs(udp->check));

    return 1;
}

int ip_header(unsigned char *buffer){

    struct sockaddr_in source, dest;
    struct iphdr *ip = (struct iphdr *)(buffer);
    memset(&source, 0, sizeof(source));
    memset(&dest, 0, sizeof(dest));
    source.sin_addr.s_addr = ip->saddr;
    dest.sin_addr.s_addr = ip->daddr;

    int iphdrlen = ip->ihl * 4;

    printf("\t|-Version : %d\n",(unsigned int)ip->version);
    printf("\t|-Internet Header Length : %d DWORDS or %d Bytes\n",(unsigned int)ip->ihl,((unsigned int)(ip->ihl))*4);
    printf("\t|-Type Of Service : %d\n",(unsigned int)ip->tos);
    printf("\t|-Total Length : %d Bytes\n",ntohs(ip->tot_len));
    printf("\t|-Identification : %d\n",ntohs(ip->id));
    printf("\t|-Time To Live : %d\n",(unsigned int)ip->ttl);
    printf("\t|-Protocol : %d\n",(unsigned int)ip->protocol);
    printf("\t|-Header Checksum : %d\n",ntohs(ip->check));
    printf("\t|-Source IP : %s\n", inet_ntoa(source.sin_addr));
    printf("\t|-Destination IP : %s\n",inet_ntoa(dest.sin_addr));


    if(ip->protocol == 17){ // if ip protocol is UDP
        return udp_header(buffer + iphdrlen);
    }
    else if (ip->protocol == 6) // if ip protocol is TCP
    {
        return tcp_header(buffer + iphdrlen);
    }

    return -1;
}

int eth_header(unsigned char *buffer){
    struct ethhdr *eth = (struct ethhdr *)(buffer);

    printf("\nEthernet Header");
    printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
    printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
    printf("\t|-Protocol : %d\n",eth->h_proto);


    if(eth->h_proto != 8) return -1; // if next header isnt an IP header

    return ip_header(buffer + sizeof(struct ethhdr));
}

namespace flows{

    std::unordered_map<char*, Flow*> database;

    char *getKeyFromPacket(unsigned char *buffer){
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        int iphdrlen = ip->ihl * 4;

        if(ip->protocol == 17){ // if ip protocol is UDP
            struct udphdr *udp = (struct udphdr*)(buffer + sizeof(struct ethhdr) + iphdrlen);
            return hashIP::hash(ip->saddr, udp->source, ip->daddr, udp->dest, 17);
        }
        else if (ip->protocol == 6) // if ip protocol is TCP
        {
            struct tcphdr *tcp = (struct tcphdr*)(buffer + sizeof(struct ethhdr) + iphdrlen);
            return hashIP::hash(ip->saddr, tcp->source, ip->daddr, tcp->dest, 6);
        }
    }

    Flow *createFlow(unsigned char *buffer){
        struct ethhdr *eth = (struct ethhdr *)(buffer);
        struct iphdr *ip = (struct iphdr *)(buffer + sizeof(struct ethhdr));
        int iphdrlen = ip->ihl * 4;

        Flow *flow = new Flow();
        flow->ip_src = ip->saddr;
        flow->ip_dst = ip->daddr;
        flow->ip_proto = ip->protocol;

        flow->flow_start_milliseconds = util::now();
        flow->flow_end_milliseconds = flow->flow_start_milliseconds;

        if(ip->protocol == 17){ // if ip protocol is UDP
            struct udphdr *udp = (struct udphdr*)(buffer + sizeof(struct ethhdr) + iphdrlen);
            flow->port_src = udp->source;
            flow->port_dst = udp->dest;
            flow->bytes = udp->len;
        }
        else if (ip->protocol == 6) // if ip protocol is TCP
        {
            struct tcphdr *tcp = (struct tcphdr*)(buffer + sizeof(struct ethhdr) + iphdrlen);
            flow->port_src = tcp->source;
            flow->port_dst = tcp->dest;
            // https://stackoverflow.com/a/6639856
            flow->bytes = ip->tot_len - (ip->ihl + tcp->th_off) * 4;

            //flags
            flow->flags = 0;
            flow->flags |= tcp->urg << 5;
            flow->flags |= tcp->ack << 4;
            flow->flags |= tcp->psh << 3;
            flow->flags |= tcp->rst << 2;
            flow->flags |= tcp->syn << 1;
            flow->flags |= tcp->fin << 0;
        }

        flow->pkts = 1;

        return flow;
    }

    Flow *mergeFlows(Flow *flow, char *key){
        Flow *base_flow = database[key];

        base_flow->flow_end_milliseconds = util::now();
        base_flow->pkts++;
        base_flow->bytes += flow->bytes;
        base_flow->flags += flow->flags;

        delete flow;

        return base_flow;
    }

    bool isFinished(Flow *flow){
        return false;
    }

    void moveFlow(Flow *flow, char *key){
        

        database.erase(key);
        delete flow;
    }

    int add(unsigned char *buffer){
        #if DEBUG
            //print packet
            eth_header(buffer);
        #endif

        char *key = getKeyFromPacket(buffer);
        Flow *flow = createFlow(buffer);

        // if database[key] = null
        if (database.find(key) == database.end())
            database[key] = flow;
        else{
            Flow *base_flow = mergeFlows(flow, key);

            if(isFinished(base_flow))
                moveFlow(base_flow, key);
        }
    }
}