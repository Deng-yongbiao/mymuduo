#include <sys/epoll.h>
#include <arpa/inet.h>
#include <cstring>
#include <iostream>
#include <fcntl.h>
#include <unistd.h>

void Epoll_add_event(int epollfd, int fd, int event)
{
    epoll_event ev;
    ev.events = event;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

void Server()
{
    int sockfd = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    bzero(&addr, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(9527);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) == -1)
    {
        std::cout << "bind error"
                  << "\n";
        return;
    }
    if (listen(sockfd, 5) == -1)
    {
        std::cout << "listen error"
                  << "\n";
        return;
    }
    std::cout << "listen OK............"
              << "\n";

    int epoll_fd = epoll_create(1024);
    if (epoll_fd == -1)
    {
        std::cout << "epoll create failed................"
                  << "\n";
        return;
    }

    Epoll_add_event(epoll_fd, sockfd, EPOLLIN);
    epoll_event all_events[100];
    while (true)
    {
        std::cout << "EPOLL waiting................"
                  << "\n";
        int num = epoll_wait(epoll_fd, all_events, 100, 1000);
        for (int i = 0; i < num; i++)
        {
            int fd = all_events[i].data.fd;
            if (EPOLLIN == all_events[i].events)
            {
                // 接受客户端请求的连接
                if (fd == sockfd)
                {
                    int clientfd = accept(sockfd, NULL, NULL);
                    if (clientfd == -1)
                    {
                        std::cout << "accept error.............\n";
                        return;
                    }
                    else
                    {
                        std::cout << "================>accept new client:" << clientfd << std::endl;
                        Epoll_add_event(epoll_fd, clientfd, EPOLLIN);
                    }
                }
                // 读取客户端发来的数据
                else
                {
                    char buf[128] = {0};
                    size_t size = read(fd, buf, sizeof(buf));
                    if (size > 0)
                    {
                        std::cout << "recv from clientfd:" << fd << ", " << buf << std::endl;
                    }
                }
            }
        }
    }
}

void Client()
{
    int sock = socket(PF_INET, SOCK_STREAM, 0);
    sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    serv_addr.sin_port = htons(9527);

    if (connect(sock, (sockaddr *)&serv_addr, sizeof(serv_addr)) == -1)
    {
        printf("connect error");
        close(sock);
        return;
    }
    char message[256] = "";
    while (true)
    {
        printf("Input message(q to quit):");
        fgets(message, sizeof(message), stdin);
        if (!strcmp(message, "q\n") || !strcmp(message, "Q\n"))
        {
            break;
        }
        write(sock, message, strlen(message));
        memset(message, 0, sizeof(message));
        ssize_t str_len = read(sock, message, sizeof(message));
        printf("server:%s\n", message);
    }
    close(sock);
    return;
}

int main(int argc, char const *argv[])
{
    // Server();
    Client();
    return 0;
}
