#include "basic_comm.h"
#include "xmlget.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifndef RELAY_IP
#define RELAY_IP "192.168.1.101"
#endif

#ifndef RELAY_PORT
#define RELAY_PORT 6722
#endif

pthread_mutex_t mutex;

int openRelay(enum RelayNumber rn) {
  int socketFD;
  if (createClientSocket(ip_str, RELAY_IP, RELAY_PORT, &socketFD) != 0) {
    return EXIT_FAILURE;
  }

  if (sendDataToRelayServer(relay_open, rn, socketFD) != 0) {
    return EXIT_FAILURE;
  }

  printf("Relé sa otvorilo úspešne\n");
  closeConnection(socketFD);
  return EXIT_SUCCESS;
}

int closeRelay(enum RelayNumber rn) {
  int socketFD;
  if (createClientSocket(ip_str, RELAY_IP, RELAY_PORT, &socketFD) != 0) {
    return EXIT_FAILURE;
  }

  if (sendDataToRelayServer(relay_close, rn, socketFD) != 0) {
    return EXIT_FAILURE;
  }
  printf("Relé sa zatvorilo úspešne\n");

  closeConnection(socketFD);
  return EXIT_SUCCESS;
}

int closeRelaysManually(char * message) {
  printf("Vypínam kúrenie a ventilátor %s\n", message);
  if (closeRelay(relay_one) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }
  if (closeRelay(relay_three) != EXIT_SUCCESS) {
    return EXIT_FAILURE;
  }
  return EXIT_SUCCESS;
}

int automaticControl(bool *relayOpen) {

  char *file = getXML();

  if (file == NULL) {
    closeRelaysManually(", lebo sa nedá pripojiť na zdroj dát o napätí\n");
    return EXIT_FAILURE;
  }

  long value;

  if (XMLExtractValue(file, "inp1", &value) != 0) {
    return EXIT_FAILURE;
  }

  if (!*relayOpen && value * 2 >= 5420) {
    printf("Napätie je %ld,%ld V, otváram relé...\n", (value * 2) / 100,
           (value * 2) % 100);
    if (openRelay(relay_one) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }
    printf("Zapínam ventilátor...\n");
    if (openRelay(relay_three) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }

    *relayOpen = true;
    return EXIT_SUCCESS;
  }

  if (*relayOpen && value * 2 <= 5310) {
    printf("Napätie je %ld,%ld V, zatváram relé...\n", (value * 2) / 100,
           (value * 2) % 100);
    if (closeRelay(relay_one) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }
    printf("Vypínam ventilátor...\n");
    if (closeRelay(relay_three) != EXIT_SUCCESS) {
      return EXIT_FAILURE;
    }

    *relayOpen = false;
    return EXIT_SUCCESS;
  }

  return EXIT_SUCCESS;
}

int getStat(bool automatic, bool correctTime) {
  char *xml = getXML();
  long value;
  XMLExtractValue(xml, "inp1", &value);

  int socketFD;
  if (createClientSocket(ip_str, RELAY_IP, RELAY_PORT, &socketFD) != 0) {
    return 1;
  }

  char buff[9] = {0};
  sendDataToRelayServer(relay_stat, relay_one, socketFD);
  readDataFromRelayServer(socketFD, buff);
  printf("Napätie je: %ld,%ld V\n", (value * 2) / 100, (value * 2) % 100);
  for (int i = 0; i < 8; i++) {
    printf("Relé %d je ", i + 1);

    if (buff[i] == '1') {
      printf("OTVORENÉ\n");
    } else {
      printf("ZATVORENÉ\n");
    }
  }

  if (automatic) {
    printf("\nMód: Automatický\n");
  }else {
    printf("\nMód: Manuálny\n");
  }

  if (automatic && !correctTime) {
    printf("Kúrenie a ventilátor sú automaticky vypnuté kvôli zlému času\n");
  }

  closeConnection(socketFD);

  return 0;
}

int setTimeConstraints(int *defaultStart, int *defaultEnd) {
  *defaultStart = 8;
  *defaultEnd = 16;

  FILE *vals = fopen("config.txt", "r");
  if (vals != NULL) {
    fscanf(vals, "%d %d", defaultStart, defaultEnd);
  }
  return 0;
}

bool isCorrecTimeToRun(int start, int end) {
  time_t rawtime;
  struct tm *timeinfo;

  time(&rawtime);
  timeinfo = localtime(&rawtime);

  return timeinfo->tm_hour >= start && timeinfo->tm_hour < end;
}

bool correctToken(char *token, long *val) {

  if (token == NULL) {
    return false;
  }

  char *end;
  long value = strtol(token, &end, 10);

  if (end == NULL) {
    return false;
  }

  if (value < 1 || value > 8) {
    return false;
  }

  *val = value;
  return true;
}

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
             "Príkazom exit sa program vypne\n"
             "Príkazom stat sa vypíše napätie na baterke a stav relátiek\n");
      continue;
    }

    if (strcmp(token, "stat") == 0) {
      pthread_mutex_lock(&mutex);
      char *shrRes = (char *)arg;
      *(shrRes + 1) = *shrRes;
      *shrRes = 's';
      pthread_mutex_unlock(&mutex);

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
    pthread_mutex_lock(&mutex);
    char *shrRes = (char *)arg;
    if (*shrRes == 'm' && strcmp(token, "open") == 0) {
      long val;
      scanf("%s", token);
      if (correctToken(token, &val)) {
        *shrRes = 'o';
        *(shrRes + 1) = 'm';
        *(shrRes + 2) = (char)val;
        pthread_mutex_unlock(&mutex);
        continue;
      }
    }

    if (*shrRes == 'm' && strcmp(token, "close") == 0) {
      long val;
      scanf("%s", token);
      if (correctToken(token, &val)) {
        *shrRes = 'c';
        *(shrRes + 1) = 'm';
        *(shrRes + 2) = (char)val;
        pthread_mutex_unlock(&mutex);
        continue;
      }
    }
    pthread_mutex_unlock(&mutex);
    printf("Neznámy príkaz. Použite help pre pomoc\n");
  }
  return NULL;
}

int main() {
  printf("Program automaticky kontroluje stav napäťia"
         "každých 5 sekúnd a v prípade nízkeho napäťia vypne relé 1\n"
         "Pre pomoc s príkazmi, napíšte help\n");

  pthread_t thread_id;

  char *shrRes = malloc(3);
  *shrRes = 'a';
  if (pthread_mutex_init(&mutex, NULL) != 0) {
    fprintf(stderr, "Error creating mutex, aborting\n");
    return EXIT_FAILURE;
  }

  if (pthread_create(&thread_id, NULL, inputThread, shrRes) != 0) {
    fprintf(stderr, "Failed to create new thread\n");
    return EXIT_FAILURE;
  }

  int start;
  int end;

  setTimeConstraints(&start, &end);

  bool automatic = true;
  bool relayOpen = false;
  bool continueRunning = true;

  while (continueRunning) {
    if (automatic) {
      bool correctTimeToRun = isCorrecTimeToRun(start, end);

      if (!correctTimeToRun && relayOpen) {
        closeRelaysManually("kvôli nesprávnemu času");
        relayOpen = false;
      }

      if (correctTimeToRun && automaticControl(&relayOpen) != EXIT_SUCCESS) {
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
      if (closeRelaysManually(", ukončujem program") != 0) {
        return EXIT_FAILURE;
      }
      break;
    case 's':
      *shrRes = *(shrRes + 1);
      if (getStat(automatic, isCorrecTimeToRun(start, end))) {
        free(shrRes);
        return EXIT_FAILURE;
      }
      break;
    case 'o':
      *shrRes = *(shrRes + 1);
      relayOpen = true;
      if (openRelay(*(shrRes + 2) + 48) != 0) {
        return EXIT_FAILURE;
      }
      break;
    case 'c':
      *shrRes = *(shrRes + 1);
      if (closeRelay(*(shrRes + 2) + 48) != 0) {
        return EXIT_FAILURE;
      }
      break;
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
