/*  Sample chat client for exploring the use of network functions in C
 *  Written by Jason Watts and John Rodkey, based on original concepts by Thomas
 * Cantrell
 *
 */

#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>

#define SERVER "10.115.20.250"
#define PORT 49153
#define BUFSIZE 1024

// prototyping the functions defined below main
int connect2v4stream(char *, int); 
int sendout(int, char *);
void recvandprint(int, char *);


int main(int argc, char *argv[]) {
  int fd, len;
  char *name, *buffer, *origbuffer;
  struct timeval timev;

  fd = connect2v4stream(SERVER, PORT);

  /* Setup recv timeout for .5 seconds */
  timev.tv_sec = 0;
  timev.tv_usec = 1000 * 500;
  setsockopt(fd, SOL_SOCKET, SO_RCVTIMEO, &timev, sizeof(timev));

  /* set name based on arguments */
  if (argc < 2) {
    printf("Usage: chat-client <screenname>\n");
    exit(1);
  }
  name = argv[1];
  len = strlen(name);
  name[len] = '\n';     /*insert new line*/
  name[len + 1] = '\0'; /* reterminate the string */
  sendout(fd, name);

  int is_done = 0;
  while (!is_done) {
    recvandprint(fd, buffer); /* print out any input from the socket */

    len = BUFSIZE;
    buffer = malloc(len + 1);
    origbuffer = buffer;
    if (getline(&buffer, (size_t *)&len, stdin) > 1) {
      /* getline returns 1 for "\n", -1 for error. and 0 for no input */
      sendout(fd, buffer);
    }

    is_done = (strcmp(buffer, "quit\n")) == 0;

    free(origbuffer);
  }
}

/* Create a TCP connection to a server on the designated port
 * return a file descripor to the socket */

int connect2v4stream(char *srv, int port) {
  int ret, sockd;
  struct sockaddr_in sin;

  if ((sockd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
    printf("ERROR: error creating socket, errno = %d\n", errno);
    exit(errno);
  }

  if ((ret = inet_pton(AF_INET, SERVER, &sin.sin_addr)) <= 0) {
    printf("ERROR: trouble converting using inet_pton. \
           return value = %d, errno = %d\n", ret, errno);
    exit(errno);
  }

  sin.sin_family = AF_INET;    // IPv4
  sin.sin_port = htons(PORT);  // Convert port to network endian

  if ((connect(sockd, (struct sockaddr *)&sin, sizeof(sin))) == -1) {
    printf("ERROR: trouble connecting to server, errno = %d\n", errno);
    exit(errno);
  }

  return sockd;
}

int sendout(int fd, char *msg) {
  int ret;
  send(fd, msg, strlen(msg), 0);

  if (ret == -1) {
    printf("ERROR: trouble sending, errno = %d\n", errno);
    exit(errno);
  }

  return strlen(msg);
}

/* Receive and print strings from a socket to stdout
 * until there is no more input on the socket */

void recvandprint(int fd, char *buff) {
  int ret;

  for (;;) {
    buff = malloc(BUFSIZE + 1);
    ret = recv(fd, buff, BUFSIZE, 0);
    if (ret == -1) {
      if (errno == EAGAIN) {
        break;  // go do it again
      } else {
        printf("ERROR: error receiving, errno = %d\n", errno);
        exit(errno);
      }
    } else if (ret == 0) {
      exit(0);  // graceful shutdown
    } else {
      buff[ret] = 0;
      printf("%s", buff);
    }

    free(buff);
  }
}

