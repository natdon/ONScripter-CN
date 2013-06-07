
#ifndef __GLIB_H__
#define __GLIB_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// type

typedef void*			gpointer;
typedef const void*		gconstpointer;
typedef int				gint;
typedef unsigned int	guint;
typedef short			gint16;
typedef unsigned short	guint16;
typedef int				gint32;
typedef unsigned int	guint32;
typedef char			gchar;
typedef int				gboolean;
typedef float			gfloat;

// malloc

#define g_malloc(size)	malloc(size)
#define g_malloc0(size)	calloc(size, 1)
#define g_free(ptr)		free(ptr)

#define g_new(type, n)	(type*)malloc(sizeof(type) * n)
#define g_new0(type, n)	(type*)calloc(n, sizeof(type))

#endif
