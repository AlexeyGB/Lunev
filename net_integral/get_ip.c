#include <stdio.h>
#include <sys/types.h>
#include <ifaddrs.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>


struct in_addr getIP()
{
    struct in_addr myIP;
    struct ifaddrs * ifAddrStruct = NULL;
    struct ifaddrs * ifa = NULL;
 
    getifaddrs(&ifAddrStruct);

    for (ifa = ifAddrStruct; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa ->ifa_addr->sa_family==AF_INET) { // Check it is
            // a valid IPv4 address
            if( !( ifa->ifa_name[0] == 'l' && ifa->ifa_name[1] == 'o' && ifa->ifa_name[2] == '\000' ) )
            {
                myIP = ((struct sockaddr_in *)ifa->ifa_addr)->sin_addr;
                break;
            }
        }
    }
    if (ifAddrStruct != NULL)
        freeifaddrs(ifAddrStruct);
    return myIP;
}

int main (int argc, const char * argv[]) {
    struct in_addr IP = getIP();
    printf("%s\n",  inet_ntoa(IP) );
    return 0;
}
