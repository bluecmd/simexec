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
#include <pthread.h>


#define DAEMON_HOST "192.168.1.2"
#define STREAM_STDIN 1


static int simexec_connect(const char* addr, int port) {
  struct addrinfo hints;
  struct addrinfo *result, *rp;
  struct sctp_event_subscribe events;
  int sock;
  char portstr[7];
  snprintf(portstr, 7, "%d", port);

  memset(&hints, 0, sizeof(hints));
  memset(&events, 0, sizeof(events));

  hints.ai_family = AF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_flags = AI_PASSIVE | AI_NUMERICSERV;
  hints.ai_protocol = IPPROTO_SCTP;
  hints.ai_canonname = NULL;
  hints.ai_addr = NULL;
  hints.ai_next = NULL;

  /* Enable receipt of SCTP Snd/Rcv Data via sctp_recvmsg */
  events.sctp_data_io_event = 1;

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

  if (setsockopt(sock, SOL_SCTP, SCTP_EVENTS, &events, sizeof(events)) == -1) {
    perror("setsockopt");
    return -1;
  }

  freeaddrinfo(result);
  return sock;
}

void * redirect_stdin(void *sock_ptr) {
  int sock = *(int*)sock_ptr;
  while (1) {
    char buf[1024];
    int bytes = read(0, buf, sizeof(buf));
    if (bytes <= 0) {
      break;
    }
    if (sctp_sendmsg(sock, buf, bytes, NULL, 0, 0, 0,
          STREAM_STDIN, 0, 0) == -1) {
      break;
    }
  }
  return NULL;
}

int main(int argc, char** argv, char** envp) {
  int sock;

  if ((sock = simexec_connect(DAEMON_HOST, 7012)) == -1) {
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

    {
      pthread_t redir_thread;
      pthread_create(&redir_thread, NULL, redirect_stdin, &sock);
    }
  }

  while(1) {
    int size;
    int flags;
    struct sctp_sndrcvinfo sinfo;
    char buf[1024];

    size = sctp_recvmsg(sock, buf, sizeof(buf), NULL, 0, &sinfo, &flags);
    /* Execution ended */
    if (size <= 0) {
      perror("recvmsg");
      return 1;
    }

    if (sinfo.sinfo_stream == 0) {
      /* ack exit code */
      sctp_sendmsg(sock, buf, size, NULL, 0, 0, 0, 0, 0, 0);
      return ntohl(*(uint32_t*)buf);
    } else {
      write(sinfo.sinfo_stream, buf, size);
    }
  }

  return 0;
}
