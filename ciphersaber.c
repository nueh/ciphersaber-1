/*
 * Copyright © 2012 Bart Massey
 * [This program is licensed under the "MIT License"]
 * Please see the file COPYING in the source
 * distribution of this software for license terms.
 */

/*
 * CipherSaber
 * http://ciphersaber.gurus.com
 */

#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
extern char *getpass();

static unsigned char state[256];
static unsigned char key[256];
static int nkey;

#ifdef linux
static void mkiv(void) {
  int fd, n;

  fd = open("/dev/random", O_RDONLY, 0);
  if (fd == -1) {
    perror("open: /dev/random");
    exit(1);
  }
  n = read(fd, (char *) &key[nkey], 10);
  if (n == -1) {
    perror("read /dev/random");
    exit(1);
  }
  if (n < 10) {
    fprintf(stderr, "short read on /dev/random\n");
    exit(1);
  }
  (void) close(fd);
}
#endif

int main(int argc, char **argv) {
  int encrypt = 0;
  unsigned char *ivp;
  int tmp, ch, k;
  unsigned char i, j, n;

#ifdef READ_KEY
  char *s;

  /* mode */
  if (argc == 2 && !strcmp(argv[1], "-e")) {
    encrypt = 1;
  } else if (argc == 2 && !strcmp(argv[1], "-d")) {
    encrypt = 0;
  } else {
    fprintf(stderr, "usage: ciphersaber [-e|-d]\n");
    return 1;
  }

  /* key acquisition */
  /* XXX 128-bit maximum key ala getpass(3)! */
  s = getpass("Key: ");
  if (!s) {
    perror("getpass");
    return 1;
  }
  (void) strcpy((char *) key, s);
  nkey = strlen((char *) key);
  if (encrypt) {
    s = getpass("Again: ");
    if (!s) {
      perror("getpass");
      return 1;
    }
    if (strcmp(s, (char *) key)) {
      fprintf(stderr, "Key mismatch\n");
      return 1;
    }
  }
#else
  /* mode */
  if (argc == 3 && !strcmp(argv[1], "-e")) {
    encrypt = 1;
  } else if (argc == 3 && !strcmp(argv[1], "-d")) {
    encrypt = 0;
  } else {
    fprintf(stderr, "usage: ciphersaber [-e|-d] <key>\n");
    return 1;
  }
  nkey = strlen(argv[2]);
  strncpy((char *)key, argv[2], nkey);
#endif

  /* iv handling */
  ivp = &key[nkey];
  if (encrypt) {
    mkiv();
    for (i = 0; i < 10; i++)
      putchar(ivp[i]);
  } else {
    for (i = 0; i < 10; i++)
      ivp[i] = getchar();
  }
  nkey += 10;

  /* mixing */
  for (k = 0; k < 256; k++)
    state[k] = k;
  j = 0;
  for (k = 0; k < 256; k++) {
    i = k;
    j += state[i] + key[i % nkey];
    tmp = state[i];
    state[i] = state[j];
    state[j] = tmp;
  }

  /* ciphering */
  i = j = 0;
  while ((ch = getchar()) != EOF) {
    i++;
    j += state[i];
    tmp = state[i];
    state[i] = state[j];
    state[j] = tmp;
    n = state[i] + state[j];
    putchar(state[n] ^ ch);
  }

  return 0;
}
