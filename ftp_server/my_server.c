/*
 * my_server.c
 *
 *  Created on: 2013-2-21
 *      Author: hyq
 */

#include "my_server.h"
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

int serverFunction()
{
	int yes = 1;
	struct addrinfo hints, *res;
	int sockfd, sockfd_new;
	int rv;
	char buffIn[256], buffOut[256];
	char *buffErr = "please input valid function";
	struct sockaddr_storage remote_addr;
	socklen_t addr_size;

	memset(&hints, 0, sizeof hints);
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	if ((rv = getaddrinfo(NULL, "11125", &hints, &res)) != 0)
	{
		printf("getaddrinfo error:%s\n", gai_strerror(rv));
		exit(1);
	}

	if ((sockfd = socket(res->ai_family, res->ai_socktype, res->ai_protocol))
			== -1)
	{
		printf("error in creating socket");
		exit(1);
	}

	if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
			== -1)
	{
		perror("setsockopt");
		exit(1);
	}

	if ((rv = bind(sockfd, res->ai_addr, res->ai_addrlen)) != 0)
	{
		printf("error in binding socket\n");
		printf("error number is %d", errno);
		exit(1);
	}

	if ((rv = listen(sockfd, 3)) != 0)
	{
		printf("error in listening\n");
		printf("error number is %d", errno);
		exit(1);
	}
	printf("start listening\n");

	// put these into a loop
	while (1)
	{
		addr_size = sizeof remote_addr;
		sockfd_new = accept(sockfd, (struct sockaddr*) &remote_addr,
				&addr_size);

		if (fork() == 0)
		{
			close(sockfd);

			// verify username & password
			while(1)
			{
				char username[256];
				char password[256];
				char passBuff[256];
				char fileName[512];
				FILE* openFile;
				int length;


				memset(username, 0, sizeof username);
				memset(password, 0, sizeof password);
				memset(passBuff, 0, sizeof passBuff);
				memset(fileName, 0, sizeof fileName);
				strcpy(fileName, "./username/");

				strcpy(buffOut, "Please input:\nUsername: ");
				send(sockfd_new, buffOut, sizeof buffOut, 0);
				recv(sockfd_new, buffIn, sizeof buffIn, 0);
				if(strncmp(buffIn, "bye!", 4) == 0)
				{
					close(sockfd_new);
					exit(0);
				}
				strcpy(username, buffIn);
				strcat(fileName, buffIn);
				memset(buffIn, 0, sizeof buffIn);
				memset(buffOut, 0, sizeof buffOut);

				strcpy(buffOut, "Password: ");
				send(sockfd_new, buffOut, sizeof buffOut, 0);
				recv(sockfd_new, buffIn, sizeof buffIn, 0);
				strcpy(password, buffIn);
				memset(buffIn, 0, sizeof buffIn);
				memset(buffOut, 0, sizeof buffOut);

				if((openFile = fopen(fileName, "r")) != NULL)
				{
					printf("username valid\n");
					length = fread(passBuff, 1, 512, openFile);
					printf("length is %d\n", length);
					if(strncmp(password, passBuff, length - 1) == 0)
					{
						strcpy(buffOut, "200OK");
						send(sockfd_new, buffOut, sizeof buffOut, 0);
						fclose(openFile);
						printf("password valid\n");
						break;
					}
					printf("password invalid\n");
					fclose(openFile);
				}

				strcpy(buffOut, "102ERR");
				send(sockfd_new, buffOut, sizeof buffOut, 0);
				continue;
			}

			// verify username & password end

			printf("a user logged in\n");

			while (1)
			{
				memset(buffIn, 0, sizeof buffIn);
				memset(buffOut, 0, sizeof buffOut);


				if ((rv = recv(sockfd_new, buffIn, sizeof buffIn, 0)) == -1)
				{
					printf("error in receiving message\n");
					exit(1);
				}

				printf("\"%s\" received\n", buffIn);

				if (strncmp(buffIn, "echo ", 5) == 0)
				{
					strncpy(buffOut, buffIn + 5, strlen(buffIn) - 5);
				}
				else if (strcmp(buffIn, "date") == 0)
				{
					time_t *timep;
					timep = malloc(sizeof(time_t));
					time(timep);
					strcpy((char*) buffOut, ctime(timep));
				}
				else if (strncmp(buffIn, "upper ", 6) == 0)
				{
					int i;
					strcpy((char*) buffOut, buffIn + 6);
					for (i = 0; i < strlen(buffOut); i++)
					{
						if (buffOut[i] >= 0x61 && buffOut[i] <= 0x7A)
						{
							buffOut[i] = buffOut[i] - 0x20;
						}
					}
				}
				else if (strncmp(buffIn, "lower ", 6) == 0)
				{
					int i;
					strcpy((char*) buffOut, buffIn + 6);
					for (i = 0; i < strlen(buffOut); i++)
					{
						if (buffOut[i] <= 0x5A && buffOut[i] >= 0x41)
						{
							buffOut[i] = buffOut[i] + 0x20;
						}
					}
				}
				else if (strncmp(buffIn, "reverse ", 8) == 0)
				{
					int m, n;
					char temp[256];
					strcpy(temp, buffIn + 8);
					n = strlen(temp);
					buffOut[n] = '\0';
					for (m = 0; n > 0; n--, m++)
					{
						buffOut[m] = temp[n - 1];
					}
				}
				else if(strcmp(buffIn, "bye!") == 0)
				{
					close(sockfd_new);
					exit(0);
				}
				else if(strncmp(buffIn, "cd ", 3) == 0)
				{
					char path[512];
					strcpy(path, buffIn + 3);
					if(chdir(path) == -1)
					{
						strcpy(buffOut, "Cannot change current directory\n");
						send(sockfd_new, buffOut, sizeof buffOut, 0);
						continue;
					}
					strcpy(buffOut, "Current dir is: ");
					strcat(buffOut, getcwd(path, 512));
					send(sockfd_new, buffOut, sizeof buffOut, 0);
					continue;
				}
				else if(strncmp(buffIn, "put ", 4) == 0)
				{
					char fileName[256];
					memset(fileName, 0, sizeof fileName);
					strcpy(fileName, buffIn + 4);
					FILE* openFile;
					openFile = fopen(fileName, "w+");
					strcpy(buffOut, "200OK");
					send(sockfd_new, buffOut, sizeof buffOut, 0);
					memset(buffOut, 0, sizeof buffOut);

					recv(sockfd_new, buffIn, sizeof buffIn, 0);
					long int chunks = atoi(buffIn);
					memset(buffIn, 0, sizeof buffIn);

					printf("create data socket\n");
					//create data socket
					struct addrinfo hintsTrans, *resTrans;
					int sockfdTrans, sockfd_newTrans;
					char buffGet[1024];
					struct sockaddr_storage remote_addrTrans;
					socklen_t addr_sizeTrans;

					memset(&hintsTrans, 0, sizeof hintsTrans);
					hintsTrans.ai_family = PF_INET;
					hintsTrans.ai_socktype = SOCK_STREAM;
					hintsTrans.ai_flags = AI_PASSIVE;

					if ((rv = getaddrinfo(NULL, "21125", &hintsTrans, &resTrans)) != 0)
					{
						printf("getaddrinfo error:%s\n", gai_strerror(rv));
						exit(1);
					}

					if ((sockfdTrans = socket(resTrans->ai_family, resTrans->ai_socktype, resTrans->ai_protocol))
							== -1)
					{
						printf("error in creating socket");
						exit(1);
					}

					if (setsockopt(sockfdTrans, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
							== -1)
					{
						perror("setsockopt");
						exit(1);
					}

					if ((rv = bind(sockfdTrans, resTrans->ai_addr, resTrans->ai_addrlen)) != 0)
					{
						printf("error in binding socket\n");
						printf("error number is %d", errno);
						exit(1);
					}

					if ((rv = listen(sockfdTrans, 3)) != 0)
					{
						printf("error in listening\n");
						printf("error number is %d", errno);
						exit(1);
					}
					strcpy(buffOut, "200OK");
					send(sockfd_new, buffOut, sizeof buffOut, 0);
					addr_size = sizeof remote_addrTrans;
					sockfd_newTrans = accept(sockfdTrans,
							(struct sockaddr*) &remote_addrTrans,
							&addr_sizeTrans);
					//finish creating data socket

					long int i;
					for(i = 0; i < chunks; i++)
					{
						rv = recv(sockfd_newTrans, buffGet, sizeof buffGet, 0);
						rv = fwrite(buffGet, 1, rv, openFile);
						memset(buffGet, 0, sizeof buffGet);
					}
					fclose(openFile);
					strcpy(buffOut, "200OK");
					send(sockfd_new, buffOut, sizeof buffOut, 0);
					close(sockfd_newTrans);
					close(sockfdTrans);
					continue;
				}
				else if(strncmp(buffIn, "get ", 4) == 0)
				{
					char temp[256];
					strcpy(temp, buffIn + 4);
					FILE *openFile;
					if((openFile = fopen(temp, "r")) == NULL)
					{
						strcpy(buffOut, "101ERR: No such file in current directory");
						send(sockfd_new, buffOut, sizeof buffOut, 0);
						printf("\"%s\" sent\n", buffOut);
						memset(buffOut, 0, sizeof buffOut);
						continue;
					}
					strcpy(buffOut, "200OK");
					send(sockfd_new, buffOut, sizeof buffOut, 0);
					printf("\"%s\" sent\n", buffOut);
					memset(buffOut, 0, sizeof buffOut);

					long int fileLength, chunks;
					fileLength = fnFileLength(openFile);
					chunks = fileLength / 1024;
					if(fileLength % 1024 > 0)
						chunks++;
					snprintf(buffOut, sizeof buffOut, "%ld", chunks);
					send(sockfd_new, buffOut, sizeof buffOut, 0);
					memset(buffOut, 0, sizeof buffOut);

					//create new socket
					struct addrinfo hintsTrans, *resTrans;
					int sockfdTrans, sockfd_newTrans;
					int rv;
					char buffPut[1024];
					struct sockaddr_storage remote_addrTrans;
					socklen_t addr_sizeTrans;
					memset(buffPut, 0, sizeof buffPut);

					memset(&hintsTrans, 0, sizeof hintsTrans);
					hintsTrans.ai_family = PF_INET;
					hintsTrans.ai_socktype = SOCK_STREAM;
					hintsTrans.ai_flags = AI_PASSIVE;

					if ((rv = getaddrinfo(NULL, "21125", &hintsTrans, &resTrans)) != 0)
					{
						printf("getaddrinfo error:%s\n", gai_strerror(rv));
						exit(1);
					}

					if ((sockfdTrans = socket(resTrans->ai_family, resTrans->ai_socktype, resTrans->ai_protocol))
							== -1)
					{
						printf("error in creating socket");
						exit(1);
					}

					if (setsockopt(sockfdTrans, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int))
							== -1)
					{
						perror("setsockopt");
						exit(1);
					}

					if ((rv = bind(sockfdTrans, resTrans->ai_addr, resTrans->ai_addrlen)) != 0)
					{
						printf("error in binding socket\n");
						printf("error number is %d", errno);
						exit(1);
					}

					if ((rv = listen(sockfdTrans, 3)) != 0)
					{
						printf("error in listening\n");
						printf("error number is %d", errno);
						exit(1);
					}
					strcpy(buffOut, "200OK");
					send(sockfd_new, buffOut, sizeof buffOut, 0);

					addr_sizeTrans = sizeof remote_addr;
					sockfd_newTrans = accept(sockfdTrans,
							(struct sockaddr*) &remote_addrTrans,
							&addr_sizeTrans);
					//finish creating new socket

					int i;
					for(i = 0; i < chunks; i++)
					{
						rv = fread(buffPut, 1, 1024, openFile);
						//printf("%d read\n", rv);
						rv = send(sockfd_newTrans, buffPut, rv, 0);
						//printf("%d send\n", rv);
						memset(buffPut, 0, sizeof buffPut);
					}
					fclose(openFile);
					recv(sockfd_new, buffIn, sizeof buffIn, 0);
					if(strcmp(buffIn, "200OK") == 0)
						close(sockfd_newTrans);
					else
					{
						printf("error in receiving confirm messange\n");
						close(sockfd_newTrans);
						exit(1);
					}
					memset(buffIn, 0, sizeof buffIn);
					printf("finishing file transmitting\n");
					close(sockfdTrans);
					continue;

				}
				else
				{
					strcpy((char*) buffOut, buffErr);
				}

				if ((rv = send(sockfd_new, buffOut, sizeof buffOut, 0)) == -1)
				{
					printf("error in sending message\n");
					exit(1);
				}
				printf("\"%s\" sent\n", buffOut);
			}
		}
	}
	// put above into a loop
	printf("server function operated\n");
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
