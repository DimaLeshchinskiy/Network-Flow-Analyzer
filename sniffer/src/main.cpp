#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <iostream>
#include <arpa/inet.h>
#include<sys/types.h>

#include<linux/if_packet.h>
#include<netinet/if_ether.h>
#include<netinet/ip.h>
#include<netinet/udp.h>	
#include<netinet/tcp.h>

#include "hash.h"

int tcp_header(unsigned char *buffer){
  struct tcphdr *tcp = (struct tcphdr*)(buffer);

  #if DEBUG
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
  #endif

  return 6;
}

int udp_header(unsigned char *buffer){
	struct udphdr *udp = (struct udphdr*)(buffer);

  #if DEBUG
    printf("\t|-Source Port    	: %d\n" , ntohs(udp->source));
    printf("\t|-Destination Port	: %d\n" , ntohs(udp->dest));
    printf("\t|-UDP Length      	: %d\n" , ntohs(udp->len));
    printf("\t|-UDP Checksum   	: %d\n" , ntohs(udp->check));
  #endif

  return 17;
}

int ip_header(unsigned char *buffer){

  struct sockaddr_in source, dest;
  struct iphdr *ip = (struct iphdr *)(buffer);
  memset(&source, 0, sizeof(source));
  memset(&dest, 0, sizeof(dest));
  source.sin_addr.s_addr = ip->saddr;
  dest.sin_addr.s_addr = ip->daddr;

  int iphdrlen = ip->ihl * 4;

  #if DEBUG
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
  #endif

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

  #if DEBUG
    std::cout << "\n" << "Ethernet Header" << "\n";
    printf("\t|-Source Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_source[0],eth->h_source[1],eth->h_source[2],eth->h_source[3],eth->h_source[4],eth->h_source[5]);
    printf("\t|-Destination Address : %.2X-%.2X-%.2X-%.2X-%.2X-%.2X\n",eth->h_dest[0],eth->h_dest[1],eth->h_dest[2],eth->h_dest[3],eth->h_dest[4],eth->h_dest[5]);
    printf("\t|-Protocol : %d\n",eth->h_proto);
  #endif

  if(eth->h_proto != 8) return -1; // if next header isnt an IP header

  return ip_header(buffer + sizeof(struct ethhdr));
}

int main()
{
  int sock_r;
  sock_r = socket(AF_PACKET,SOCK_RAW,htons(0x0003));

  if(sock_r < 0)
  { 
    return -1;
  }

  unsigned char *buffer = new unsigned char[65536];
  memset(buffer,0,65536);
  struct sockaddr saddr;
  int saddr_len = sizeof (saddr);

  std::cout << "\n" << "Recieving..." << "\n";
  
  while(true){
    int buflen = recvfrom(sock_r,buffer, 65536, 0, &saddr, (socklen_t *)&saddr_len);
    if(buflen > 0)
    {
      int success = eth_header(buffer);
      if(success > 0){

      }

      if(success == -1) continue;
    }
  }

  return 0;
}