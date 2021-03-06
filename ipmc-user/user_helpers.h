#ifndef USER_HELPERS_H
#define USER_HELPERS_H

int
str_eq(const unsigned char *s1,
       const unsigned char *s2);

int
a_from_i(unsigned char * s,
         int n,
         unsigned int hex);

int
i_from_a(int * n,
         unsigned char s[],
         unsigned int * hex);

unsigned
strcpyl(unsigned char *dest,
        const unsigned char *src);

unsigned
strlenu(const unsigned char * str);

// void
// memcpy_(void *dest, const void *src, int n);

#endif // USER_HELPERS_H
