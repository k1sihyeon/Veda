#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <pthread.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

static void* clnt_connection(void* arg);
int sendData(FILE* fp, char* ct, char* filename);
void sendOk(FILE* fp);
void sendError(FILE* fp);

int main(int argc, char** argv) {
    int ssock;
    pthread_t thread;
    struct sockaddr_in servaddr, cliaddr;
    unsigned int len;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        return -1;
    }

    // 소켓 생성
    ssock = socket(AF_INET, SOCK_STREAM, 0);
    if (ssock == -1) {
        perror("socket()");
        return -1;
    }

    // servaddr, port bind
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(atoi(argv[1]));
    if (bind(ssock, (struct sockaddr*)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind()");
        return -1;
    }

    // listen -> 대기 큐 설정
    if (listen(ssock, 10) == -1) {
        perror("listen()");
        return -1;
    }

    while (1) {
        char mesg[BUFSIZ];
        int csock;

        len = sizeof(cliaddr);
        csock = accept(ssock, (struct sockaddr*)&cliaddr, &len);
        // client 연결 요청 대기

        inet_ntop(AF_INET, &cliaddr.sin_addr, mesg, BUFSIZ);
        printf("Client IP : %s:%d\n", mesg, ntohs(cliaddr.sin_port));
        
        pthread_create(&thread, NULL, clnt_connection, &csock);
        //pthread_join(thread, NULL); // 연속적 클라이언트 처리
        pthread_detach(thread);
    }

    return 0;
}


void* clnt_connection(void* arg) {
    // 스레드 매개변수 arg를 int형 fd로 변환
    int csock = *((int *) arg);

    FILE *clnt_read, *clnt_write;
    char reg_line[BUFSIZ], reg_buf[BUFSIZ];
    char method[BUFSIZ], type[BUFSIZ];
    char filename[BUFSIZ], *ret;

    // fd를 FILE 스트림으로 변환
    clnt_read = fdopen(csock, "r");
    clnt_write = fdopen(dup(csock), "w");

    // clnt_read 읽어서 reg_line에 저장
    fgets(reg_line, BUFSIZ, clnt_read);
    fputs(reg_line, stdout);

    // reg_line을 token parsing 해서 method에 저장
    ret = strtok(reg_line, "/ ");
    strcpy(method, (ret != NULL) ? ret : "");

    // Post method 일 때
    if (strcmp(method, "POST") == 0) {
        sendOk(clnt_write);
        goto END;
    }
    // GET method가 아닐 때
    else if (strcmp(method, "GET") != 0) {
        sendError(clnt_write);
        goto END;
    }

    // request line에서 path 가져오기
    ret = strtok(NULL, " ");
    strcpy(filename, (ret != NULL) ? ret : "");

    // 경로가 슬래시로 시작하면 제거
    if (filename[0] == '/') {
        for (int i = 0, j = 0; i < BUFSIZ; i++, j++) {            
            filename[i] = filename[j];
            
            if (filename[j] == '\0') 
                break;
        }
    }

    /////////////// Host 확인
    //
    char host[BUFSIZ];
    char hostAddr[BUFSIZ];

    fgets(reg_line, BUFSIZ, clnt_read);

    ret = strtok(reg_line, " ");
    strcpy(host, (ret != NULL) ? ret : "");

    if (strcmp(host, "Host:") == 0) {
        ret = strtok(NULL, " ");
        strcpy(hostAddr, (ret != NULL) ? ret : "");
        printf("%s", hostAddr);
    }
    //
    ////////////////

    // // 메시지 헤더를 읽어서 화면에 출력, 나머지는 무시
    // do {
    //     fgets(reg_line, BUFSIZ, clnt_read);
    //     fputs(reg_line, stdout);

    //     strcpy(reg_buf, reg_line);
    //     char* buf = strchr(reg_buf, ':');
    // } while (strncmp(reg_line, "\r\n", 2));

    sendData(clnt_write, type, filename);

END:;
    fclose(clnt_read);
    fclose(clnt_write);

    pthread_exit(0);

    return (void *)NULL;
}

int sendData(FILE* fp, char* ct, char* filename) {
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[]   = "Server:Netscape-Enterprise/6.0\r\n";
    char cnt_type[] = "Content-Type:text/html\r\n";
    char end[]      = "\r\n";
    char buf[BUFSIZ];
    int fd, len;

    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_type, fp);
    fputs(end, fp);

    // 파일 열어서 전송
    fd = open(filename, O_RDWR);
    do {
        len = read(fd, buf, BUFSIZ);
        fputs(buf, fp);
    } while (len == BUFSIZ);

    close(fd);

    return 0;
}

void sendOk(FILE* fp) {
    // HTTP response - OK
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[]   = "Server: Netscape-Enterprise/6.0\r\n\r\n";

    fputs(protocol, fp);
    fputs(server, fp);

    fflush(fp);
}

void sendError(FILE* fp) {
    // HTTP response - error
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[]   = "Server: Netscape-Enterprise/6.0\r\n";
    char cnt_len[]  = "Content-Length:1024\r\n";
    char cnt_type[] = "Content-Type:text/html\r\n\r\n";

    // html 내용 
    char content1[] = "<html><head><title>BAD Connection</title></head>";
    char content2[] = "<body><font size=+5>BAD REQUEST</font></body></html>";

    printf("send_error\n");
    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);
    fputs(content1, fp);
    fputs(content2, fp);

    fflush(fp);
}