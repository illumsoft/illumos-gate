
/* : : generated by proto : : */

#if !defined(__PROTO__)
#include <prototyped.h>
#endif
#if !defined(__LINKAGE__)
#define __LINKAGE__		/* 2004-08-11 transition */
#endif
#include <ast_getopt.h>

#if !defined(_GETOPT_H) && !defined(_AST_STD_I)

#define _GETOPT_H		1

#define no_argument		0
#define required_argument	1
#define optional_argument	2

struct option
{
	const char*	name;
	int		has_arg;
	int*		flag;
	int		val;
};

extern __MANGLE__ int	getopt_long __PROTO__((int, char* const*, const char*, const struct option*, int*));
extern __MANGLE__ int	getopt_long_only __PROTO__((int, char* const*, const char*, const struct option*, int*));

#endif
