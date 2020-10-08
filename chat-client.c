#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#define PORT 10140

int main(int argc,char **argv)
{

    int sock;
    struct sockaddr_in host;
    struct hostent *hp;
    int reuse;
    char rbuf[1024],buf[1024];
    int buflen,nbytes;
    fd_set rfds;
    struct timeval tv;
    char name[64]= {"\0"};

    
    //ソケットの生成
    if((sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0){
        perror("socket");
        exit(1);
    }

    //ソケットアドレスの再指定
    reuse =1;
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0){
        perror("setsocket");
        exit(1);
    }


    //hostと接続する
    bzero(&host,sizeof(host));
    host.sin_family = AF_INET;
    host.sin_port = htons(PORT); 



    if((hp = gethostbyname(argv[1])) ==NULL){
        printf("hosterror");
        exit(1);
    }
    bcopy(hp->h_addr,&host.sin_addr,hp->h_length);

    //ソケットをサーバに接続
    if(connect(sock,(struct sockaddr *)&host,sizeof(host))<0){
        perror("client: connect");
        exit(1);
    }else{
        printf("connected\n");
    }

    if((nbytes == read(sock,rbuf,sizeof(rbuf)))<0){
        perror("client read");
    }
    if((strncmp(rbuf,"REQUEST ACCEPTED\n",17))!=0){
        printf("rbuf:%s",rbuf);
        close(sock);
        exit(0);
    }
    printf("name:%s",argv[2]);
    strcat(name,argv[2]);
    strcat(name,"\n");
    printf("name:%s",name);
    if((nbytes = write(sock,name,strlen(name)))<0){
        perror("name write");
    }

    if((nbytes = read(sock,rbuf,sizeof(rbuf)))<0){
        perror("name answer");
    }
    if(strncmp(rbuf,"USERNAME REGISTERED\n",20)!=0){
        printf("socket close");
        close(sock);
        exit(0);
     }


    //データの送受信
    do{
        //入力を関しするファイルの記述子の集合を変数rfdsにセットする

        FD_ZERO(&rfds);
        FD_SET(0,&rfds);
        FD_SET(sock,&rfds);

        tv.tv_sec = 1;
        tv.tv_usec =0;

        if(select(sock+1,&rfds,NULL,NULL,&tv)>0){
            if(FD_ISSET(0,&rfds)){
                fgets(rbuf,1024,stdin);
                if((nbytes = write(sock,rbuf,sizeof(rbuf)))<0){
                    perror("write error");
                }
            }

            if(FD_ISSET(sock,&rfds)){

	      if((nbytes = read(sock,rbuf,sizeof(rbuf)))<0){
                    perror("read error");
                }
                else {
                    printf("sever:%s",rbuf);
                }
                

            }
        }
        bzero(rbuf,1024);
    }while(1);





    

    close(sock);
    printf("closed\n");

    






}
