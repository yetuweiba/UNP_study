#ifndef UTIL_H_
#define UTIL_H_
#include<sys/socket.h>
#include<bits/types.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<errno.h>
#include<iostream>
#include<unistd.h>

int Writen(int fd, const void *vptr, size_t n)
{
    size_t nleft = n;
    size_t nwritten = 0;
    const char* ptr = (const char*)vptr;

    while(nleft > 0)
    {
        if((nwritten = write(fd, vptr, nleft)) <= 0)
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
        nleft = nleft - nwritten;
        ptr += nwritten;
    }

    return n;
}

void Write(int fd, void *ptr, size_t nbytes)
{
    if(write(fd, ptr, nbytes) != nbytes)
        std::cout << "write error" << std::endl;
}


size_t Read(int fd, void *ptr, size_t nbytes)
{
    ssize_t n = 0;
    if((n = read(fd, ptr, nbytes)) == -1)
    {
        std::cout << "Read error" << std::endl;
    }
    return n;
}

int readline(int fd, char *buffer)
{
    if(!buf)
    {
        return 0;
    }

    char data = 0;
    int read_len = 0;
    while(true)
    {
        rc = read(fd, &data, 1);
        if(rc == 1)
        {
            if(data == '\n')
            {
                buffer[++read_len] = '\0';
                break;
            }
            buffer[++read_len] = data;
        }
        else if(rc == 0)
        {
            buffer[++read_len] = '\0';
            break;
        }
        else
        {
            if(errno == EINTR)
                continue;
            read_len = -1;
            break;
        }
    }

    return read_len;
}
#endif
