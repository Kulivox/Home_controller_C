#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>

enum OpenOrClose {
  relay_open = 49,
  relay_close = 50
};


enum RelayNumber {
    relay_zero,
    relay_one,
    relay_two,
    relay_three,
    relay_four,
    relay_five,
    relay_six,
    relay_seven
};




int createClientSocket(char *ip, int port, int *sockFD) {
  *sockFD = socket(AF_INET, SOCK_STREAM, 0);

  if (sockFD < 0) {
    fprintf(stderr, "Could not obtain socket FD\n");
    return 1;
  }

  struct sockaddr_in serverAddr = {.sin_family = AF_INET,
                                   .sin_port = htons(port)};

  int retOfConvert = inet_pton(AF_INET, ip, &serverAddr.sin_addr);

  if (retOfConvert != 1) {
    fprintf(stderr, "Failed to convert ip to binary form\n");
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

int sendDataToServer(enum OpenOrClose oc, enum RelayNumber rn, int sockFD) {
    char payload[2] = {0};
    payload[0] = (char) oc;
    payload[1] = (char) rn;

    int writtenBytes = write(sockFD, payload, 2);

    if (writtenBytes != 2) {
        fprintf(stderr, "Error sending data\n");

    }

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