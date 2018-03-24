#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>

#define MODEM "tiny9300"
#define PORT 3105

#define BUF_SIZE 4096

static char buf[BUF_SIZE];

int main(int argc, char *argv[])
{
  struct addrinfo hints, *res;
  struct sockaddr_in addr;
  int rsock, wsock;
  int len;
  int n;
  FILE *fp = stdin;
  int err;
  int pid;
  int status;
  char *modem;

  switch (argc) {
  case 1:
    modem = MODEM;
    break;
  case 2:
    modem = argv[1];
    break;
  default:
    fprintf(stderr, "usage: modem_send [host]\n");
    exit(1);
  }

  bzero(&hints, sizeof(hints));
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_family = AF_INET;

  err = getaddrinfo(modem, NULL, &hints, &res);
  if (err != 0) {
    fprintf(stderr, "getaddrinfo error: %d\n", err);
    exit(1);
  }

  addr.sin_addr.s_addr = ((struct sockaddr_in *)(res->ai_addr))->sin_addr.s_addr;
  addr.sin_family = AF_INET;
  addr.sin_port = htons(PORT);

  wsock = socket(AF_INET, SOCK_STREAM, 0);
  if (wsock < 0) {
    perror("socket");
    exit(1);
  }
  if (connect(wsock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(1);
  }

#if 0
  rsock = socket(AF_INET, SOCK_STREAM, 0);
  if (rsock < 0) {
    perror("socket");
    exit(1);
  }
  if (connect(rsock, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
    perror("connect");
    exit(1);
  }
#else
  rsock = wsock;
#endif

  pid = fork();
  if (pid < 0) {
    perror("fork");
    exit(1);
  }

  if (pid == 0) {
    /* write data to modem */
    while ((len = fread(buf, 1, BUF_SIZE, fp)) > 0) {
      n = write(wsock, buf, len);
      if (n != len) {
	fprintf(stderr, "write modem error: %d\n", n);
	break;
      }
    }
    fclose(fp);
    shutdown(wsock, 1); // close write side
    exit(0);
  }

  /* read dat from modem */
  while ((len = read(rsock, buf, BUF_SIZE)) > 0) {
    n = fwrite(buf, 1, len, stdout);
    if (n != len) break;
  }
  if (wait(&status) < 0) {
    perror("wait");
  }
  close(rsock);

  return 0;
}
