#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#define PORT 10140
#define MAXUSER 5
#define TIMEOUT 30


int main(int argc,char **argv)
{

  int sock,csock;
    int reuse=0;
    int i=0;
    int same=0;
    struct sockaddr_in svr;
    struct sockaddr_in clt;
    struct hostent *cp;
    char rbuf[1024];
    int clen;
    int buflen,nbytes;
    fd_set rfds;
    struct timeval tv;
    int dsock[MAXUSER]={-1,-1,-1,-1,-1};
    char name[10];
    char clientuser[MAXUSER][10]={"","","","",""};
    struct tm tm;
    char *week[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
    



    //ソケットの生成
    if((sock = socket(AF_INET,SOCK_STREAM,IPPROTO_TCP))<0){
        perror("socket");
        exit(1);
    }
    
    reuse=1;
    if(setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&reuse,sizeof(reuse))<0) {
      perror("setsockopt");
      exit(1);
    }
    
    //hostと接続する
    bzero(&svr,sizeof(svr));
    svr.sin_family = AF_INET;
    svr.sin_addr.s_addr=htonl(INADDR_ANY);
    svr.sin_port = htons(PORT);

    if(bind(sock,(struct sockaddr *)&svr,sizeof(svr))<0){
      perror("bind");
      exit(1);

    }

    //待受クライアント数の設定
    if(listen(sock,5)<0){
      perror("listen");
      exit(1);

    }

    int k=0;
   
    

   
    //データの送受信

    do{

     

     
      FD_ZERO(&rfds);
      FD_SET(0,&rfds);
      FD_SET(sock,&rfds);

      tv.tv_sec = 1;
      tv.tv_usec =0;

      select(sock+1,&rfds,NULL,NULL,&tv)>0;

      for(int x = 0; x < MAXUSER; x++){
           if(dsock[x] != -1){   
              //printf("nummber:%d %d\n",i,csock[i]);
                FD_SET(dsock[x], &rfds);
                select(dsock[x]+1, &rfds, NULL, NULL, &tv);
            }
      }        

      if(FD_ISSET(sock,&rfds)){

      
        clen = sizeof(clt);
        if((csock = accept(sock,(struct sockaddr *)&clt,&clen))<0){
	        perror("accept");
	        exit(2);
        }

        cp=gethostbyaddr((char * )&clt.sin_addr,sizeof(struct in_addr),AF_INET);
        printf("[%s]\n",cp->h_name);

        if(k>=MAXUSER){
          if(write(csock,"REQUEST REJECTED\n",strlen("REQUEST REJECTED\n"))<0){
              perror("write csock\n");
          }          
          close(csock);

        }
        else{
          write(csock,"REQUEST ACCEPTED\n",strlen("REQUEST ACCEPTED\n"));
          if((nbytes=read(csock,name,sizeof(name)))<0){
            perror("name receive");
          }
          else{
            
            
            for(int i=0;i<MAXUSER;i++){
              if(strncmp(clientuser[i],name,nbytes-1)==0){
                write(csock,"USERNAME REJECTED\n",strlen("USERNAME REJECTED\n"));
                printf("%s is already joined",name);
                close(csock);
                same++;
              }
            }
            if(same==0){
            nbytes--;
            strncpy(clientuser[k],name,nbytes);
            clientuser[k][nbytes]='\0';
            
            write(csock,"USERNAME REGISTERED\n",strlen("USERNAME REGISTERED\n"));
          
           
 
       
            dsock[k]=csock;
	    k++;
      }
      same=0;
      printf("k=%d",k);
      for(int h=0;h<k;h++){
        printf("client[%d];%s\n",h,clientuser[h]);
      }
          }
          

        }
      }
        //入力を関しするファイルの記述子の集合を変数rfdsにセットする
     
receive:       for(i=0;i<MAXUSER;i++){
//printf("i;%d",i);

            if(FD_ISSET(dsock[i],&rfds)){
              printf("dsock");
             

	      if((nbytes = read(dsock[i],rbuf,sizeof(rbuf)))<0){
                    perror("read error");
        }
        else if(nbytes>0){
            
        if(strncmp(rbuf,"/list",5) == 0){
                char namebuf[50]="";
               
                for(int namecount=0;namecount<k;namecount++){
                strcat(namebuf,"client:");
                strcat(namebuf,clientuser[namecount]);
                strcat(namebuf,"\n");
                }
                 for(int b=0;b<MAXUSER;b++){
                    if(dsock[b]!=-1) {
                          if(write(dsock[b],namebuf,strlen(namebuf))){
                            perror("socket write");
                      }
                    }           

                  }
                  goto receive;

                
            }

                    
              printf("client:%s",clientuser[i]);
             char msgbuf[1024]={};
             time_t t = time(NULL);
             localtime_r(&t,&tm);
             int yaer = tm.tm_year + 1900;

             sprintf(msgbuf,"%04d/%02d/%02d %s %02d:%02d:%02d :",yaer,tm.tm_mon+1,tm.tm_mday,week[tm.tm_wday],tm.tm_hour,tm.tm_min,tm.tm_sec);
              
              strcat(msgbuf,clientuser[i]);
              strcat(msgbuf,">");
              strcat(msgbuf,rbuf);
 //             printf("msgbuf:%s",msgbuf);
                  for(int b=0;b<MAXUSER;b++){
                    if(dsock[b]!=-1) {
                          if(write(dsock[b],msgbuf,strlen(msgbuf))){
                            perror("socket write");
                      }
                    }           

                  }
        }
                 
       else{
              printf("%s is disconnected\n",clientuser[i]);
              close(dsock[i]);
              strcpy(clientuser[i],"");
              printf("dletate;client%s",clientuser[i]);
              for(int j=i;j<MAXUSER-1;j++){
                  dsock[j]=dsock[j+1];
                  strncpy(clientuser[j],clientuser[j+1],10);
              }
              k--;
        }

}



                

            
        }
        bzero(rbuf,1024);
    }while(1);
    

   

   


}
