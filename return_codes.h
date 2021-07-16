#ifndef RETURN_CODES_H_INCLUDED
#define RETURN_CODES_H_INCLUDED

typedef enum return_codes
{
    E_SUCCESS = 0,
    E_CONNECTION_CLOSED = -1,
    E_INVALID_INPUT = 1,
    E_OUT_OF_MEMORY = 2,
    E_FILE_NOT_FOUND = 3,
    E_FILE_CREATE_ERROR = 4,
    E_FILE_READ_ERROR = 5,
    E_FILE_WRITE_ERROR = 6,
} return_codes;

#endif
