#define PAGE_SIZE (1<<13)
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
#include <linux/limits.h>
#include <sys/wait.h>
#include <linux/binfmts.h>

static int setup() {
  struct sockaddr_in6 addr;
  int sock;

  memset(&addr, 0, sizeof(addr));

  if ((sock = socket(AF_INET6, SOCK_STREAM, IPPROTO_SCTP)) == -1) {
    perror("socket");
    return -1;
  }

  addr.sin6_family = AF_INET6;
  addr.sin6_port = htons(7012);
  addr.sin6_addr = in6addr_any;

  if (bind(sock, (struct sockaddr *)&addr, sizeof(addr)) == -1) {
    perror("bind");
    return -1;
  }

  if (listen(sock, 5) == -1) {
    perror("listen");
    return -1;
  }

  return sock;
}

static int receive_string(int sock, char** ptr) {
  struct sctp_sndrcvinfo sinfo;
  int flags = 0;
  int recved = 0;

  *ptr = malloc(MAX_ARG_STRLEN);
  if ((recved = sctp_recvmsg(sock, *ptr, MAX_ARG_STRLEN, NULL, NULL,
        &sinfo, &flags)) == -1) {
    perror("sctp_recvmsg");
    exit(1);
  }

  if (recved == 0) {
    fprintf(stderr, "Client disconnected\n");
    exit(0);
  }

  if (**ptr == 0) {
    free(*ptr);
    *ptr = NULL;
  }
  return *ptr != NULL;
}

static int handle_peer(int sock) {
  struct sctp_sndrcvinfo sinfo;
  int flags = 0;
  int recved = 0;
  int pid;

  char cwd[PATH_MAX];
  char* argv[ARG_MAX];
  char* envp[ARG_MAX];
  char** ptr;

  memset(&argv, 0, sizeof(argv));
  memset(&envp, 0, sizeof(envp));

  sctp_recvmsg(sock, cwd, sizeof(cwd), NULL, NULL, NULL, NULL);

  for (ptr = envp; receive_string(sock, ptr); ptr++);
  for (ptr = argv; receive_string(sock, ptr); ptr++);

  if ((pid = fork()) == 0) {
    int out = sock;
    /* TODO(bluecmd): Stderr should be on an other stream, but I haven't
     * figured out how to do that nicely yet. */

    memset(&sinfo, 0, sizeof(sinfo));
    sinfo.sinfo_stream = 1;
    setsockopt(out, IPPROTO_SCTP, SCTP_DEFAULT_SEND_PARAM, &sinfo, sizeof(sinfo));

    dup2(out, 0);
    dup2(out, 1);
    dup2(out, 2);
    chdir(cwd);
    execve(argv[0], argv, envp);
  } else {
    uint32_t status = 0;
    int ret;
    printf("[%d] %s\n", pid, argv[0]);
    waitpid(pid, &status, 0);
    printf("[%d] exited: %d\n", pid, WEXITSTATUS(status));
    status = htonl(WEXITSTATUS(status));
    sctp_sendmsg(sock, &status, sizeof(status), NULL, 0, 0, 0, 0, 0, 0);
    ret = sctp_recvmsg(sock, &status, sizeof(status), NULL, 0, NULL, NULL);
    printf("[%d] acked exit (%d), ret: %d\n", pid, status, ret);
    shutdown(sock, SHUT_RDWR);
  }

  return 0;
}

int main() {
  int sock;

  if ((sock = setup()) == -1) {
    return 1;
  }

  /* we don't care about the return codes of our forks */
  signal(SIGCHLD, SIG_IGN);

  while(1)
  {
    int peer = accept(sock, NULL, NULL);
    if (fork() == 0) {
      signal(SIGCHLD, SIG_DFL);
      return handle_peer(peer);
    }
    close(peer);
  }

  return 0;
}
