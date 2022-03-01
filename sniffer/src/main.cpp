#include "main.h"

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
      flows::add(buffer);
    }
  }

  return 0;
}