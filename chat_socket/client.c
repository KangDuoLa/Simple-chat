#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <signal.h>
#include <pthread.h>
#include "chat.h"

#define PORT 2018
#define MAXLINE 128


// 客户端接收消息的线程函数
void* recv_func(void *args)
{
    char buf[MAXLINE];
    int sock_fd = *(int*)args;
	int len = 0;
	Chat message;
	bzero(&message, sizeof(message));
    while(1) 
	{
		len = read(sock_fd, &message, sizeof(message));
		if(len > 0)
		{
			if(message.protocal_num == 3)
			{
				printf("对方不在线!\n");
			}
			else if(message.protocal_num == 2)
			{
				printf("                       %s: %s\n", message.srcname, message.content);
			}
			else if(message.protocal_num == 4)
			{
				printf("重名，重新登录！\n");
				close(sock_fd);
				exit(1);
			}
		}
		else if(len < 0) 
		{
			break;
        }	
    }
    close(sock_fd);
    exit(0);
}

// 客户端和服务端进行通信的处理函数
void process(int listenfd, char argv[20])
{
	Chat talk;
	bzero(&talk, sizeof(talk));
    pthread_t tid;
    pthread_create(&tid, NULL, recv_func, (void*)&listenfd);      

    char buf[MAXLINE];
    while(1) 
	{
		loop:
		bzero(buf, sizeof(buf));
        int len = read(STDIN_FILENO, buf, MAXLINE);
		buf[len - 1] = '\0';

		if(len > 0)
		{
			char *dname, *content;
			memset(&talk, 0, sizeof(talk));

			if(strcmp("exit", buf) == 0)
			{
				talk.protocal_num = 4;
				strcpy(talk.srcname, argv);
				write(listenfd, &talk, sizeof(talk));
				sleep(1);
				break;
			}
			else if(strstr(buf, "#") == NULL)
			{
				printf("输入有误!格式：desname#xxxxxx\n");
				goto loop;
			}
			else
			{
				dname = strtok(buf, "#\n");
				talk.protocal_num = 2;
				strcpy(talk.desname, dname);	
				strcpy(talk.srcname, argv);	
				content = strtok(NULL, "\0");
				strcpy(talk.content, content);
			}
			write(listenfd, &talk, sizeof(talk));
		}
	}	
    close(listenfd);
}

char str[20] = {0};
int listenfd;
void sig_handler(int sig)
{
	if(sig == SIGINT)
	{
		Chat Exit;
		bzero(&Exit, sizeof(Exit));
		Exit.protocal_num = 4;
		strcpy(Exit.srcname, str);
		write(listenfd, &Exit, sizeof(Exit));
		sleep(1);
		close(listenfd);
		printf("You have been put the --Ctrl C\n");
		exit(1);
	}
}


int main(int argc, char **argv)
{
	if(argc < 2)
	{
		printf("./client name");
		exit(1);
	}
	strcpy(str, argv[1]);
	
	signal(SIGINT, sig_handler);
	
	struct sockaddr_in servaddr;
	char buf[MAXLINE];
	int len;
	int flag;
	Chat login;
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	inet_pton(AF_INET, "127.0.0.1", &servaddr.sin_addr);
	servaddr.sin_port = htons(PORT);
	
	connect(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	
	bzero(&login, sizeof(login));
	
	login.protocal_num = 1;
	strcpy(login.srcname, argv[1]);
	write(listenfd, &login, sizeof(login));

	process(listenfd, argv[1]);
}



