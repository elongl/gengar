#ifndef RETURN_CODE_H
#define RETURN_CODE_H

typedef enum return_code
{
    E_SUCCESS = 0,
    E_CONNECTION_CLOSED = -1,
    E_INVALID_INPUT = 1,
    E_OUT_OF_MEMORY = 2,
    E_FILE_NOT_FOUND = 3,
    E_FILE_CREATE_ERROR = 4,
    E_FILE_READ_ERROR = 5,
    E_FILE_WRITE_ERROR = 6,
} return_code;

#endif
