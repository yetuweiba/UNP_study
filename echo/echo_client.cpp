#include <sys/socket.h>
#include <bits/types.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <iostream>
#include <arpa/inet.h>
#include <stdio.h>


int Writen(int fd, const void *vptr, size_t n)
{
    size_t nleft = n;
    size_t nwritten = 0;
    const char *ptr = (const char *)vptr;

    while(nleft > 0)
    {
        std::cout << "writen: " << nwritten << std::endl;
        if((nwritten = write(fd, ptr, nleft)) <= 0 )
        {
            if(nwritten < 0 && errno == EINTR)
            {
                nwritten = 0;
            }
            else
            {
                return -1;
            }
        }
        nleft -= nwritten;
        ptr += nwritten;
    }

    return n;
}

static int read_cnt;
static char *read_ptr;
static char read_buf[1024];

static ssize_t my_read(int fd, char *ptr)
{
    if(read_cnt <= 0)
    {
again:
        if((read_cnt = read(fd, read_buf, sizeof(read_buf))) < 0)
        {
            if(errno == EINTR)
            {
                goto again;
            }
            return -1;
        }
        else if(read_cnt == 0)
        {
            return 0;
        }
        read_ptr = read_buf;
    }
    read_cnt--;
    *ptr = *read_ptr++;
    return 1;
}

ssize_t readline(int fd, void *vptr, size_t maxlen)
{
    ssize_t n, rc;
    char c, *ptr;
    ptr = (char *)vptr;
    for(n = 1; n < maxlen; ++n)
    {
        if((rc = my_read(fd, &c)) == 1)
        {
            std::cout << "n:" << n << std::endl;
            *ptr++ = c;
            if(c == '\n')
            {
                break;
            }
        }
        else if(rc == 0)
        {
            *ptr = 0;
            return n - 1;
        }
        else
        {
            return -1;
        }
    }
    *ptr = 0;
    std::cout << "return n: " << n << std::endl;
    return n;

}

ssize_t Readline(int fd, void *ptr, size_t maxlen)
{
    ssize_t n;
    if((n = readline(fd, ptr, maxlen)) < 0)
    {
        std::cout << "Readline error" << std::endl;
    }
    return n;
}

void str_cli(FILE *fp, int sockfd)
{
    char sendline[1024] = {0};
    char recvline[1024] = {0};
    int n = 0;
    while(fgets(sendline, 1024, fp) != NULL)
    {
        std::cout << "sendline: "<< sendline << std::endl;
        std::cout << strlen(sendline) << std::endl; 
        Writen(sockfd, sendline, strlen(sendline));
        if((n = Readline(sockfd, recvline, 1024)) == 0)
        {
            std::cout << "server terminate" << std::endl;
        }
        std::cout << "recv_count: " << n << std::endl;
        std::cout << "recvline: " << recvline << std::endl;
        //if(fputs(recvline, stdout) < 0)
        //{
            //std::cout << "fputs is error" << std::endl;
        //}
    }
}

int main(int argc, char **argv)
{
    int sockfd;
    struct sockaddr_in servaddr;

    if(argc != 2)
    {
        std::cout << "error arg " << std::endl;
    }

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if(sockfd < 0)
    {
        std::cout << "socket error" << std::endl;
        return sockfd;
    }

    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(9877);
    if(!inet_pton(AF_INET, argv[1], &servaddr.sin_addr))
    {
        std::cout << "error" << std::endl;
        return 0;
    }

    int ret = connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret != 0)
    {
        std::cout << "error connect: " << ret << std::endl;
        return ret;
    }
    str_cli(stdin, sockfd);
    return 0;
}
