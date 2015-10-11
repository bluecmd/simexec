#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/sctp.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <netdb.h>
#include <linux/limits.h>


#define STREAM_STDIN 1
#define STREAM_STDOUT 1
#define STREAM_STDERR 2


static int simexec_connect(const char* addr, int port) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  int sock;
  char portstr[7];
  snprintf(portstr, 7, "%d", port);

  memset(&hints, 0, sizeof(hints));

  hints.ai_family = AF_INET6;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
  hints.ai_protocol = IPPROTO_SCTP;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  {
    int s;
    if ((s = getaddrinfo(addr, portstr, &hints, &result)) != 0) {
      fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(s));
      return -1;
    }
  }

  for (rp = result; rp != NULL; rp = rp->ai_next) {
    sock = socket(rp->ai_family, rp->ai_socktype,
        rp->ai_protocol);
    if (sock == -1)
      continue;

    if (connect(sock, rp->ai_addr, rp->ai_addrlen) == 0)
      break;

    close(sock);
  }

   if (rp == NULL) {
     fprintf(stderr, "Could not bind\n");
     return -1;
   }

  freeaddrinfo(result);           /* No longer needed */
  return sock;
}

void redirect_stdin(int sock) {
  while (1) {
    char buf[1024];
    int bytes = read(0, buf, sizeof(buf));
    if (bytes <= 0) {
      return;
    }
    if (sctp_sendmsg(sock, buf, bytes, NULL, 0, 0, 0,
          STREAM_STDIN, 0, 0) == -1) {
      return;
    }
  }
}

int main(int argc, char** argv, char** envp) {
  int sock;

  if ((sock = simexec_connect("::1", 7012)) == -1) {
    return 1;
  }

  {
    char cwd[PATH_MAX]; 
    char** ptr;
    char null = 0;

    getcwd(cwd, PATH_MAX);

    /* Send CWD */
    sctp_sendmsg(sock, cwd, strlen(cwd), NULL, 0, 0, 0, 0, 0, 0);

    /* Send environment */
    for (ptr = envp; *ptr != NULL; ptr++) {
      sctp_sendmsg(sock, *ptr, strlen(*ptr), NULL, 0, 0, 0, 0, 0, 0);
    }

    sctp_sendmsg(sock, &null, 1, NULL, 0, 0, 0, 0, 0, 0);

    /* Send arguments */
    for (ptr = argv+1; *ptr != NULL; ptr++) {
      sctp_sendmsg(sock, *ptr, strlen(*ptr), NULL, 0, 0, 0, 0, 0, 0);
    }

    sctp_sendmsg(sock, &null, 1, NULL, 0, 0, 0, 0, 0, 0);
    if (fork() == 0) {
      redirect_stdin(sock);
      exit(0);
    }
  }

  while(1) {
    int size;
    int flags;
    struct sctp_sndrcvinfo sinfo;
    char buf[1024];

    size = sctp_recvmsg(sock, &buf, sizeof(buf), NULL, 0, &sinfo, &flags);
    /* Execution ended */
    if (size <= 0) {
      perror("recvmsg");
      return 1;
    }

    printf("stream: %d\n",sinfo.sinfo_stream);
    if (sinfo.sinfo_stream == 0) {
      /* exit code */
      return ntohl(*(uint32_t*)buf);
    } else if (sinfo.sinfo_stream == STREAM_STDOUT) {
      fprintf(stdout, "%.*s\n", size, buf);
    } else if (sinfo.sinfo_stream == STREAM_STDERR) {
      fprintf(stderr, "%.*s\n", size, buf);
    }
  }

  return 0;
}
