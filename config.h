#ifndef _STANDARD_CONFIG_H_
#define _STANDARD_CONFIG_H_

#include <stdbool.h>

/* TODO: add config_next_root() method to iterate nodes with same name */

/* config_t class operates with configuration files with Löwenware syntax.
 * It allows both: to read and to write your settings to text files.
 *
 * Löwenware config_turation Syntax:
 * 1. Comments. Everything that follow semicolon sign is a comment
 * 2. Nodes. Node defines group of parameters. Node name must begin from new
 *    line and colon sign must follow it
 * 3. Parameters and values. Must start from new line and end up on it. 
 *    Parameter should be splitted from value with equal sign.
 * 4. Indents are important. The define ownership of parameters and values to
 *    some node.
 *
 * ; + BEGIN EXAMPLE CONFIG ---------------------------------------------------
 * name = default config
 * file = lw/config.h
 * example:                             ; this is node
 *     indent = 4                       ; indent for this node is 4 spaces
 *     aim = demonstrate config usage
 *     syntax = strict
 *     speed  = fast
 * ; + END EXAMPLE CONFIG ---------------------------------------------------
 *
 * Tips:
 * 1. Reading and Writing of config will save indents and comments
 * 2. Do not keep config_t instance in memory all the time. Use it to read
 *    settings, pass it to necessary components of your program and free used
 *    resources.
 *
 * */

typedef struct _config_t * config_t;

typedef enum {

  CONFIG_SUCCESS,
  CONFIG_FILE_ERROR,
  CONFIG_SYNTAX_ERROR

} config_status_t;


/* create new empty instance of config_t
 * @return new config_t object
 * */
config_t
config_new();


/* free resources used by config_t object
 * @param self config_t object
 * */
void
config_free(config_t self);


/* get line number. can be used it in case of read error
 * @param self config_t object
 * @return line number
 * */
uint32_t
config_get_lines(config_t self);


/* read methods ------------------------------------------------------------- */

/* read configuration from file
 * @param self config_t object
 * @param file null-terminated string with path to file
 * @return lwSuccess or lwReadFileError
 * */
config_status_t
config_read(config_t self, const char * file);


/* get param as a string (caller must free result using ws_string_free)
 * @param self config_t object
 * @param key null-terminated string with key name
 * @return null-terminated string with value
 * */
char *
config_get_string(config_t self, const char * key);

/* get param as a chars
 * @param self config_t object
 * @param key null-terminated string with key name
 * @param fb null-terminated string with fallback value
 * @return null-terminated string with value
 * */
const char *
config_get_chars(config_t self, const char * key, const char * fb);


/* get param as an integer
 * @param self config_t object
 * @param key null-terminated string with key name
 * @param fb fallback value
 * @return key value
 * */
int
config_get_integer(config_t self, const char * key, int fb);


/* get param as boolean
 * @param self config_t object
 * @param key null-terminated string with key name
 * @param fb fallback value
 * @return key value
 * */
bool
config_get_boolean(config_t self, const char * key, bool fb);


/* change root to some child node. if same node requested, function works as 
 * an iterator. Use relative keys to access settings
 * @param self config_t object
 * @param key null-terminated string with node name, NULL for top level
 * @return true if node exists or false
 * */
bool
config_set_root(config_t self, const char * root);

/* write methods ------------------------------------------------------------ */

/* write configuration to file
 * @param self config_t object
 * @param file null-terminated string with path to file
 * @return lwSuccess or lwWriteFileError
 * */
config_status_t
config_write(config_t self, const char * file);


/* set key value
 * @param self config_t object
 * @param key null-terminated string with key name
 * @param value null-terminated string with key value
 * */
void
config_set_string(config_t self, const char * key, const char * value);


/* set key value
 * @param self config_t object
 * @param key null-terminated string with key name
 * @param value key value
 * */
void
config_set_integer(config_t self, const char * key, int value);


/* set key value
 * @param self config_t object
 * @param key null-terminated string with key name
 * @param value key value
 * */
void
config_set_boolean(config_t self, const char * key, bool value);


/* add node to config
 * @param self config_t object
 * @param name null-terminated string with node name
 * */
void
config_add_node(config_t self, const char * name);


#endif
