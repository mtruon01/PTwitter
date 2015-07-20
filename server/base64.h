#ifndef BASE64_H
#define BASE64_H

#ifdef __cplusplus
#include <string>

/** 
 * encode a string in base64.
 * @param data the data to be encoded.
 * @param length the length of data.
 * @return the base64 string
 */
std::string base64_encode_cpp(const std::string &data);

/** 
 * encode a string in base64.
 * @param data the data to be encoded.
 * @param length the length of data.
 * @return the base64 string
 */
std::string base64_decode_cpp(const std::string &base64);

#endif // __cplusplus


#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

/** 
 * encode string in base64.
 * mostly based on the the same function implemented in wget. 
 * @param data the data to be encoded.
 * @param length the length of data.
 * @return the base64 string, or NULL if failed.
 * @warning the caller must free the returned char.
 */
char *base64_encode(const char *data);

/** 
 * decode string in base64.
 * partly based on the the same function implemented in wget. 
 * We assume the decoded values IS a string.
 * @param base64 the encode input, its length should be multiple of 4
 * @return the base64 string, or NULL if failed.
 * @warning the caller must free the returned char.
 */
char *base64_decode(const char *base64);

#ifdef __cplusplus
}	// extern "C"
#endif // __cplusplus

#endif // BASE64_H
