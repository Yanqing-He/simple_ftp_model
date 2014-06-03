/*
 * my_client.c
 *
 *  Created on: 2013-2-21
 *      Author: hyq
 */

#include "my_client.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

int clientFunction(char* hostName)
{
	struct addrinfo hints, *res;
	int sockfd;
	int rv;
	char buffIn[256], buffOut[256];

	memset(&hints, 0, sizeof hints);
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;

	if((rv = getaddrinfo(hostName, "11125", &hints, &res)) != 0)
	{
		printf("getaddrinfo error:%s\n", gai_strerror(rv));
		exit(1);
	}

	if((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol)) == -1)
	{
		printf("error in creating socket\n");
		exit(1);
	}

	if((rv = connect(sockfd, res->ai_addr, res->ai_addrlen)) != 0)
	{
		printf("connect error\n");
		exit(1);
	}

	// put these into a loop
	memset(buffIn, 0, sizeof buffIn);
	memset(buffOut, 0, sizeof buffOut);


	// verify username & password
	while(1)
	{
//		char c;
//		int i = 0;

		recv(sockfd, buffIn, sizeof buffIn, 0);
		printf("%s", buffIn);
		gets(buffOut);
		if(strncmp(buffOut, "bye!", 4) == 0)
		{
			close(sockfd);
			exit(0);
		}
		send(sockfd, buffOut, sizeof buffOut, 0);
		memset(buffOut, 0, sizeof buffOut);
		memset(buffIn, 0, sizeof buffIn);
		recv(sockfd, buffIn, sizeof buffIn, 0);
		printf("%s", buffIn);
//		while((c = getch()) != 0x0d)
//		{
//			printf("*");
//			buffOut[i++] = c;
//		}
//		buffOut[i] = '\0';
		gets(buffOut);
		send(sockfd, buffOut, sizeof buffOut, 0);
		memset(buffOut, 0, sizeof buffOut);
		memset(buffIn, 0, sizeof buffIn);

		recv(sockfd, buffIn, sizeof buffIn, 0);
		if(strncmp(buffIn, "200OK", 5) == 0)
		{
			printf("log in successful\n");
			break;
		}
		else if(strncmp(buffIn, "102ERR", 6) == 0)
		{
			printf("username or password invalid, please try again\n");
			printf("type \"bye!\" in username to exit\n");
			continue;
		}

	}

	// verify username & password end

	printf("Connected with the server\n");
	printf("You can type \"bye!\" to exit this application\n");

	while (1)
	{
		memset(buffOut, 0, sizeof buffOut);
		memset(buffIn, 0, sizeof buffIn);
		gets(buffOut);
		if(strcmp(buffOut, "bye!") == 0)
		{
			if ((rv = send(sockfd, buffOut, sizeof buffOut, 0)) == -1)
					{
						printf("error in sending \"quit\"\n");
						exit(1);
					}
			break;
		}

		if(strncmp(buffOut, "put ", 4) == 0)
		{
			char temp[256];
			strcpy(temp, buffOut + 4);
			FILE *openFile;
			if((openFile = fopen(temp, "r")) == NULL)
			{
				printf("101ERR: No such file in current directory\n");
				continue;
			}
			send(sockfd, buffOut, sizeof buffOut, 0);
			recv(sockfd, buffIn, sizeof buffIn, 0);
			if(strncmp(buffIn, "200OK", 5) != 0)
			{
				printf("unknown error in server\n");
				memset(buffIn, 0, sizeof buffIn);
				continue;
			}
			memset(buffIn, 0, sizeof buffIn);

			printf("send # of chunks\n");
			long int fileLength, chunks;
			fileLength = fnFileLength(openFile);
			chunks = fileLength / 1024;
			if(fileLength % 1024 > 0)
				chunks++;
			snprintf(buffOut, sizeof buffOut, "%ld", chunks);
			send(sockfd, buffOut, sizeof buffOut, 0);
			memset(buffOut, 0, sizeof buffOut);

			recv(sockfd, buffIn, sizeof buffIn, 0);
			if(strncmp(buffIn, "200OK", 5) != 0)
			{
				printf("unknown error in server\n");
				continue;
			}
			// create connection
			struct addrinfo hintsTrans, *resTrans;
			int sockfdTrans;
			char buffPut[1024];

			memset(&hintsTrans, 0, sizeof hintsTrans);
			hintsTrans.ai_family = PF_INET;
			hintsTrans.ai_socktype = SOCK_STREAM;

			if((rv = getaddrinfo(hostName, "21125", &hintsTrans, &resTrans)) != 0)
			{
				printf("getaddrinfo error:%s\n", gai_strerror(rv));
				exit(1);
			}

			if((sockfdTrans = socket(resTrans->ai_family, resTrans->ai_socktype, resTrans->ai_protocol)) == -1)
			{
				printf("error in creating socket\n");
				exit(1);
			}

			if((rv = connect(sockfdTrans, resTrans->ai_addr, resTrans->ai_addrlen)) != 0)
			{
				printf("connect error\n");
				exit(1);
			}
			// finishing creating connection

			long int i;
			for(i = 0; i < chunks; i++)
			{
				rv = fread(buffPut, 1, 1024, openFile);
				//printf("%d read\n", rv);
				rv = send(sockfdTrans, buffPut, rv, 0);
				//printf("%d send\n", rv);
				memset(buffPut, 0, sizeof buffPut);
			}
			fclose(openFile);
			recv(sockfd, buffIn, sizeof buffIn, 0);
			if(strcmp(buffIn, "200OK") == 0)
			{
				close(sockfdTrans);
				printf("finish putting\n");
			}

			else
			{
				printf("error in receiving confirm messange\n");
				close(sockfdTrans);
				exit(1);
			}
			memset(buffIn, 0, sizeof buffIn);
			continue;

		}

		if(strncmp(buffOut, "get ", 4) == 0)
		{
			send(sockfd, buffOut, sizeof buffOut, 0);
			char fileName[256];
			strcpy(fileName, buffOut + 4);
			memset(buffOut, 0, sizeof buffOut);
			recv(sockfd, buffIn, sizeof buffIn, 0);
			if(strncmp(buffIn, "101ERR", 6) == 0)
			{
				printf("101ERR: No such file in current directory\n");
				continue;
			}
			else if(strncmp(buffIn, "200OK", 5) == 0)
			{
				memset(buffIn, 0, sizeof buffIn);
				recv(sockfd, buffIn, sizeof buffIn, 0);
				long int chunks = atoi(buffIn);
				memset(buffIn, 0, sizeof buffIn);

				recv(sockfd, buffIn, sizeof buffIn, 0);
				if(strncmp(buffIn, "200OK", 5) != 0)
				{
					printf("unknown error in server\n");
					continue;
				}

				// create connection
				struct addrinfo hintsTrans, *resTrans;
				int sockfdTrans;
				char buffGet[1024];

				memset(&hintsTrans, 0, sizeof hintsTrans);
				hintsTrans.ai_family = PF_INET;
				hintsTrans.ai_socktype = SOCK_STREAM;

				if((rv = getaddrinfo(hostName, "21125", &hintsTrans, &resTrans)) != 0)
				{
					printf("getaddrinfo error:%s\n", gai_strerror(rv));
					exit(1);
				}

				if((sockfdTrans = socket(resTrans->ai_family, resTrans->ai_socktype, resTrans->ai_protocol)) == -1)
				{
					printf("error in creating socket\n");
					exit(1);
				}

				if((rv = connect(sockfdTrans, resTrans->ai_addr, resTrans->ai_addrlen)) != 0)
				{
					printf("connect error\n");
					exit(1);
				}
				// finishing creating connection

				FILE *openFile;
				openFile = fopen(fileName, "w+");

				long int i;
				for(i = 0; i < chunks; i++)
				{
					rv = recv(sockfdTrans, buffGet, sizeof buffGet, 0);
					//printf("%d recved\n", rv);
					rv = fwrite(buffGet, 1, rv, openFile);
					//printf("%d write\n", rv);
					memset(buffGet, 0, sizeof buffGet);
				}
				fclose(openFile);
				strcpy(buffOut, "200OK");
				send(sockfd, buffOut, sizeof buffOut, 0);
				close(sockfdTrans);
				continue;
			}
			else
			{
				printf("unknown error\n");
				continue;
			}
		}

		if ((rv = send(sockfd, buffOut, sizeof buffOut, 0)) == -1)
		{
			printf("error in sending message\n");
			exit(1);
		}

		if ((rv = recv(sockfd, buffIn, sizeof buffIn, 0)) == -1)
		{
			printf("error in receiving message\n");
			exit(1);
		}

		printf("%s\n", buffIn);
	}

	// put above into a loop

	printf("client function operated\n");
	printf("ip is %s\n", hostName);

	close(sockfd);
	return 0;
}


long fnFileLength(FILE *fpstream)
{
   long int lCurpos, lLength;

   lCurpos = ftell(fpstream);
   fseek(fpstream, 0L, SEEK_END);
   lLength = ftell(fpstream);
   fseek(fpstream, lCurpos, SEEK_SET);
   return lLength;
}
