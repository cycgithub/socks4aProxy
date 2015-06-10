//socks4aProxy服务器端
#include<stdio.h>
#include<malloc.h>

#include<fstream>
#include<iostream>
#include<string>

#include<event.h>
#include<netdb.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<fcntl.h>
#include<arpa/inet.h>
using namespace std;

//struct event_base* main_base;

//void on_read(int sock,short event,void * arg)
void on_read(int sock)
{
	int ip[4];
    int recvbyte=0;
    //struct event* ev=(struct event *)arg ;
	//接收第一个报文
    char bufferClient[10250];
	memset(bufferClient,0,sizeof(bufferClient));
	int size=recv(sock,bufferClient,10250,0);
	cout<<"size1:"<<size<<endl;
	for(int i=0;i<size;++i)
	{
		printf("%d  ",bufferClient[i]&0x00ff);
	}
	//提取报文信息目的IP PORT
	char ipp[4];int port=80;
	int p1=bufferClient[2]&0x00ff;
	int p2=bufferClient[3]&0x00ff;
	port=p1*256+p2;
	string ippp;
	for(int i=0;i<4;++i)
	{
		memset(ipp,'\0',sizeof ipp);
		ip[i]=bufferClient[i+4]&0x00ff;
        sprintf(ipp,"%d",ip[i]);
        ippp.append(ipp,strlen(ipp));
     	if(i<3)ippp+=".";
	}
	cout<<ippp<<endl;
    if(size<=0){close(sock);return;}
	//应答第一个报文
	char FirstResponse[9];
	for(int i=0;i<8;++i)
    FirstResponse[i]=bufferClient[i];
	FirstResponse[8]='\0';
	FirstResponse[0]=0x00;
    FirstResponse[1]='Z';
	//for(int i=0;i<8;++i)
		//printf("%d  ",FirstResponse[i]&0x00ff);
	send(sock,FirstResponse,8,0);
	//接收第二个报文
	memset(bufferClient,'\0',sizeof(bufferClient));
	size=recv(sock,bufferClient,1025,0);

     
	cout<<"size2:"<<size<<endl;
    cout<<"client ->proxy"<<bufferClient<<endl;

    //printf("client -> proxy:\n%s",bufferClient);
    printf("\n");
    if(size<=0)
    {
		//event_del(ev);free(ev);
        close(sock);
        return;
	}
    string sendss="";
	sendss.append(bufferClient,size);
	string tempss=sendss;
	//string hostname;
	cout<<"proxy -> server\r\n"<<bufferClient<<endl;
	//cout<<"cyc host name:"<<hostname<<endl;
	//proxy connect dest
	int ss;
	struct  sockaddr_in destaddr;

	//hostent* he=gethostbyname(hostname.c_str());
    // bzero(&destaddr,sizeof(struct sockaddr_in));
	//char **pptr=he->h_addr_list;
    destaddr.sin_family=AF_INET;
    //destaddr.sin_addr.s_addr=inet_addr("119.75.217.109");//119.75.217.109
    destaddr.sin_addr.s_addr=inet_addr(ippp.c_str());//119.75.217.109
	//destaddr.sin_addr=*((struct in_addr*)he->h_addr);
	//cout<<"ip address:"<<inet_ntoa(*(struct in_addr* )*pptr)<<endl;
    destaddr.sin_port=htons(port);
	memset(destaddr.sin_zero,'\0',sizeof destaddr.sin_zero);
    ss=socket(AF_INET,SOCK_STREAM,0);
    char bufferServer[10240];
	if(connect(ss,(struct sockaddr *)&destaddr,sizeof destaddr)==-1)
	{
		cout<<"connct fail"<<endl;
		return ; 
    }
    //转发给服务器第二个请求报文
    send(ss,bufferClient,size,0);
	string str;
    int countbyte=0;
	//将从服务器端接收的报文转发给客户端
	do
	{
		recvbyte=recv(ss,bufferServer,10240,0);

        if(recvbyte<=0)
			break ;
		send(sock,bufferServer,recvbyte,0);
		countbyte+=recvbyte;
		str.append(bufferServer,recvbyte);
	}while(recvbyte>0);
	close(sock);
	close(ss);
}
//void accept_handle(const int sfd, const short event, void *arg)
void accept_handle(const int sfd)
{
	cout<<"accept handle"<<endl;
	while(1)
	{
		struct sockaddr_in addr;

	    socklen_t addrlen = sizeof(addr);
		//处理连接
        int fd = accept(sfd, (struct sockaddr *) &addr, &addrlen);
		if(!fork())
		{
			close(sfd);
			on_read(fd);
			return ;
		}
		close(fd);
	}
	return ;
	//通过libevent方式
    //struct event *read_ev=(struct event *)malloc(sizeof(struct event));
    //event_set(read_ev,fd,EV_READ|EV_PERSIST,on_read,read_ev);
    //event_base_set(main_base,read_ev);
    //event_add(read_ev,NULL);       
    //struct bufferevent* buf_ev;

	//buf_ev = bufferevent_new(fd, NULL, NULL, NULL, NULL);

    //buf_ev->wm_read.high = 4096;

	//bufferevent_write(buf_ev, MESSAGE1, 8);//strlen(MESSAGE1));
       

}

int main()
{
	cout<<"hello man!"<<endl;

	//1. 初始化EVENT
    //main_base = event_init();
	//if(main_base)
	//	cout<<"init event ok!"<<endl;

	// 2. 初始化SOCKET
	int sListen;

	// Create listening socket
	sListen = socket(AF_INET, SOCK_STREAM,0);// IPPROTO_TCP);

	struct sockaddr_in server_addr;
	//bzero(&server_addr,sizeof(struct sockaddr_in));
	server_addr.sin_family=AF_INET;
	//server_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    server_addr.sin_addr.s_addr=INADDR_ANY;//inet_addr("192.168.72.134");
	int portnumber = 9080;
	server_addr.sin_port = htons(portnumber);
    memset(server_addr.sin_zero,'\0',sizeof server_addr.sin_zero);
	//捆绑sockfd描述符
	if(bind(sListen,(struct sockaddr *)(&server_addr),sizeof(struct sockaddr))==-1)
	{
		cout<<"error!"<<endl;
		return -1;
	}
        cout<<"bind sucess"<<endl;
	// Listen
	::listen(sListen, 5);
	cout<<"Server is listening!\n"<<endl;

	//将描述符设置为非阻塞
	//int flags = ::fcntl(sListen, F_GETFL);

	//flags |= O_NONBLOCK;short a=0;
	accept_handle(sListen);
    //struct event ev;
        
	//fcntl(sListen, F_SETFL, flags);
    //accept_handle(sListen,a,&ev);
	//3. 创建EVENT 事件
    //struct event ev;
    //event_set(&ev, sListen, EV_READ | EV_PERSIST, accept_handle, (void *)&ev);

	//4. 事件添加与删除
	//event_add(&ev, NULL);

	//5. 进入事件循环
	//event_base_loop(main_base, 0);
    //event_dispatch();
	cout<<"over!"<<endl;
	return 0;
}
