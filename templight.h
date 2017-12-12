#ifndef _LOWENWARE_TEMPLIGHT_H_
#define _LOWENWARE_TEMPLIGHT_H_

#include <stdarg.h>
#include <aisl/aisl.h>

/* types -------------------------------------------------------------------- */

typedef struct _templight_t * templight_t;

/* functions ---------------------------------------------------------------- */

/* Constructor
 * template_name - name of template file without .tpl.html
 * root - absolute or relative path to templates folder
 * Result: a new instance of object */
templight_t
templight_new(const char * name, const char * root);




/* Destructor
 * Ignores content of LINK atoms
 * Result: NULL */
void
templight_free(templight_t self);


const char *
templight_get_name(templight_t self);


/* Clones template to new one
 * self - existing template
 * Result: new templight object with LINK atoms instead of PLAIN */
templight_t
templight_clone(templight_t self);


/* Repeates the sub block in atom
 * REsult: pointer to new block or NULL on fail */
templight_t
templight_new_block(templight_t self, const char * block);


/* Composes one label inside block
 * Result: amount of composes */
int
templight_set_string(templight_t self, const char *var_name, 
                                       const char *value);


int
templight_set_integer(templight_t self, const char *var_name,
                                        const int   value);


int
templight_set_float(templight_t self, const char   *var_name,
                                      const double  value);

int
templight_set_block(templight_t self, const char  *var_name,
                                      templight_t  value);

int
templight_set_printf( templight_t self, const char *var_name, 
                                        const char *format, ...);

int
templight_set_vprintf( templight_t self, const char *var_name, 
                                         const char *format,
                                         va_list     list);

/* Outputs templight
 * Result: void */

int
templight_get_content_length(templight_t self);


void
templight_to_stream(templight_t self, aisl_stream_t s);






#endif
