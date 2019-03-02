
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>

int main(int argc, char* argv[]) {
	int so;
	struct sockaddr_in ad;
    char buff[8192];
	socklen_t ad_length = sizeof(ad);
	struct hostent *hep;

	// create socket
	int serv = socket(AF_INET, SOCK_STREAM, 0);

	// init address
	hep = gethostbyname(argv[1]);
	memset(&ad, 0, sizeof(ad));
	ad.sin_family = AF_INET;
	ad.sin_addr = *(struct in_addr *)hep->h_addr_list[0];
	ad.sin_port = htons(9001);

	// connect to server
	connect(serv, (struct sockaddr *)&ad, ad_length);
	int fileToBeSended = open("test.jpg", O_RDONLY);
	printf("%d\n", fileToBeSended);
	//send the name to the server
	//write(serv, argv[2], sizeof(argv[2]));

	//printf("2nd checkpoint");
	while(1){
		int n = read(fileToBeSended, buff, sizeof(buff));
		if (n <=0) break;
		write(serv, buff, n);	
	}
	close(fileToBeSended);
	return 0;



}

