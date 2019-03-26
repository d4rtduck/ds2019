#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#include <pthread.h>
#include <unistd.h>
#include <stdbool.h>

struct Account
{
    char name[30];
    char dob[15];
    int id;
    float money;
};


const int SERVER_PORT = 20000;
const char* localHost = "127.0.0.1";

char* servers[10];
int server_ports[10];
int thisServerId;
int nServer = 1;
int clis[100000], nClient = 0;
pthread_t receiving_threads[10], sending_threads[10], client_threads[10];
int needSend[10];
int locked[100000], serverConfirmed[10000], nConfirmed;
char* queryToServer[50];

int nAccount = 4;
struct Account accounts[1000];

struct Account read_account(FILE *fp);
void write_account(struct Account account, FILE *fp);
void split(char *string, char *token[], char *tmp);
int char2int(char* arr);
int isCharacter(char x);

void init_server(int thisServerId)
{
    servers[0] = malloc(sizeof(char)*15);
    servers[0] = "192.168.1.69";
    server_ports[0] = 20000;

    servers[1] = malloc(sizeof(char)*15);
    servers[1] = "192.168.30.19";
    server_ports[1] = 20001;

    servers[2] = malloc(sizeof(char)*15);
    servers[2] = "192.168.12.35";
    server_ports[2] = 20002;


    FILE *fp = fopen("bank_database.dat","rb");
    for  (int i=1;i<=nAccount;i++) accounts[i] = read_account(fp);
    fclose(fp);

    for (int i=1;i<=nAccount;i++) {
        struct Account tmp = accounts[i];
        printf("%s\n%s\n%d\n%f\n\n",tmp.name,tmp.dob,tmp.id,tmp.money);
        printf("Account %d money = %f\n",i,accounts[i].money);
    }

    for (int i=0;i<=10;i++) queryToServer[i] = malloc(sizeof(char)*50);
}

int sendThreadWait = 0;

void* receiving_server_thread(void *arg)
{
    int *values = (int *)arg;
    int sourceServerId = values[0],scli = values[1];
    printf("%d %d %d A\n",thisServerId,sourceServerId,scli);
    char s[100];
    while(1)
    {
        read(scli, s, sizeof(s));
        if (!isCharacter(s[0])) continue;
        printf("Receive message from server number %d: %s\n",sourceServerId, s);


        char* tmp = malloc(sizeof(char)*30);
        char** token = malloc(sizeof(char)*30);;
        split(s,token,tmp);

        char* queryType = token[0];
        int id = char2int(token[1]);
        int amount = char2int(token[2]);
        printf("%s\n%s\n%s\n",token[0],token[1],token[2]);

        if (queryType[0]=='L' && queryType[1]=='O' && queryType[2]=='C') {
        printf("Before update money = %f\n",accounts[0].money);
            sendThreadWait = 1;

            printf("We got here\n");
            //queryToServer[sourceServerId] = (char*)"CONFIRM_LOCK";
            strcpy(queryToServer[sourceServerId],(char*)"CONFIRM_LOCK ");
            //printf("queryToServer = %s\n",queryToServer[sourceServerId]);
            printf("We got here 2\n");
            strcat((char*)queryToServer[sourceServerId],(char*)token[1]);
            printf("We got here 2.5\n");
            strcat(queryToServer[sourceServerId]," ");
            printf("We got here 3\n");
            strcat(queryToServer[sourceServerId],token[2]);
            printf("We got here 4\n");
            printf("Query to server = %s\n",queryToServer[sourceServerId]);
            locked[id] = 1;
            needSend[sourceServerId] = true;

            sendThreadWait = 0;
        }

        if (queryType[0]=='U' && queryType[1]=='P' && queryType[2]=='D') {
            sendThreadWait = 1;

            printf("We're before CONFIRM_UPDATE\n");
            strcpy(queryToServer[sourceServerId],(char*)"CONFIRM_UPDATE ");
            strcat(queryToServer[sourceServerId],token[1]); strcat(queryToServer[sourceServerId],(char*)" ");
            strcat(queryToServer[sourceServerId],token[2]);
            locked[id] = 0;
            printf("We're middle CONFIRM_UPDATE\n");
            for (int i=1;i<=nAccount;i++) if (accounts[i].id==id) {
                printf("We're adding money for account %d, money = %f\n",id,accounts[i].money);
                accounts[i].money += (float)amount;

                break;
            }
            printf("We're after CONFIRM_UPDATE\n");
            for (int i=1;i<=nAccount;i++) if (accounts[i].id==id) {
                printf("We're adding money = %f\n",accounts[i].money);
                printf("After transaction account %d has %f\n",id,accounts[i].money);
            }
            needSend[sourceServerId] = true;

            sendThreadWait = 0;
        }

        //if (queryType==(char*)"CONFIRM_LOCK" || queryType==(char*)"CONFIRM_UPDATE") nConfirmed++;
        if (queryType[0]=='C' && queryType[1]=='O' && queryType[2]=='N') nConfirmed++;
        free(tmp);
        free(token);
    }
}

void* sending_server_thread(void *arg)
{
    int *values = (int *)arg;
    int target_serverId = values[0], scli = values[1];
    printf("%d %d %d A\n",thisServerId,target_serverId,scli);

    while(1)
    {
        if (sendThreadWait) continue;
        if (needSend[target_serverId])
        {
            printf("This worked\n");
            needSend[target_serverId] = 0;
            char *server_msg = malloc(sizeof(char)*50);
            printf("Query to server ' = %s\n",queryToServer[target_serverId]);
            strcpy(server_msg,queryToServer[target_serverId]);
            printf("Server %d is sending: %s\n",thisServerId,server_msg);
            write(scli, server_msg, strlen(server_msg) + 1);
        }
    }
}

void* listening_client_thread(void *arg)
{
   printf("Before query money = %f\n",accounts[0].money);
    int *values = (int*)arg;
    int currentClient = values[0], cli = values[1];
    while (1) {
        char queryString[101];
        // it's client turn to chat, I wait and read message from client
        read(cli, queryString, sizeof(queryString));
        printf("client number %d says: %s\n",currentClient, queryString);

        char *tmp = malloc(sizeof(char)*20);;
        char** token = malloc(sizeof(char)*20);;
        split(queryString,token,tmp);

        char *queryType = token[0];
        int id = char2int(token[1]);
        int amount = char2int(token[2]);
        printf("%s\n%s\n%s\n",queryType,token[1],token[2]);

        //printf("Nserver = %d\n",nServer);
        // tell other server to lock
        nConfirmed = 0;
        for (int i=0;i<nServer;i++) if (i!=thisServerId) {
            printf("Arrived at %d\n",i);
            serverConfirmed[i] = 0;
            queryToServer[i] = malloc(sizeof(char) * 100);
            strcpy(queryToServer[i],(char*)"LOCK ");
            strcat(queryToServer[i],token[1]); strcat(queryToServer[i]," ");
            strcat(queryToServer[i],token[2]);
            needSend[i] = 1;
        }

        // wait until other servers confirm
        while (nConfirmed < nServer-1) {}//{printf("nConfirm = %d\n",nConfirmed);}
        printf("We almost success");

        int success = 0;
        for (int i=1;i<=nAccount;i++) if (id==accounts[i].id) {
            if (locked[i]) {success = 0; break;}
            if (accounts[i].money + amount < 0) {success = 0; break;}
            else {
                success = 1;
                accounts[i].money += (float)amount;
            }
            break;
        }

        nConfirmed = 0;
        for (int i=0;i<nServer;i++) if (i!=thisServerId) {
            printf("Arrived at %d\n",i);
            serverConfirmed[i] = 0;
            queryToServer[i] = malloc(sizeof(char) * 100);
            strcpy(queryToServer[i],(char*)"UPDATE ");
            strcat(queryToServer[i],token[1]); strcat(queryToServer[i]," ");
            strcat(queryToServer[i],token[2]);
            needSend[i] = 1;
        }
         while (nConfirmed < nServer-1) {} //{printf("nConfirm = %d\n",nConfirmed);}
         for (int i=1;i<=nAccount;i++) if (accounts[i].id==id) {
            printf("We're adding money for account %d, money = %f\n",id,accounts[i].money);
            printf("After transaction account %d has %f\n",id,accounts[i].money);
         }

         // now it's my (server) turn
        char *server_reply = malloc(sizeof(char)*50);
        if (success) strcpy(server_reply,(char*)"Transaction successful\n");
        else strcpy(server_reply,(char*)"Transaction failed\n");
        //strcat(server_reply,(char *)" server replied that");
        write(cli, server_reply, strlen(server_reply) + 1);
        free(server_reply);
    }
}

void* server_connection_thread(void *arg)
{
    char **argv = (char**)arg;

    int sss, scli, spid;

    struct sockaddr_in sad;

    socklen_t sad_length = sizeof(sad);
    sss = socket(AF_INET, SOCK_STREAM, 0);
    memset(&sad, 0, sizeof(sad));
    sad.sin_family = AF_INET;
    sad.sin_addr.s_addr = INADDR_ANY;
    sad.sin_port = htons(SERVER_PORT);
    bind(sss, (struct sockaddr *)&sad, sad_length);

    listen(sss, 0);

    if (thisServerId!=0)
    {
        for (int i=0;i<thisServerId;i++)
        {
            struct sockaddr_in mad;
            socklen_t mad_length = sizeof(mad);
            struct hostent *mhep;

            int mainServer = socket(AF_INET, SOCK_STREAM, 0);
        // init address
            mhep = gethostbyname(argv[2]);
            memset(&mad, 0, sizeof(mad));
            mad.sin_family = AF_INET;
            mad.sin_addr = *(struct in_addr *)mhep->h_addr_list[0];
            mad.sin_port = htons(SERVER_PORT);

            connect(mainServer, (struct sockaddr *)&mad, mad_length);
            int *arg = (int*)malloc(sizeof(int)*2);
            arg[0] = 0;
            arg[1] = mainServer;
            pthread_create(&receiving_threads[i],NULL,receiving_server_thread,(void*)arg);
            pthread_create(&sending_threads[i],NULL,sending_server_thread,(void*)arg);
        }
    }


    while(1)
    {
        scli = accept(sss, (struct sockaddr *)&sad, &sad_length);
        nServer++;
        printf("Current nServer = %d\n",nServer);
        printf("Server number %d connected\n",nServer-1);

        int *arg = (int*)malloc(sizeof(int)*2);
        arg[0] = nServer-1;
        arg[1] = scli;
        printf("%d %d B\n",nServer,scli);
        pthread_create(&receiving_threads[nServer-1],NULL,receiving_server_thread,(void*)arg);
        pthread_create(&sending_threads[nServer-1],NULL,sending_server_thread,(void*)arg);
    }
}

void* client_connection_thread(void *arg)
{
    int ss, cli, pid;
    struct sockaddr_in ad;

        // socket to clients
    socklen_t ad_length = sizeof(ad);
        // create the socket
    ss = socket(AF_INET, SOCK_STREAM, 0);

        // bind the socket to port 12345
    memset(&ad, 0, sizeof(ad));
    ad.sin_family = AF_INET;
    ad.sin_addr.s_addr = INADDR_ANY;
    ad.sin_port = htons(12345);
    bind(ss, (struct sockaddr *)&ad, ad_length);

        // then listen
    listen(ss, 0);


    while (1) {
        // an incoming connection
        cli = accept(ss, (struct sockaddr *)&ad, &ad_length);
        nClient++;

        int *arg = (int*)malloc(sizeof(int)*2);
        arg[0] = nClient;
        arg[1] = cli;

        pthread_create(&client_threads[nClient], NULL, listening_client_thread,(void*)arg);
    }


    // disconnect
    close(cli);
}


int main(int argc, char* argv[])
{
    sscanf(argv[1], "%d", &thisServerId);
    printf("Thisserverid = %d\n ",thisServerId);
    printf("GCC successful");
    init_server(thisServerId);

    pthread_t server_serving_thread, client_serving_thread;
    pthread_create(&server_serving_thread,NULL,server_connection_thread,(void*)argv);
    pthread_create(&client_serving_thread,NULL,client_connection_thread,(void*)NULL);
    pthread_join(server_serving_thread,NULL);
    pthread_join(client_serving_thread,NULL);

    return 0;
}












//
void write_account(struct Account account, FILE *fp)
{
    fwrite(account.name , sizeof(char)*30, 1, fp);
    fwrite(account.dob, sizeof(char)*15, 1, fp);
    fwrite((void*)(&account.id),sizeof(int),1,fp);
    fwrite((void*)(&account.money),sizeof(float),1,fp);
}

struct Account read_account(FILE *fp)
{
    struct Account res;
    char buffer[101];
    float v1; int newId;

    fread(buffer, sizeof(char)*30,1,fp);
    strncpy(res.name,buffer,30);
    fread(buffer, sizeof(char)*15,1,fp);
    strncpy(res.dob,buffer,15);
    fread((void*)(&newId),sizeof(newId),1,fp);
    res.id = newId;
    fread((void*)(&v1),sizeof(v1),1,fp);
    res.money = v1;

    return res;
}

int char2int(char *arr)
{
    int res = 0, l = strlen(arr);
    for (int i=0;i<l;i++) res = res*10  + arr[i] - '0';
    return res;
}

void split(char *string, char *token[], char* tmp)
{
    const char delimiters[] = " ";
    tmp = strdup(string);

    token[0] = strsep(&tmp, delimiters);    /* token => "header" */
    token[1] = strsep(&tmp, delimiters);    /* token => "dataA" */
    token[2] = strsep(&tmp, delimiters);    /* token => "dataB" */
}

int isCharacter(char x)
{
    return (x>='A' && x<='Z');
}
