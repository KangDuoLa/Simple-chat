#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>
#include <pthread.h>
#include "chat.h"
#include "list.h"

#define MAXLINE 128
#define SERV_PORT 2018
mylink head = NULL;

pthread_mutex_t mutex; 

struct s_info
{                     
    struct sockaddr_in cliaddr;
    int connfd;
};

void sys_err(char *str)
{
    perror(str);
    exit(1);
}


/*有新用户登录,将该用户插入链表*/
int login_qq(Chat *buf, mylink *head, int fd)
{
    mylink node = make_node(buf->srcname, fd);  	
	if(mylink_search(head, buf->srcname) != NULL)
	{
		buf->protocal_num = 4;
		write(fd, buf, sizeof(buf));
		sleep(1);
		close(fd);
		pthread_exit((void *)2);
	}	
	
    mylink_insert(head, node);                  	/*将新创建的节点插入链表*/
	
	printf("%s已登录\n", buf->srcname);
    return 0;
}

/*客户端发送聊天,服务器负责转发聊天内容*/
void transfer_qq(Chat *buf, mylink *head)
{
    mylink p = mylink_search(head, buf->desname);      /*遍历链表查询目标用户是否在线*/
    if(p == NULL) 
	{
        Chat lineout = {3};              				/*目标用户不在, 封装3号数据包*/
        strcpy(lineout.desname, buf->desname);        	/*将目标用户名写入3号包*/
        mylink q = mylink_search(head, buf->srcname);  
        
        write(q->fd, &lineout, sizeof(lineout));  
    }
	else
	{
        write(p->fd, buf, sizeof(*buf));           		/*目标用户在线,将数据包写给目标用户*/
	}
}

/*客户端退出*/
int exit_qq(Chat *buf, mylink *head)
{
    mylink p = mylink_search(head, buf->srcname);       /*从链表找到该客户节点*/
	close(p->fd);    	
	printf("%s已退出\n", buf->srcname);
    mylink_delete(head, p);                             /*将对应节点从链表摘下*/
	
	if(*head == NULL)
	{
		return 0;
	}
    free_node(p);                                       /*释放节点*/
	pthread_exit((void *)2);
}

void err_qq(Chat *buf)
{
    fprintf(stderr, "bad client %s connect \n", buf->srcname);
}

void *do_work(void *arg)
{
    int n,i;
    struct s_info *ts = (struct s_info*)arg;
    char buf[MAXLINE];
	Chat message;
  
	while(1)
	{
		read(ts->connfd, &message, sizeof(message));
		switch(message.protocal_num)
		{
			pthread_mutex_lock(&mutex);
			case 1: login_qq(&message, &head, ts->connfd); break;
			case 2: transfer_qq(&message, &head); break;
			case 4: exit_qq(&message, &head);                       //退出则线程也退出
				pthread_exit((void *)2);                            //结束线程
			default: err_qq(&message);
			pthread_mutex_lock(&mutex);
		}	
	}

    return (void *)0;
}


void main()
{
	struct sockaddr_in servaddr, cliaddr;
	int listenfd, connfd;
	int cliaddr_len;
	int i = 0;
	
	pthread_t tid;
	
	struct s_info ch[256];
	
	listenfd = socket(AF_INET, SOCK_STREAM, 0);
	
	int opt = 1;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	
	bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(SERV_PORT);
	
	bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
	
	listen(listenfd, 128);
		
	while (1) 
	{
        cliaddr_len = sizeof(cliaddr);
        connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);   //阻塞监听客户端链接请求
        ch[i].cliaddr = cliaddr;
        ch[i].connfd = connfd;

        /* 达到线程最大数时，pthread_create出错处理, 增加服务器稳定性 */
        pthread_create(&tid, NULL, do_work, (void*)&ch[i]);
        pthread_detach(tid);                                                    //子线程分离,防止僵线程产生.
        i++;
    }
}
















