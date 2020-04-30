//
// Created by Michal on 29-Apr-20.
//

#ifndef HOME_CONTROLLER_C_BASIC_COMM_H
#define HOME_CONTROLLER_C_BASIC_COMM_H

enum relayOperations {
  relay_open = 49,
  relay_close = 50,
  relay_stat = 51
};


enum RelayNumber {
  relay_one = 49,
  relay_two = 50,
  relay_three = 51,
  relay_four = 52,
  relay_five = 53,
  relay_six = 54,
  relay_seven = 55,
  relay_eight = 56
};

enum IPVarType {
  ip_int,
  ip_str
};

/*
 * This function creates TCP connection to server specified by ip and port in first two args
 * and returns writes socket File Descriptor to the third argument
 *
 * If the socket creation or ip string to number conversion fails, the function returns 1
 * If the server could not be reached, function returns 2
 *
 * If everything went fine, function returns 0
 *
 */

int createClientSocket(enum IPVarType type, void *ip, int port, int *sockFD);


/*
 * This function closes tcp connection and closes client socket file descriptor
 * If the shutdown or the close fail, function returns 1
 * If everything goes OK, function returns 0
 */

int closeConnection(int socketFD);

int sendDataToRelayServer(enum relayOperations oc, enum RelayNumber rn, int sockFD);

int readDataFromRelayServer(int sockFD, char *payload) ;


#endif // HOME_CONTROLLER_C_BASIC_COMM_H
