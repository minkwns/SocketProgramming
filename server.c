/* Socket Programming with Webpages as Client */
/* Server.c */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <fcntl.h>

/* 각 filetype에 맞는 헤더 파일 제작, 요청 시 파일형식에 맞춰서 보내준다.*/

char header[] = "HTTP/1.1 200 OK\r\n"
                "Content-Type: text/html\r\n\r\n";

char gifheader[] = "HTTP/1.1 200 OK\r\n"
                   "Content-Type: image/gif\r\n\r\n";
                
char imgheader[] = "HTTP/1.1 200 OK\r\n"
                "Content-Type: image/jpeg\r\n\r\n";

char pdfheader[] = "HTTP/1.1 200 OK\r\n"
                "Content-Type: application/pdf\r\n\r\n";

char mp3header[] = "HTTP/1.1 200 OK\r\n"
                "Content-Type: audio/mpeg\r\n\r\n";

/*에러 처리 함수*/
void error(char *msg)
{
    perror(msg);
    exit(1);
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, fd; // descriptors rturn from socket and accept system calls
    int portno;                // port number
    socklen_t clilen;

    char buffer[1024];

    /*sockaddr_in: Structure Containing an Internet Address*/
    struct sockaddr_in serv_addr, cli_addr;

    int n;
    if (argc < 2) // 입력받을 때 요청을 잘못할 시 에러 반환 ex) port를 입력하지 않음
    {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    /* 소켓 생성 */
    sockfd = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // TCP 지향형이며 ipv4 도메인을 위한 소켓 생성
    if (sockfd < 0)
        error("ERROR opening socket");

    bzero((char *)&serv_addr, sizeof(serv_addr)); //주소를 초기화하고 IP주소를 지정하고 port 지정
    portno = atoi(argv[1]); // portno에 대한 것은 int여야 하기 때문에 string에서 Int형으로 변환
    serv_addr.sin_family = AF_INET; // 타입 ipv4
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // server는 계속 유지가 되어야 함 , ip주소
    serv_addr.sin_port = htons(portno);            // port

    /* 소켓과 서버 주소를 바인딩 */
    if (bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) // 소켓과 서버 주소를 바인딩
        error("ERROR on binding");
    /* listen(), 연결 대기열을 5개 생성 */
    listen(sockfd, 5);

    clilen = sizeof(cli_addr); // client의 주소의 길이
    int count = 1; // favicon 추가로 탐색하는 것을 막기 위한 변수 count 
    /* client에게 요청이 오면 요청 수락 */
    while (1) // 요청 계속 진행 가능 
    {
        newsockfd = accept(sockfd, (struct sockaddr *)&cli_addr, &clilen); // 요청 수락
        if (newsockfd < 0)
            error("ERROR on accept"); // 요청 에러 처리

        
        bzero(buffer, 1024); // 버퍼에 담긴 내용 초기화
        read(newsockfd, buffer, 1024); // 요청받은 메시지 필드 버퍼에 저장 
        printf("%s\n", buffer); // 로그에 출력
        /* GET 이하에 들어오는 요청받은 file Type 알아내는 작업 */
        char *token = strtok(buffer, " ");  // 버퍼에 들어온 내용에서 파일 타입에 관한 것만 잘라서 저장하는 작업
        token = strtok(NULL, " ") + 1; 
        char *temp = token; // 파일 타입을 저장 

        char *fileType=(char *)malloc(sizeof(char) * 30); // 파일 타입에 대해 동적 할당
        strcpy(fileType,temp); // token 변수 향후에 또 사용할 것이기 때문에 token에 들어간 요청 파일 fileType 변수에 복사 
        fileType = strtok(fileType, "."); // 파일 타입만 남기고 이름 제거
        fileType = strtok(NULL, "."); // 다시 요청을 받을 때 위와 동일하게 처리할 수 있도록 함

        /* 파일 타입에 대한 헤더 처리 */
        if(!strcmp(fileType,"html")){
            write(newsockfd, header, strlen(header));  // html 파일을 요청 받았을 때 html 헤더 전송
        }
        else if(!strcmp(fileType,"gif")){
            write(newsockfd, gifheader, strlen(gifheader)); // gif 파일을 요청 받았을 때 gif 헤더 전송
        }
        else if(!strcmp(fileType,"mp3")){
            write(newsockfd, mp3header, strlen(mp3header));  // mp3 파일을 요청 받았을 때 mp3 헤더 전송
        }
        else if(!strcmp(fileType,"pdf")){
            write(newsockfd, pdfheader, strlen(pdfheader));  // pdf 파일을 요청 받았을 때 pdf 헤더 전송
        }
        else if(!strcmp(fileType, "jpeg")){
            write(newsockfd, imgheader, strlen(imgheader));  // jpeg 파일을 요청 받았을 때 jpeg 헤더 전송
        }
        else if(!strcmp(fileType, "ico")){ // favicon은 favicon이 존재하지 않더라도 계속 요청을 한다. 그렇기 때문에 count라는 변수로 favicon의 출력을 최소화한다.
            if(count == 1){
                printf("%s\n", buffer);
                count = 0;
            }
            continue;
        }
        else 
            continue;

        FILE *fp;  
        if ((fp = fopen(token, "rb")) != NULL) // 요청받은 해당 파일(token에 담겨있음) 직접 읽는 역할 
        {
            int size = 0; 
            fseek(fp, 0, SEEK_END);
            size = ftell(fp);
            fseek(fp, 0, SEEK_SET);
            char *buf = (char *)malloc(sizeof(char) * size);
            fread(buf, 1, size, fp);
            write(newsockfd, buf, size);
        }
        fclose(fp); // fp 구조체 포인터 닫음

        close(newsockfd); // 소켓 close
    }
    close(sockfd); // 소켓 close, 서버 종료 

    return 0;
}
