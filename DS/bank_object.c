#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>


struct Account
{
    char name[30];
    char dob[15];
    int id;
    float money;
};

struct Account makeAccount(char *newName, char *newDob, int newId,float newMoney)
{
    struct Account res;
    strncpy(res.name,newName,30);
    strncpy(res.dob,newDob,15);
    res.id = newId;
    res.money = newMoney;

    return res;
}

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

int main()
{

    struct Account account1, account2, account3, account4;
    account1 = makeAccount((char*)"Le Huy Duc",(char*)"10/08/1998",100,(float)100);
    account2 = makeAccount((char*)"Le Huy Nam",(char*)"02/06/1998",110,(float)200);
    account3 = makeAccount((char*)"Ngoc Trung",(char*)"07/02/1998",120,(float)400);
    account4 = makeAccount((char*)"Huy HH Bui",(char*)"13/04/1998",150,(float)500);

    FILE *fp;

    /*
    float v = 100, v1;
    fp = fopen( "file.txt" , "wb");
    fwrite((void*)(&v), sizeof(v), 1, fp);
    fclose(fp);

    fp = fopen("file.txt","rb");
    fread((void*)(&v1), sizeof(v), 1, fp);
    printf("%f\n",v1);
    fclose(fp);
    */

    float v = 100, v1 = -10;

    fp = fopen("bank_database.dat" , "wb");
    write_account(account1,fp);
    write_account(account2,fp);
    write_account(account3,fp);
    write_account(account4,fp);
    fclose(fp);


    fp = fopen("bank_database.dat","rb");
    for (int i=1;i<=4;i++)
    {
        struct Account tmp = read_account(fp);
        printf("%s\n%s\n%d\n%f\n\n",tmp.name,tmp.dob,tmp.id,tmp.money);
    }
    fclose(fp);

    printf("Finished\n");

    return 0;
}


