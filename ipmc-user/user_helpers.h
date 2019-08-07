#ifndef USER_HELPERS_H
#define USER_HELPERS_H

int
strlen(const char * str);

int
str_eq(const char *s1,
       const char *s2);

int
a_from_i(char s[],
         int n,
         char hex);

int
i_from_a(int * n,
         char s[],
         char * hex);

int
strlcpy(char *dest,
        const char *src);

// int
// vec_a_from_vec_i (char * a,
//                   char * i,
//                   int len,
//                   char hex);
// 
// int
// vec_i_from_vec_a (char * i,
//                   char * a,
//                   int len,
//                   char hex);

#endif // USER_HELPERS_H
