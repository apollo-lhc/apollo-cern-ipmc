#include <user_helpers.h>

#include <debug.h>

static const char msg_i_c_n[] =
  "~~~~~~~ i: %d; c: %x; n: %d\n";

static const char msg_i_c_n_2[] =
  "------- i: %d; c: %x; n: %d\n";

static const unsigned char DEBUG = 0;

/* reverse:  reverse string s in place */
void
reverse(unsigned char s[])
{
  int i, j;
  unsigned char c;
  
  for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}  

/* itoa:  convert n to characters in s */
int
a_from_i(unsigned char s[],
         int n,
         unsigned char hex)
{
  int i, sign;
  int aux;

  sign = n; /* record sigh */
  if (n < 0) {
    n = -n;          /* make n positive */
  }

  /* generate digits in reverse order */
  i = 0;
  do {
    if (hex == 1) {
      /* store next digit */
      aux = n % 16;
      if (aux < 10) {
        s[i++] = aux + '0';
      } else {
        s[i++] = aux - 10 + 'A';
      }
      /* then eliminate it */
      if (DEBUG) {
        debug_printf(msg_i_c_n_2, i-1, s[i-1], n);
      }
      n /= 16;
    } else {
      /* store next digit */
      s[i++] = n % 10 + '0';
      /* then eliminate it */
      if (DEBUG) {
        debug_printf(msg_i_c_n_2, i-1, s[i-1], n);
      }
      n /= 10;
    }
  } while (n > 0);    

  // prepending 0x in case of hex
  if (hex == 1) {
    s[i++] = 'x';    
    s[i++] = '0';    
  }  

  /* recovering signal */
  if (sign < 0) {
    s[i++] = '-';
  }

  // closing up
  s[i] = '\0';

  if (DEBUG) {
    debug_printf("!!!!!!! %s\n", s);
  }
  
  /* string is backwards; let's reverse it */
  reverse(s);
  
  return 0;
}  


/* itoa:  convert n to characters in s */
int
i_from_a(int * n,
         unsigned char s[],
         unsigned char * hex)
{
  int i;

  *n = 0; // Initialize result 

  if (s[0] == '0' && s[1] == 'x') {
    *hex = 1;
    for (i = 2; s[i] != '\0'; i++) {
      if ('0' <= s[i] && s[i] <= '9') {
        *n = *n * 16 + s[i] - '0';
        if (DEBUG) {
          debug_printf(msg_i_c_n, i, s[i], *n);
        }
      } else if ('a' <= s[i] && s[i] <= 'f') {
        *n = *n * 16 + s[i] - 'a' + 10;
        if (DEBUG) {
          debug_printf(msg_i_c_n, i, s[i], *n);
        }
      }
      else {
        return 1;
      }
    }
  } else {
    *hex = 0;
    for (i = 0; s[i] != '\0'; i++) {
      *n = *n * 10 + s[i] - '0';
      if (DEBUG) {
        debug_printf(msg_i_c_n, i, s[i], *n);
      }
    }
  }

  return 0;
}

// freebsd implementation of strlen
int
strlen(const unsigned char * str)
{
    const unsigned char *s;
    for (s = str; *s; ++s) {}
    return(s - str);
}

// compare two strings. returns 1 when equal.
int
str_eq (const unsigned char *s1,
        const unsigned char *s2)
{
  const unsigned char *p1 = (const unsigned char *) s1;
  const unsigned char *p2 = (const unsigned char *) s2;

  // debug_printf("@@@@@@@ str_eq 1\n");
  
  while (*p1 != '\0') {
    // debug_printf("%c %c\n", *p1, *p2);
    if (*p1 != * p2){
      return 0;
    }
    p1++;
    p2++;
  }

  // debug_printf("@@@@@@@ str_eq 2\n");

  if (*p2 != '\0') {
    return 0;
  }

  // debug_printf("@@@@@@@ str_eq 3\n");

  return 1;
}

// copying strings
int
strlcpy(unsigned char *dest,
        const unsigned char *src)
{
  unsigned i;
  for (i=0; src[i] != '\0'; ++i) {
    dest[i] = src[i];
  }
  dest[i]= '\0';
  return i;
}
