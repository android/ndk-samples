#ifndef BAR_H
#define BAR_H

/* FOO should be defined to '1' here with the magic of LOCAL_EXPORT_CFLAGS */
#ifndef FOO
#error FOO should be defined here !
#endif

#if FOO != 1
#error FOO is not correctly defined here !
#endif

extern int  bar(int  x);

#endif /* BAR_H */
