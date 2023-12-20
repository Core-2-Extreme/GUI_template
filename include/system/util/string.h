#ifndef _STRING_H
#define _STRING_H
#include <stdbool.h>
#include <stdint.h>

#define DEF_STRING_INITIAL_CAPACITY 16

typedef struct
{
	uint8_t sequencial_id;	//Used to detect string buffer changes.
	uint32_t capacity;		//Current buffer capacity (without NULL terminator, so (capacity + 1) bytes are allocated).
	uint32_t length;		//Current string length (without NULL terminator).
	char* buffer;		    //String buffer.
} Util_string;

/**
 * @brief Initialize a string struct.
 * @param string Pointer for target struct.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_string_init(Util_string* string);

/**
 * @brief Free a string struct.
 * @param string Pointer for target struct.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
void Util_string_free(Util_string* string);

/**
 * @brief Clear string data.
 * @param string Pointer for target struct.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_string_clear(Util_string* string);

/**
 * @brief Set string data.
 * @param string Pointer for target struct.
 * @param source_string Pointer for source string.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_string_set(Util_string* string, const char* source_string);

/**
 * @brief Add (append) string data.
 * @param string Pointer for target struct.
 * @param source_string Pointer for source string to add.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_string_add(Util_string* string, const char* source_string);

/**
 * @brief Set string data with format.
 * @param string Pointer for target struct.
 * @param format_string Pointer for format string.
 * @param additional_parameters Additional parameters for format_string.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_string_format(Util_string* string, const char* format_string, ...);

/**
 * @brief Resize string buffer size.
 * @note If new_capacity is less than current string length,
 * string data WILL BE TRUNCATED to length of new_capacity.
 * @param string Pointer for target struct.
 * @param new_capacity New buffer capacity.
 * @return On success DEF_SUCCESS, on failure DEF_ERR_*.
 * @note Thread safe
*/
uint32_t Util_string_resize(Util_string* string, uint32_t new_capacity);

/**
 * @brief Check if struct is valid (so that safe to access the buffer).
 * @note If string is empty (buffer is allocated but string length is 0)
 * this will return true.
 * @param string Pointer for target struct.
 * @return True if struct is valid, otherwise false.
 * @note Thread safe
*/
bool Util_string_is_valid(Util_string* string);

/**
 * @brief Check if struct is valid and contains at least 1 character.
 * @note If string is empty (buffer is allocated but string length is 0)
 * this will return false.
 * @param string Pointer for target struct.
 * @return True if struct is valid AND contains at least 1 character, otherwise false.
 * @note Thread safe
*/
bool Util_string_has_data(Util_string* string);

#endif //_STRING_H
