#include <wiringPi.h>
#include <softTone.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <stdbool.h>
#include <pthread.h>

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#define SW      5   // gpio 24
#define CDS     0   // gpio 17
#define LED     1   // gpio 18
#define SPKR    6   // gpio 25
#define MOTOR   3

static bool alram = false;  // 알람 설정 flag
static bool isRun = true;   // 스레드 종료 flag
static bool isLight = false; // led 켜짐?

int notes[] = {
    261, 329, 392, 440, 0,         // C4, E4, G4, A4, 쉼표
    392, 329, 293, 261, 0,         // G4, E4, D4, C4, 쉼표
    261, 329, 392, 440, 0,         // C4, E4, G4, A4, 쉼표
    392, 329, 293, 261, 0,         // G4, E4, D4, C4, 쉼표
    261, 293, 329, 392, 523, 0,    // C4, D4, E4, G4, C5, 쉼표
    523, 392, 329, 293, 261, 0     // C5, G4, E4, D4, C4, 쉼표
};

void* webserverFunc(void* arg);
static void *clnt_connection(void* arg);
int sendData(FILE* fp, char* ct, char* filename);
void sendOk(FILE* fp);
void sendError(FILE* fp);

void* switchFunc(void* arg);
void* lightFunc(void* arg);

void sigfunc(int signo);
int musicPlay();


int main(int argc, char** argv) {
    pthread_t ptLight, ptSwitch, ptWebserver;

    if (argc != 2) {
        printf("Usage : %s <port>\n", argv[0]);
        return -1;
    }

    signal(SIGINT, sigfunc);
    wiringPiSetup();
    
    pinMode(SW, INPUT);
    pinMode(CDS, INPUT);
    pinMode(LED, OUTPUT);

    pthread_create(&ptLight, NULL, lightFunc, NULL);
    pthread_create(&ptSwitch, NULL, switchFunc, NULL);
    pthread_create(&ptWebserver, NULL, webserverFunc, (void *)(atoi(argv[1])));

    printf("q : Quit\n");
    while (isRun) {
        if (getchar() == 'q') {
            pthread_kill(ptWebserver, SIGINT);
            pthread_cancel(ptWebserver);
            sigfunc(0);
            break;
        }
    }

    return 0;
}

void sigfunc(int signo) {
    isRun = false;

    delay(10);

    digitalWrite(LED, LOW);
    digitalWrite(MOTOR, LOW);
    softToneWrite(SPKR, 0);
    softToneStop(SPKR);    

    printf("goodbye!\n");

    delay(1000);
    exit(0);
}

int musicPlay() {
    int i;

    softToneCreate(SPKR);

    int length = sizeof(notes) / sizeof(notes[0]);

    for (i = 0; i < length; i++) {
        softToneWrite(SPKR, notes[i]);
        delay(280);
    }

    softToneWrite(SPKR, 0);
    softToneStop(SPKR);
    delay(10);

    return 0;
}


void *switchFunc(void* arg) {
    while (isRun) {
        if (digitalRead(SW) == LOW) {
            alram = !alram;

            printf("alram is %s\n", alram ? "true" : "false");

            if (alram == false) {
                softToneWrite(SPKR, 0);
                softToneStop(SPKR);
            }

            delay(1000);
        }
    }
}

void *lightFunc(void* arg) {
    while (isRun) {
        if (digitalRead(CDS) == LOW) {   
            pinMode(LED, OUTPUT);  
            isLight = true;
            digitalWrite(LED, HIGH);
            delay(10);
        }

        else if (digitalRead(CDS) == HIGH) {
            if (alram) {
                musicPlay();
            }
        }
    }
}

void *webserverFunc(void* arg) {
    int ssock;
    pthread_t thread;
    struct sockaddr_in servaddr, cliaddr;
    unsigned int len;

    int port = (int)arg;

    ssock = socket(AF_INET, SOCK_STREAM, 0);
    if (ssock == -1) {
        perror("socket()");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);
    if (bind(ssock, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
        perror("bind()");
        exit(1);
    }

    if (listen(ssock, 10) == -1) {
        perror("listen()");
        exit(1);
    }

    while (isRun) {
        char mesg[BUFSIZ];
        int csock;

        len = sizeof(cliaddr);
        csock = accept(ssock, (struct sockaddr *)&cliaddr, &len);
        inet_ntop(AF_INET, &cliaddr.sin_addr, mesg, BUFSIZ);
        printf("Client IP : %s:%d\n", mesg, ntohs(cliaddr.sin_port));

        pthread_create(&thread, NULL, clnt_connection, &csock);
        // 연속적인 클라이언트 처리
        // pthread_join(thread, NULL);
    }

    return 0;
}

void *clnt_connection(void* arg) {
    int csock = *((int *)arg);
    int clnt_fd;

    FILE *clnt_read, *clnt_write;
    char reg_line[BUFSIZ], reg_buf[BUFSIZ];
    char method[BUFSIZ], type[BUFSIZ];
    char filename[BUFSIZ], *ret;

    clnt_read = fdopen(csock, "r");
    clnt_write = fdopen(dup(csock), "w");

    fgets(reg_line, BUFSIZ, clnt_read);
    fputs(reg_line, stdout);

    ret = strtok(reg_line, "/ ");
    strcpy(method, (ret != NULL) ? ret : "");
    if (strcmp(method, "POST") == 0) {
        sendOk(clnt_write);
        goto END;
    }
    else if (strcmp(method, "GET") != 0) {
        sendError(clnt_write);
        goto END;
    }

    ret = strtok(NULL, " ");
    strcpy(filename, (ret != NULL) ? ret : "");

    if (filename[0] == '/') {
        for (int i = 0, j = 0; i < BUFSIZ; i++, j++) {
            filename[i] = filename[j];
            
            if (filename[j] == '\0')
                break;
        }
    }

    if (!strlen(filename))
        strcpy(filename, "index.html");
    
    if (strstr(filename, "?") != NULL) {
        char optLine[BUFSIZ];
        char optStr[32][BUFSIZ];
        char opt[BUFSIZ], var[BUFSIZ], *tok;
        int count = 0;

        ret = strtok(filename, "?");
        if (ret == NULL)
            goto END;
        strcpy(filename, ret);
        
        ret = strtok(NULL, "?");
        if (ret == NULL)
            goto END;
        strcpy(optLine, ret);

        tok = strtok(optLine, "&");
        while (tok != NULL) {
            strcpy(optStr[count++], tok);
            tok = strtok(NULL, "&");
        }

        printf("clnt_connection\n");

        for (int i = 0; i < count; i++) {
            strcpy(opt, strtok(optStr[i], "="));
            strcpy(var, strtok(NULL, "="));
            printf("%s = %s\n", opt, var);

            if (!strcmp(opt, "led")) {
                pinMode(LED, OUTPUT);

                if (!strcmp(var, "On")) {
                    digitalWrite(LED, HIGH);
                    delay(10);
                }
                else if (!strcmp(var, "Off")) {
                    digitalWrite(LED, LOW);
                    delay(10);
                }
            }
            else if (!strcmp(opt, "motor")) {
                digitalWrite(MOTOR, HIGH);
                
                if (!strcmp(var, "On")) {
                    digitalWrite(MOTOR, HIGH);
                }
                else if (!strcmp(var, "Off")) {
                    digitalWrite(MOTOR, LOW);
                }
            }
            else if (!strcmp(opt, "speaker")) {
                if (!strcmp(var, "On")) {
                    musicPlay();
                }
                else if (!strcmp(var, "Off")) {
                    softToneWrite(SPKR, 0);
                    softToneStop(SPKR);
                }
            }
        }
    }

    do {
        fgets(reg_line, BUFSIZ, clnt_read);
        fputs(reg_line, stdout);
        strcpy(reg_buf, reg_line);
        char* buf = strchr(reg_buf, ':');
    } while (strncmp(reg_line, "\r\n", 2));

    sendData(clnt_write, type, filename);

END:;
    fclose(clnt_read);
    fclose(clnt_write);

    pthread_exit(0);

    return (void *)NULL;
}

int sendData(FILE* fp, char* ct, char* filename) {
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n";
    char cnt_type[] = "Content-Type:text/html\r\n";
    char end[] = "\r\n";
    char html[BUFSIZ];

    double temperature, humidity;
    double t_c = 0.0;
    double pressure = 0.0;

    int cdsLight;

    pinMode(CDS, INPUT);
    cdsLight = digitalRead(CDS);

    // 장치 값


    sprintf(html, "<html><head><meta http-equiv=\"Content-Type\" " \
                  "content=\"text/html; charset=UTF-8\" />" \
                  "<title>Raspberry Pi Controller</title></head><body><table>" \
                  "<tr><td>LIGHT</td><td colspan=2>" \
                  "<input readonly name=\"light\"value=%s></td></tr></table>" \
                  
                  "<form action=\"index.html\" method=\"GET\" "\
                  "onSubmit=\"document.reload()\"><table>" \
                  "<tr><td>LED</td><td>" \
                  "<input type=radio name=\"led\" value=\"On\" checked=checked>On</td>" \
                  "<td><input type=radio name=\"led\" value=\"Off\">Off</td></tr>" \

                  "<tr><td>Motor</td><td>" \
                  "<input type=radio name=\"motor\" value=\"On\" checked=checked>On</td>" \
                  "<td><input type=radio name=\"motor\" value=\"Off\">Off</td></tr>" \
                  
                  "<tr><td>Speaker</td><td>" \
                  "<input type=radio name=\"speaker\" value=\"On\" checked=checked>On</td>" \
                  "<td><input type=radio name=\"speaker\" value=\"Off\">Off</td></tr>" \
                  
                  "<tr><td>Submit</td>" \
                  "<td colspan=2><input type=submit value=\"Submit\"></td></tr>" \
                  "</table></form></body></html>",
                  (cdsLight == HIGH) ? "OK" : "NO LIGHT"
            );
    
    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_type, fp);
    fputs(end, fp);
    fputs(html, fp);
    fflush(fp);

    return 0;   
}

void sendOk(FILE* fp) {
    char protocol[] = "HTTP/1.1 200 OK\r\n";
    char server[] = "Server:Netscape-Enterprise/6.0\r\n\r\n";

    fputs(protocol, fp);
    fputs(server, fp);
    fflush(fp);   
}

void sendError(FILE* fp) {
    char protocol[] = "HTTP/1.1 400 Bad Request\r\n";
    char server[] = "Server: Netscape-Enterprise/6.0\r\n";
    char cnt_len[] = "Content-Length:1024\r\n";
    char cnt_type[] = "Content-Type:text/html\r\n\r\n";

    char content1[] = "<html><head><title>BAD Connection</title></head>";
    char content2[] = "<body><font size=+5>Bad Request</font></body></html>";
    printf("send_error\n");

    fputs(protocol, fp);
    fputs(server, fp);
    fputs(cnt_len, fp);
    fputs(cnt_type, fp);
    fputs(content1, fp);
    fputs(content2, fp);
    fflush(fp);
}