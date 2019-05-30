#include <user_helpers.h>

/* reverse:  reverse string s in place */
void reverse(char s[])
{
  int i, j;
  char c;
  
  for (i = 0, j = strlen(s)-1; i<j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}  

/* itoa:  convert n to characters in s */
void itoa(int n, char s[])
{
  int i, sign;
  
  if ((sign = n) < 0)  /* record sign */
    n = -n;          /* make n positive */
  i = 0;
  do {       /* generate digits in reverse order */
    s[i++] = n % 10 + '0';   /* get next digit */
  } while ((n /= 10) > 0);     /* delete it */
  if (sign < 0)
    s[i++] = '-';
  s[i] = '\0';
  reverse(s);
}  

// freebsd implementation of strlen
int
strlen(const char * str)
{
    const char *s;
    for (s = str; *s; ++s) {}
    return(s - str);
}

// compare two strings. returns 1 when equal.
int
str_eq (const char *s1,
        const char *s2)
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
