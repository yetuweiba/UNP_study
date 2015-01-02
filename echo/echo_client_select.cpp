#include "util.hpp"
#include <arpa/inet.h>
#include <stdio.h>
#include <iostream>

void cli_echo(FILE* fp, const int sockfd)
{
    int maxfd, stdineof;
    fd_set rset;
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    FD_ZERO(&rset);

    stdineof = 0;
    
    int n = 0;
    for(; ;)
    {
        if(stdineof == 0)
        {
            FD_SET(fileno(fp), &rset);
        }
        FD_SET(sockfd, &rset);
        maxfd = std::max(fileno(fp), sockfd) + 1;
        select(maxfd, &rset, NULL, NULL, NULL);
        if(FD_ISSET(sockfd, &rset))
        {
            if((n = Read(sockfd, buf, 1024)) == 0)
            {
                if(stdineof == 1)
                {
                    std::cout << "normal termination" << std::endl;
                    return;
                }
                else
                {
                    std::cout << "error" << std::endl;
                    break;
                }
            }
            std::cout << "buf: " << buf << std::endl;
        }

        if(FD_ISSET(fileno(fp), &rset))
        {
            if((n = Read(fileno(fp), buf, 1024)) == 0)
            {
                std::cout << buf << std::endl;
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(fp), &rset);

                continue;
            }
            Write(sockfd, buf, n);
        }
    }

}

int main(int argc, char **argv)
{
    int sockfd = 0;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        std::cout << "Socket is error: " << sockfd << std::endl;
        return sockfd;
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9877);
    inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
    int ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret != 0)
    {
        std::cout << "connect is error: " << ret << std::endl;
        close(sockfd);
        return ret;
    }
    cli_echo(stdin, sockfd);
    return 0;
}
