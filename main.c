#include "basic_comm.h"
#include "xmlget.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

pthread_mutex_t mutex;



void *inputThread(void *arg) {
  while (1) {
    char str[20] = {0};
    scanf("%19s", str);

    char *token = strtok(str, " ");

    if (strcmp(token, "auto") == 0) {
      pthread_mutex_lock(&mutex);
      char *shrRes = (char *)arg;
      *shrRes = 'a';
      printf("Mením nastavenie na automatické\n");
      pthread_mutex_unlock(&mutex);
      continue;
    }

    if (strcmp(token, "manual") == 0) {
      pthread_mutex_lock(&mutex);
      char *shrRes = (char *)arg;
      *shrRes = 'm';
      printf("Mením nastavenie na manuálne\n");
      pthread_mutex_unlock(&mutex);
      continue;
    }

    if (strcmp(token, "help") == 0) {
      printf("Príkazom auto sa automaticky zapne kontrola napätia každých 5s\n"
             "Príkazom manual sa táto funkcia vypne\n"
             "Príkazom exit sa program vypne\n");
      continue;
    }

    if (strcmp(token, "exit") == 0) {
      printf("Vypínam program...\n");

      pthread_mutex_lock(&mutex);
      char *shrRes = (char *)arg;
      *shrRes = 'e';
      pthread_mutex_unlock(&mutex);

      break;
    }

    printf("Neznámy príkaz. Použite help pre pomoc\n");
  }
  return NULL;
}

int openRelay(enum RelayNumber rn) {
  int socketFD;
  if (createClientSocket(ip_str, "192.168.1.101", 6722, &socketFD) != 0) {
    return EXIT_FAILURE;
  }

  if (sendDataToRelayServer(relay_open, rn, socketFD) != 0) {
    return EXIT_FAILURE;
  }

  printf("Relay opened successfully\n");
  closeConnection(socketFD);
  return EXIT_SUCCESS;
}

int closeRelay(enum RelayNumber rn) {
  int socketFD;
  if (createClientSocket(ip_str, "192.168.1.101", 6722, &socketFD) != 0) {
    return EXIT_FAILURE;
  }

  if (sendDataToRelayServer(relay_close, rn, socketFD) != 0) {
    return EXIT_FAILURE;
  }

  closeConnection(socketFD);
  return EXIT_SUCCESS;
}

int automaticControl(bool *relayOpen) {

  char *file = getXML();

  if (file == NULL) {
    return EXIT_FAILURE;
  }

  long value;

  if (XMLExtractValue(file, "inp1", &value) != 0) {
    return EXIT_FAILURE;
  }

  if (!*relayOpen && value * 2 >= 5420) {
    printf("Opening relay...\n");
    if (openRelay(relay_two) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }
    *relayOpen = true;
    return EXIT_SUCCESS;
  }

  if (*relayOpen && value * 2 <= 5350) {
    printf("Closing relay...\n");
    if (closeRelay(relay_two) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }
    *relayOpen = false;
    return EXIT_SUCCESS;
  }

  return EXIT_SUCCESS;
}

int main() {
  printf("Program automaticky kontroluje stav napäťia"
         "každých 5 sekúnd a v prípade nízkeho napäťia vypne relé 2\n"
         "Pre pomoc s príkazmi, napíšte help\n");

  pthread_t thread_id;

  char *shrRes = malloc(1);
  *shrRes = 'a';
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    fprintf(stderr, "Error creating mutex, aborting\n");
    return EXIT_FAILURE;
  }

  if (pthread_create(&thread_id, NULL, inputThread, shrRes) != 0) {
    fprintf(stderr, "Failed to create new thread\n");
    return EXIT_FAILURE;
  }

  bool automatic = true;
  bool relayOpen = false;
  bool continueRunning = true;

  while (continueRunning) {
    if (automatic) {

      if (automaticControl(&relayOpen) != EXIT_SUCCESS) {
        free(shrRes);
        return EXIT_FAILURE;
      }
    }

    pthread_mutex_lock(&mutex);

    switch (*shrRes) {
    case 'a':
      automatic = true;
      break;
    case 'm':
      automatic = false;
      break;
    case 'e':
      continueRunning = false;
      break;
    case 'f':
      free(shrRes);
      return EXIT_FAILURE;
    default:
      fprintf(stderr, "Unknown shared value, exiting\n");
      pthread_mutex_unlock(&mutex);
      pthread_mutex_destroy(&mutex);
      free(shrRes);
      return EXIT_FAILURE;
    }


    pthread_mutex_unlock(&mutex);
  }

  free(shrRes);
  pthread_join(thread_id, NULL);

  return 0;
}
