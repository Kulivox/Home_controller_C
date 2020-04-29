//
// Created by Michal on 29-Apr-20.
//

#ifndef HOME_CONTROLLER_C_RELLAY_COMM_H
#define HOME_CONTROLLER_C_RELLAY_COMM_H

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

int createClientSocket(char *ip, int port, int *sockFD);


/*
 * This function closes tcp connection and closes client socket file descriptor
 * If the shutdown or the close fail, function returns 1
 * If everything goes OK, function returns 0
 */

int closeConnection(int socketFD);

int sendDataToServer(char * data, int sockFD);


#endif // HOME_CONTROLLER_C_RELLAY_COMM_H
