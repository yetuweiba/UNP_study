#include "util.hpp"
#include <limits.h>
#include <poll.h>
#include <sys/stropts.h>

#ifndef INFTIM
#define INFTIM -1
#endif

int main(int argc, char **argv)
{
    int listenfd = -1;
    struct sockaddr_in sock_server;
    sock_server.sin_family = AF_INET;
    sock_server.sin_port = htons(9877);
    sock_server.sin_addr.s_addr = htonl(INADDR_ANY);
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        std::cout << "socket is error: " << listenfd << std::endl;
        return listenfd;
    }

    int ret = bind(listenfd, (struct sockaddr*)&sock_server, sizeof(sock_server));
    if(ret != 0)
    {
        std::cout << "bind is error: " << ret << std::endl;
        return ret;
    }

    ret = listen(listenfd, 255);
    if(ret != 0)
    {
        std::cout << "listen is error: " << ret << std::endl;
        return ret;
    }
    
    struct pollfd client_fd[255];
    client_fd[0].fd = listenfd;
    client_fd[0].events = POLLRDNORM;
    for(int i = 1; i < 254; ++i)
    {
        client_fd[i].fd = -1;
    }

    int ready_ret = -1;
    int maxfd = 0;
    int client_len = 0;
    int connfd = -1;
    struct sockaddr_in client_addr;
    char buf[1024];
    memset(buf, 0, sizeof(buf));
    for(;;)
    {
        ready_ret = poll(client_fd, maxfd + 1, INFTIM);
        if(ready_ret < 0)
        {
            std::cout << "poll is error: " << ready_ret << std::endl;
        }


        if(client_fd[0].revents & POLLRDNORM)
        {
            client_len = sizeof(client_addr);
            connfd = accept(listenfd, (struct sockaddr*)&client_addr, (socklen_t *)&client_len);

            int i = 1;
            for(; i < 255; ++i)
            {
                if(client_fd[i].fd < 0)
                {
                    client_fd[i].fd = connfd;
                    break;
                }
            }
            if(i == 254)
            {
                std::cout << "too many clients" << std::endl;
                return 0;
            }

            client_fd[i].events = POLLRDNORM;
            if(i > maxfd)
            {
                maxfd = i;
            }
            if(--ready_ret < 0)
            {
                continue;
            }

        }

        for(int i = 1; i <= maxfd; ++i)
        {
            if(client_fd[i].fd < 0)
                continue;

            if(client_fd[i].revents & (POLLRDNORM | POLLERR))
            {
                int n = 0;
                if((n = read(client_fd[i].fd, buf, 1024)) < 0)
                {
                    if(errno == ECONNRESET)//connection is reset by client
                    {
                        close(client_fd[i].fd);
                        client_fd[i].fd = -1;
                    }
                    else
                    {
                        std::cout << "error client_fd: " << client_fd[i].fd << std::endl;
                    }
                }
                else if(n == 0)
                {
                    close(client_fd[i].fd);
                    client_fd[i].fd = -1;
                }
                else
                {
                    std::cout << "buf: " << buf << std::endl;
                    Writen(client_fd[i].fd, buf, n);
                }

                std::cout << "n: " << n << std::endl;
                if(--ready_ret <= 0)
                    break;
            }
        }
    }

}
