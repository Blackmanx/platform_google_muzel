/*
 * json_print.h		"print regular or json output, based on json_writer".
 *
 *             This program is free software; you can redistribute it and/or
 *             modify it under the terms of the GNU General Public License
 *             as published by the Free Software Foundation; either version
 *             2 of the License, or (at your option) any later version.
 *
 * Authors:    Julien Fortin, <julien@cumulusnetworks.com>
 */

#ifndef _JSON_PRINT_H_
#define _JSON_PRINT_H_

#include "json_writer.h"
#include "color.h"

json_writer_t *get_json_writer(void);

/*
 * use:
 *      - PRINT_ANY for context based output
 *      - PRINT_FP for non json specific output
 *      - PRINT_JSON for json specific output
 */
enum output_type {
	PRINT_FP = 1,
	PRINT_JSON = 2,
	PRINT_ANY = 4,
};

void new_json_obj(int json);
void delete_json_obj(void);

bool is_json_context(void);

void fflush_fp(void);

void open_json_object(const char *str);
void close_json_object(void);
void open_json_array(enum output_type type, const char *delim);
void close_json_array(enum output_type type, const char *delim);

#define _PRINT_FUNC(type_name, type)					\
	void print_color_##type_name(enum output_type t,		\
				     enum color_attr color,		\
				     const char *key,			\
				     const char *fmt,			\
				     type value);			\
									\
	static inline void print_##type_name(enum output_type t,	\
					     const char *key,		\
					     const char *fmt,		\
					     type value)		\
	{								\
		print_color_##type_name(t, COLOR_NONE, key, fmt, value);	\
	}
_PRINT_FUNC(int, int);
_PRINT_FUNC(bool, bool);
_PRINT_FUNC(null, const char*);
_PRINT_FUNC(string, const char*);
// ANDROID: upstream used 'uint' we rename the true function to 'uint32', see below
#define print_color_uint32 print_color_uint
_PRINT_FUNC(uint32, unsigned int);
_PRINT_FUNC(hu, unsigned short);
_PRINT_FUNC(hex, unsigned int);
_PRINT_FUNC(0xhex, unsigned int);
_PRINT_FUNC(lluint, unsigned long long int);
#undef _PRINT_FUNC

// ANDROID: The upstream version of iproute2 has a bug where print_uint() gets
// called with "%u" fmt string and u64 value, which fails to generate the
// correct behaviour on 32-bit userspace.  Detect this and autocorrect.
#define print_uint(t,key,fmt,val) do {       \
  if (sizeof(val) <= sizeof(unsigned int)) { \
    print_uint32((t),(key),(fmt),(val));     \
  } else {                                   \
    print_lluint((t),(key),(fmt),(val));     \
  };                                         \
} while (0)

#endif /* _JSON_PRINT_H_ */
