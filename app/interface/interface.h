/**
 * @file interface.h
 * @brief 
 * @author islamkosker (https://github.com/islamkosker)
 * @date 2026-05-06 21:49
 * @version 1.0.0
 * @copyright Copyright (c) 2026 Islam Kosker.
 */


#ifndef INTERFACE_H
#define INTERFACE_H

typedef enum
{
    INTERFACE_STATUS_OK = 0,
    INTERFACE_STATUS_ERROR = -1,
    INTERFACE_STATUS_BUSY = -2,
    INTERFACE_STATUS_INVALID_ARG = -3
} interface_status_t;

typedef void* hal_abc_ptr_t; // abstract hardware object pointer


#endif