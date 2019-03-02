#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
int main()
{
	int i;
	int ss, cli, pid;
	struct sockaddr_in ad;
	socklen_t ad_length = sizeof(ad);
	char buff[8192];

	// create the socket
	ss = socket(AF_INET, SOCK_STREAM, 0);

	// bind the socket to port 9001
	memset(&ad, 0, sizeof(ad));
	ad.sin_family = AF_INET;
	ad.sin_addr.s_addr = INADDR_ANY;
	ad.sin_port = htons(9001);
	bind(ss, (struct sockaddr *)&ad, ad_length);

	// then listen
	listen(ss, 0);

	while (1)
	{
		// an incoming connectionint n
		cli = accept(ss, (struct sockaddr *)&ad, &ad_length);

		pid = fork();
		if (pid == 0)
		{
			// I'm the son, I'll serve this client
			printf("client connected\n");
			int fileSended = open("fileReceived.jpg", O_CREAT | O_WRONLY);
			while (1)
			{
				int n = read(cli, buff, sizeof(buff));
				printf("n is :%d\n", n);
				if (n <= 0)
				{
					system("chmod 755 fileReceived.jpg");
					break;
				}
				write(fileSended, buff, n);
			}
			return 0;
		}
		else
		{
			// I'm the father, continue the loop to accept more clients
			continue;
		}
	}
	// disconnect
	close(cli);
	return 0;
}
