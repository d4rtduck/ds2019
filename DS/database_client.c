
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

int finished = 0, waiting;

int isCharacter(char x);

void* send_msg(void* arg)
{
    int *values = (int*)arg;
    int serv = values[0];
    char s[100];

    while (1)
    {
        //printf("Finished = %d\n",finished);
        if (finished==1) break;
        printf("client>");
        fgets(s,100,stdin);
        write(serv, s, strlen(s) + 1);
    }
}

void* receive_msg(void* arg)
{
    int *values = (int*)arg;
    int serv = values[0];
    char s[100];

    // then it's server turn
    while(1)
    {
        if (finished==1) break;
        read(serv, s, sizeof(s));
        if (!isCharacter(s[0])) continue;
        printf("server says: %s\n", s);
        finished = 1;
    }

}

int main(int argc, char* argv[]) {
    int so;
    char s[100];
    struct sockaddr_in ad;

    socklen_t ad_length = sizeof(ad);
    struct hostent *hep;

    // create socket
    int serv = socket(AF_INET, SOCK_STREAM, 0);

    // init address
    hep = gethostbyname(argv[1]);
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    ad.sin_addr = *(struct in_addr *)hep->h_addr_list[0];
    ad.sin_port = htons(12345);

    // connect to server
    connect(serv, (struct sockaddr *)&ad, ad_length);

    pthread_t sending_thread, receiving_thread;
    int *arg = malloc(sizeof(int));
    arg[0] = serv;
    pthread_create(&sending_thread,NULL,send_msg,(void*)arg);
    pthread_create(&receiving_thread,NULL,receive_msg,(void*)arg);
    pthread_join(sending_thread,NULL);
    pthread_join(receiving_thread,NULL);

    return 0;
}

int isCharacter(char x)
{
    return (x>='A' && x<='Z');
}
