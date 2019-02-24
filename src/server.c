#include "server.h"
#include "common.h"
#include "object.h"
#include "db.h"
#include "command.h"

extern dictType dbDictType;

// struct redisServer server;
// sqlite3 *db;

void setNonblocking(int sockfd)
{
	int opts;
    opts=fcntl(sockfd,F_GETFL);
    if(opts<0)
    {
        perror("fcntl(sock,GETFL)");
        return;
    }//if

    opts = opts|O_NONBLOCK;
    if(fcntl(sockfd,F_SETFL,opts)<0)
    {
 		perror("fcntl(sock,SETFL,opts)");
        return;
    }//if
}

// void openSql(){
// 	int ret;
// 	ret = sqlite3_open(DB_NAME, &db);
// 	if(ret != SQLITE_OK)
// 	{
// 		printf("unable open database!\n");
// 		_exit(EXIT_FAILURE);
// 	}
// 	_exit(EXIT_SUCCESS);
// }

int main(int argc , char **argv)
{
	// socket
	int i, listenfd, connfd, sockfd, epfd, nfds;
	ssize_t n, ret;
	char inbuf[MAX_LINE];
	sds outbuf = sdsnewlen("", MAX_LINE);
	socklen_t clilen;
	struct sockaddr_in servaddr , cliaddr;
	/*声明epoll_event结构体变量，ev用于注册事件，数组用于回传要处理的事件*/
	struct epoll_event ev, events[20];

	// sqlite
	// char *errmsg = NULL;
	// char **dbRet;
	// int nRow , nCol ,  idx;

	// openSql();
	// int keyId = 1;
	// memset(sql , 0 , sizeof(sql));
	// sprintf(sql , "select * from counter where key_id='%d'", keyId);
	// ret = sqlite3_get_table(db , sql , &dbRet , &nRow , &nCol , &errmsg);
	// /*查询不成功*/
	// if(ret != SQLITE_OK)
	// {
	// 	sqlite3_close(db);
	// 	printf("database select fail!\n");
	// 	_exit(EXIT_FAILURE);
	// }

	/*(1) 得到监听描述符*/
	listenfd = socket(AF_INET , SOCK_STREAM , 0);
	setNonblocking(listenfd);

	/*生成用于处理accept的epoll专用文件描述符*/	
	epfd = epoll_create(CONNECT_SIZE);
	/*设置监听描述符*/
	ev.data.fd = listenfd;
	/*设置处理事件类型*/
	ev.events = EPOLLIN | EPOLLET;
	/*注册事件*/
	epoll_ctl(epfd, EPOLL_CTL_ADD, listenfd, &ev);		

	/*(2) 绑定套接字*/	
	bzero(&servaddr , sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(PORT);

	bind(listenfd , (struct sockaddr *)&servaddr , sizeof(servaddr));

	/*(3) 监听*/
	listen(listenfd , LISTENQ);

	// 实例化redisDb
	redisDb* db = zmalloc(sizeof(redisDb));
    db->dict = dictCreate(&dbDictType,NULL);

	/*(4) 进入服务器接收请求死循环*/
	while(1)
	{
		/*等待事件发生*/
		nfds = epoll_wait(epfd , events , CONNECT_SIZE , -1);
		if(nfds <= 0)
			continue;
	
		printf("nfds = %d\n" , nfds);
		/*处理发生的事件*/
		for(i=0 ; i<nfds ; ++i)
		{
			/*检测到用户链接*/
			if(events[i].data.fd == listenfd)
			{	
				/*接收客户端的请求*/
				clilen = sizeof(cliaddr);

				if((connfd = accept(listenfd , (struct sockaddr *)&cliaddr , &clilen)) < 0)
				{
					perror("accept error.\n");
					exit(1);
				}//if		

				printf("accpet a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr) , cliaddr.sin_port);
			
				/*设置为非阻塞*/
				setNonblocking(connfd);
				ev.data.fd = connfd;
				ev.events = EPOLLIN | EPOLLET;
				epoll_ctl(epfd , EPOLL_CTL_ADD , connfd , &ev);
			}//if
			/*如果是已链接用户，并且收到数据，进行读入*/
			else if(events[i].events & EPOLLIN){

				if((sockfd = events[i].data.fd) < 0)
					continue;
				bzero(inbuf , MAX_LINE);
				
				printf("reading the socket~~~\n");
				if((n = read(sockfd , inbuf , MAX_LINE)) <= 0)
				{
					close(sockfd);
					events[i].data.fd = -1;
				}//if
				else{
					inbuf[n] = '\0';
					printf("clint[%d] send message: %s\n", i , inbuf);
					commandServer(db, inbuf, outbuf);


					// robj* ret = dictFetchValue(db->dict, sdsnew("one"));

					// printf("%d\n", (int*)(ret->ptr));
				
					/*设置用于注册写操作文件描述符和事件*/
					ev.data.fd = sockfd;
					ev.events = EPOLLOUT| EPOLLET;	
					epoll_ctl(epfd , EPOLL_CTL_MOD , sockfd , &ev);			
				}//else											
			}//else
			else if(events[i].events & EPOLLOUT)
			{
				if((sockfd = events[i].data.fd) < 0)
				continue;
                
                if((ret = write(sockfd , outbuf , sdslen(outbuf))) != sdslen(outbuf))	
				{
					printf("error writing to the sockfd!\n");
					break;
				}//if
				/*设置用于读的文件描述符和事件*/
				ev.data.fd = sockfd;
				ev.events = EPOLLIN | EPOLLET;
				/*修改*/
				epoll_ctl(epfd , EPOLL_CTL_MOD , sockfd , &ev);				
			}//else
		}//for
	}//while
	free(events);
	close(epfd);
	exit(0);
}
