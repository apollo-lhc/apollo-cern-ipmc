#ifndef USER_HELPERS_H
#define USER_HELPERS_H

int
strlen(const unsigned char * str);

int
str_eq(const unsigned char *s1,
       const unsigned char *s2);

int
a_from_i(unsigned char s[],
         int n,
         unsigned char hex);

int
i_from_a(int * n,
         unsigned char s[],
         unsigned char * hex);

int
strlcpy(unsigned char *dest,
        const unsigned char *src);

// int
// vec_a_from_vec_i (unsigned char * a,
//                   unsigned char * i,
//                   int len,
//                   unsigned char hex);
// 
// int
// vec_i_from_vec_a (unsigned char * i,
//                   unsigned char * a,
//                   int len,
//                   unsigned char hex);

#endif // USER_HELPERS_H
