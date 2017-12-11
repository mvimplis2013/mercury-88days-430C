#include <sys/types.h>
#include <unistd.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include <syslog.h>

#include "mercury_red_daemon.h"

typedef enum {FALSE, TRUE} Boolean;

static int foreground = 0;

static FILE *errFifo;

static uint64_t  getNum(char* str, int *err) {
  uint64_t val;
  char* endptr;

  *err = 0;
  val = strtoll(str, &endptr, 0);

  if (*endptr) {
    *err = -1;
    val = -1;
  }

  return val;
}

void usage() {
  fprintf(stderr, "\n" \
    "BluSky" \
    "\n");
}

static void initOpts(int argc, char* argv[]) {
  int opt, err, i;

  while ((opt=getopt(argc, argv, "a:b:c:d:e:fgkln:p:s:t:x:vV")) != -1) {
    switch (opt) {
      case 'a':
        break;

      case 'g':
        foreground = 1;
        break;

      default:  /* ? */
        usage();
        exit(EXIT_FAILURE);
    };
  }
}

void fatal(char *fmt, ...) {
  char buf[128];
  va_list ap;

  va_start(ap, fmt);
  vsnprintf(buf, sizeof(buf), fmt, ap);
  va_end(ap);

  fprintf(stderr, "%s\n", buf);

  fflush(stderr);

  exit(EXIT_FAILURE);
}

static void listFiles(const char *dirpath) {
  DIR *dirp;
  struct dirent *dp;
  Boolean isCurrent;

  isCurrent = strcmp(dirpath, ".") == 0;

  dirp = opendir(dirpath);
  if (dirp == NULL) {
    syslog(LOG_ERR, "opendir failed on '%s'", dirpath);
    return;
  }

  // For every entry in this directory print directory + filename
  for (;;){
    syslog(LOG_INFO, "Miltos");

    errno = 0;
    dp = readdir(dirp);
    if (dp == NULL) {
      break;
    } else {
      syslog(LOG_INFO, "XXX ... %s", dp->d_name);
    }

    if (strcmp(dp->d_name, ".") == 0 || strcmp(dp->d_name, "..") == 0) {
      continue;
    }

    if (!isCurrent) {
      syslog(LOG_INFO, "%s/", dirpath);
    }

    syslog(LOG_INFO, "%s\n", dp->d_name);
  }

  if (errno != 0)
    syslog(LOG_ERR, "readdir");

  if (closedir(dirp) == -1)
    syslog(LOG_ERR, "closedir");
}

int main(int argc, char **argv) {
  pid_t pid;
  int flags;

  // check command line parameters
  initOpts(argc, argv);

  if (!foreground) {
    // fork off Parent process
    pid = fork();

    if (pid < 0) {
      exit(EXIT_FAILURE);
    }

    // if we got a good PID ... exit the parrent process
    if (pid > 0) {
      exit(EXIT_SUCCESS);
    }

    // clear file mode creation mask
    umask(0);

    // create a new SID for child process
    if (setsid() < 0) {
      fatal("setsid failed (%m)");
    }

    // change current working directory
    if (chdir("/") < 0) {
      fatal("chdir failed (%m)");
    }

    // close out the standard descriptors
    fclose(stdin);
    fclose(stdout);
  }

  // configure library
  // DBG(4, "millis=%d", 1000);

  // start library

  // create pipe for error reporting
  //unlink(MRD_ERRFIFO);
  //mkfifo(MRD_ERRFIFO, 0664);

  //if (chmod(MRD_ERRFIFO, 0664) < 0) {
  //  fatal("chmod %s failed (%m)", MRD_ERRFIFO);
  //}

  //errFifo = freopen(MRD_ERRFIFO, "w+", stderr);

  //if (errFifo) {
    // set stderr non-blocking
    //flags = fcntl(fileno(errFifo), F_GETFL, 0);
    //fcntl(fileno(errFifo), F_SETFL, flags | O_NONBLOCK);

    // request SUGUP/ SIGTERM from library for termination
    // sleep for ever
    while (1) {
      // cat /dev/pigerr to view daemon errors
      listFiles("/tmp");

      sleep(20);

      fflush(errFifo);
    }
  //} else {
  //  fprintf(stderr, "freopen failed (%m)");
  //}

  return 0;
}
