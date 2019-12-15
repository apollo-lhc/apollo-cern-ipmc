#include <user_helpers.h>

#include <debug.h>


// static const unsigned char DEBUG = 1;

/* reverse:  reverse string s in place */
void
reverse(unsigned char * s)
{
  int i, j;
  unsigned char c;
  
  for (i = 0, j = strlenu(s)-1; i<j; i++, j--) {
    c = s[i];
    s[i] = s[j];
    s[j] = c;
  }
}  

/* itoa:  convert n to characters in s */
int
a_from_i(unsigned char * s,
         int n,
         unsigned int hex)
{
  int i, sign;
  int aux;


  // if (DEBUG) {
  //   debug_printf("\n... converting %d to string", n);
  // }


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
      n /= 16;
    } else {
      /* store next digit */
      s[i++] = n % 10 + '0';
      /* then eliminate it */
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

  // if (DEBUG) {
  //   debug_printf("\n!!!!!!! %s", s);
  // }
  
  /* string is backwards; let's reverse it */
  reverse(s);
  
  return 0;
}  


/* itoa:  convert n to characters in s */
int
i_from_a(int * n,
         unsigned char s[],
         unsigned int * hex)
{
  int i;
  char sign = 0;

  // if (DEBUG) {
  //   debug_printf("\n... converting %s to integer", s);
  // }

  *n = 0; // Initialize result

  if (s[0] == '-') {
    sign = 1;
    s++;
  }

  if (s[0] == '0' && s[1] == 'x') {
    *hex = 1;
    for (i = 2; s[i] != '\0'; i++) {
      if ('0' <= s[i] && s[i] <= '9') {
        *n = *n * 16 + s[i] - '0';
      } else if ('a' <= s[i] && s[i] <= 'f') {
        *n = *n * 16 + s[i] - 'a' + 10;
      }
      else {
        return 1;
      }
    }
  } else {
    *hex = 0;
    for (i = 0; s[i] != '\0'; i++) {
      *n = *n * 10 + s[i] - '0';
    }
  }

  if (sign == 1) {
    *n *= -1;
  }

  return 0;
}

// freebsd implementation of strlen but with unsigned input
unsigned
strlenu(const unsigned char * str)
{
    const unsigned char * s;
    for (s = str; *s; ++s);
    return (s - str);
}

// compare two strings. returns 1 when equal.
int
str_eq (const unsigned char *s1,
        const unsigned char *s2)
{
  const unsigned char *p1 = s1;
  const unsigned char *p2 = s2;

  // if (DEBUG) {
  //   debug_printf("\n@@@@@@@ str_eq %s %s", s1, s2);
  // }
  
  while (*p1 != '\0') {
    if (*p1 != *p2){
      return 0;
    }
    // if (DEBUG) {
    //   debug_printf("\n%c %c", *p1, *p2);
    // }
    p1++;
    p2++;
  }

  // if (DEBUG) {
  //   debug_printf("\n@@@@@@@ str_eq 2\n");
  // }
  if (*p2 != '\0') {
    return 0;
  }

  // if (DEBUG) {
  //   debug_printf("\n@@@@@@@ str_eq 3\n");
  // }
  return 1;
}

// copying strings
unsigned
strcpyl(unsigned char *dest,
        const unsigned char *src)
{
  unsigned l = strlenu(src); 
  memcpy(dest, src, l+1);
  return l;
}


// void
// memcpy_(void *dest, const void *src, int n) 
// { 
//    // Typecast src and dest addresses to (char *) 
//    const char *csrc = (const char *)src; 
//    char *cdest = (char *)dest; 
//    int i;
//    // Copy contents of src[] to dest[] 
//    for (i = 0; i < n; i++) { 
//        cdest[i] = csrc[i];
//    }
// } 
