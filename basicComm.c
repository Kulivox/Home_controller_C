#include "basic_comm.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

static int setIP(enum IPVarType type, void *ip,
                 struct sockaddr_in *serverAddr) {
  if (type == ip_str) {
    int retOfConvert = inet_pton(AF_INET, ((char *)ip), &serverAddr->sin_addr);

    if (retOfConvert != 1) {
      fprintf(stderr, "Failed to convert ip to binary form\n");
      return 1;
    }
    return 0;
  }

  if (type == ip_int) {
    char *int_ip = (char *) ip;
    memcpy(&serverAddr->sin_addr, int_ip, 4);
    return 0;
  }

  return 1;
}

int createClientSocket(enum IPVarType type, void *ip, int port, int *sockFD) {
  *sockFD = socket(AF_INET, SOCK_STREAM, 0);

  if (sockFD < 0) {
    fprintf(stderr, "Could not obtain socket FD\n");
    return 1;
  }

  struct sockaddr_in serverAddr = {.sin_family = AF_INET,
                                   .sin_port = htons(port)};

  if (setIP(type, ip, &serverAddr) != 0) {
    return 1;
  }

  int connRet =
      connect(*sockFD, (struct sockaddr *)&serverAddr, sizeof(serverAddr));

  if (connRet < 0) {
    fprintf(stderr, "Could not connect to server\n");
    return 2;
  }

  return 0;
}

int sendDataToRelayServer(enum OpenOrClose oc, enum RelayNumber rn,
                          int sockFD) {
  char payload[2] = {0};
  payload[0] = (char) oc;
  payload[1] = (char) rn;

  int writtenBytes = write(sockFD, payload, 2);

  if (writtenBytes != 2) {
    fprintf(stderr, "Error sending data\n");
    return 1;
  }

#ifdef DEBUG
  char buffer[10] = {0};
  read(sockFD, buffer, 10);
  printf("RETURN: %s\n", buffer);
#endif
  return 0;
}


int readDataFromRelayServer(int sockFD, char **payload) {
  int writtenBytes = read(sockFD, payload, 100);

  if (writtenBytes < 0) {
    fprintf(stderr, "Error receiving data\n");
    return 1;
  }

  return 0;
}

int closeConnection(int socketFD) {
  int shutdownRet = shutdown(socketFD, SHUT_RDWR);

  if (shutdownRet != 0) {
    fprintf(stderr, "Problem with shutting down socket connection\n");
    return 1;
  }

  int closeRet = close(socketFD);

  if (closeRet == -1) {
    fprintf(stderr, "Could not close socket FD\n");
    return 1;
  }

  return 0;
}