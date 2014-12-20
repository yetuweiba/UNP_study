#include <bits/types.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

//typedef void(*signal (int signo, void(*func)(int)))(int);
typedef void Sigfunc(int);
Sigfunc* signal(int signo, Sigfunc *func)
{
    struct sigaction act, oact;
    act.sa_handler = func;
    sigemptyset(&act.as_mask);
    act.sa_flags = 0;
    if(signo == SIGALRM)
    {
#ifdef SA_INTERRUPT
        act.sa_flags |= SA_INTERRUT;
#endif
    }
    else
    {
#ifdef SA_RESTART
        ac.as_flags |= SA_RESULT;
#endif
    }
    if(sigaction(signo, &act, &oact) < 0)
    {
        return SIG_ERR;
    }
    return oact.sa_handler;
}

void sig_chld(int signo)
{
    pid_t pid;
    int stat;

    while((pid = waitpid(-1, &stat, WNOHANG)) > 0)
        std::cout << "child: " << pid << " terminated" << std::endl;

    return;
}

Sigfunc * Signal(int signo, Sigfunc *func)
{
    Sigfunc *sigfunc;

    if((sigfunc = signal(signo, func)) == SIG_ERR)
        err_sys("signal error");
    return sigfunc;
}

int Writen(int fd, const void *vptr, size_t n)
{
    int nleft = n;
    int nwritten = 0;
    const char * ptr = (const char *)vptr;

    while(nleft > 0)
    {
        if((nwritten = write(fd, ptr, nleft)) <= 0)
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
        nleft = n - nwritten;
        ptr += nwritten;
    }

    return n;
}

void str_echo(int sockfd)
{
    int n = 0;
    char buf[1024];
again:
    std::cout << "into again:" << std::endl;
    while((n = read(sockfd, buf, 1024)) > 0)
    {
        std::cout << "n:" << n << std::endl;
        //std::cout << "recv:" << buf << std::endl;
        Writen(sockfd, buf, n);
    }

    if(n < 0 && errno == EINTR)
    {
        goto again;
    }
    else if(n < 0)
    {
        std::cout << "read error" << std::endl;
    }
}

int main(int argc, char **argv)
{
    int listenfd = 0;
    int connfd = 0;
    pid_t childpid;
    socklen_t clilen;

    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd < 0)
    {
        std::cout << "socket error" << std::endl;
        return listenfd;
    }
    
    struct sockaddr_in cliaddr;
    struct sockaddr_in servaddr;
    bzero(&servaddr, sizeof(sockaddr_in));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htons(INADDR_ANY);
    servaddr.sin_port = htons(9877);

    int ret = bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    if(ret != 0)
    {
        std::cout << "bind is error: " << ret << std::endl;
        return ret;
    }

    ret = listen(listenfd, 1024);
    if(ret < 0)
    {
        std::cout << "listen is error: " << ret << std::endl;
        return ret;
    }

    for( ; ; )
    {
        std::cout << "process" << std::endl;
        clilen = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
        if(connfd < 0)
        {
            std::cout << "accept is error: " << connfd << std::endl;
            std::cout << "error:" << errno << std::endl;
        }
        if ((childpid = fork()) ==0 )
        {
            close(listenfd);
            str_echo(connfd);
            exit(0);
        }
        close(connfd);
    }
}
