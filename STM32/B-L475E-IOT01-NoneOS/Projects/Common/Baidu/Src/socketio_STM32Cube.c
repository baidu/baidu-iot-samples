/**
 ******************************************************************************
 * @file    socketio_SMT32Cube.c
 * @author  Central LAB
 * @version V3.0.0
 * @date    21-April-2017
 * @brief   Adapter for Socket I/O
 ******************************************************************************
 * @attention
 *
 * <h2><center>&copy; COPYRIGHT(c) 2017 STMicroelectronics</center></h2>
 *
 * Licensed under MCD-ST Liberty SW License Agreement V2, (the "License");
 * You may not use this file except in compliance with the License.
 * You may obtain a copy of the License at:
 *
 *        http://www.st.com/software_license_agreement_liberty_v2
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *   1. Redistributions of source code must retain the above copyright notice,
 *      this list of conditions and the following disclaimer.
 *   2. Redistributions in binary form must reproduce the above copyright notice,
 *      this list of conditions and the following disclaimer in the documentation
 *      and/or other materials provided with the distribution.
 *   3. Neither the name of STMicroelectronics nor the names of its contributors
 *      may be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 ******************************************************************************
 */
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include "azure_c_shared_utility/socketio.h"
#include "azure_c_shared_utility/singlylinkedlist.h"
#include "tcpsocketSTM32Cube.h"
#include "azure_c_shared_utility/xlogging.h"

#define UNABLE_TO_COMPLETE -2
#define STM32CUBE_RECEIVE_BYTES_VALUE    128


typedef enum IO_STATE_TAG
{
    IO_STATE_CLOSED,
    IO_STATE_OPENING,
    IO_STATE_OPEN,
    IO_STATE_CLOSING,
    IO_STATE_ERROR
} IO_STATE;

typedef struct PENDING_SOCKET_IO_TAG
{
    unsigned char* bytes;
    size_t size;
    ON_SEND_COMPLETE on_send_complete;
    void* callback_context;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} PENDING_SOCKET_IO;

typedef struct SOCKET_IO_INSTANCE_TAG
{
    TCPSOCKETCONNECTION_HANDLE tcp_socket_connection;
    ON_BYTES_RECEIVED on_bytes_received;
    ON_IO_ERROR on_io_error;
    void* on_bytes_received_context;
    void* on_io_error_context;
    char* hostname;
    int port;
    IO_STATE io_state;
    SINGLYLINKEDLIST_HANDLE pending_io_list;
} SOCKET_IO_INSTANCE;

/**
 * @brief this function will clone an option given by name and value
 * @param char* name option name
 * @param void* value pointer to the option value
 * @retval None
 */
static void* socketio_CloneOption(const char* name, const void* value)
{
    //(void)(name, value);
    return NULL;
}

/**
 * @brief this function destroys an option previously created
 * @param char* name option name
 * @param void* value pointer to the option value
 * @retval int OK/Error (0/!=0)
 */
static void socketio_DestroyOption(const char* name, const void* value)
{
    //(void)(name, value);
}

/**
 * @brief this function retrieves the options
 * @param CONCRETE_IO_HANDLE socket_io pointer to the socket I/O
 * @retval OPTIONHANDLER_HANDLE pointer to the options handle
 */
static OPTIONHANDLER_HANDLE socketio_retrieveoptions(CONCRETE_IO_HANDLE socket_io)
{
    OPTIONHANDLER_HANDLE result;
    (void)socket_io;
    result = OptionHandler_Create(socketio_CloneOption, socketio_DestroyOption, socketio_setoption);
    if (result == NULL) {
      /*return as is*/
    } else {
      /*insert here work to add the options to "result" handle*/
    }
    return result;
}

static const IO_INTERFACE_DESCRIPTION socket_io_interface_description =
{
    socketio_retrieveoptions,
    socketio_create,
    socketio_destroy,
    socketio_open,
    socketio_close,
    socketio_send,
    socketio_dowork,
    socketio_setoption
};

/**
 * @brief function called when there is one error on the Socket I/O instance
 * @param SOCKET_IO_INSTANCE* socket_io_instance pointer to the socket I/O instance
 * @retval None
 */
static void indicate_error(SOCKET_IO_INSTANCE* socket_io_instance)
{
    if (socket_io_instance->on_io_error != NULL)
    {
        socket_io_instance->on_io_error(socket_io_instance->on_io_error_context);
    }
}

/**
 * @brief function called for adding data to the queue
 * @param SOCKET_IO_INSTANCE* socket_io_instance pointer to the socket I/O instance
 * @param unsigned char* buffer data to send
 * @param size_t size length of the data to send
 * @param ON_SEND_COMPLETE on_send_complete callback to call when all the data will be added to the queue
 * @param void* callback_context pointer to the on_send_complete callback
 * @retval None
 */
static int add_pending_io(SOCKET_IO_INSTANCE* socket_io_instance, const unsigned char* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;
    PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)malloc(sizeof(PENDING_SOCKET_IO));
    if (pending_socket_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        pending_socket_io->bytes = (unsigned char*)malloc(size);
        if (pending_socket_io->bytes == NULL)
        {
            free(pending_socket_io);
            result = __LINE__;
        }
        else
        {
            pending_socket_io->size = size;
            pending_socket_io->on_send_complete = on_send_complete;
            pending_socket_io->callback_context = callback_context;
            pending_socket_io->pending_io_list = socket_io_instance->pending_io_list;
            (void)memcpy(pending_socket_io->bytes, buffer, size);

            if (singlylinkedlist_add(socket_io_instance->pending_io_list, pending_socket_io) == NULL)
            {
                free(pending_socket_io->bytes);
                free(pending_socket_io);
                result = __LINE__;
            }
            else
            {
                result = 0;
            }
        }
    }

    return result;
}

/**
 * @brief Concrete function that destroies a socket I/O
 * @param void *io_create_parameters pointer to the SOCKETIO_CONFIG structure
 * @retval CONCRETE_IO_HANDLE* socket_io pointer to the socket I/O instance
 */
CONCRETE_IO_HANDLE socketio_create(void* io_create_parameters)
{
    SOCKETIO_CONFIG* socket_io_config = io_create_parameters;
    SOCKET_IO_INSTANCE* result;

    if (socket_io_config == NULL)
    {
        result = NULL;
    }
    else
    {
        result = malloc(sizeof(SOCKET_IO_INSTANCE));
        if (result != NULL)
        {
            result->pending_io_list = singlylinkedlist_create();
            if (result->pending_io_list == NULL)
            {
                free(result);
                result = NULL;
            }
            else
            {
                result->hostname = (char*)malloc(strlen(socket_io_config->hostname) + 1);
                if (result->hostname == NULL)
                {
                    singlylinkedlist_destroy(result->pending_io_list);
                    free(result);
                    result = NULL;
                }
                else
                {
                    strcpy(result->hostname, socket_io_config->hostname);
                    result->port = socket_io_config->port;
                    result->on_bytes_received = NULL;
                    result->on_io_error = NULL;
                    result->on_bytes_received_context = NULL;
                    result->on_io_error_context = NULL;
                    result->io_state = IO_STATE_CLOSED;
                    result->tcp_socket_connection = NULL;
                }
            }
        }
    }

    return result;
}

/**
 * @brief Concrete function that destroies a socket I/O
 * @param CONCRETE_IO_HANDLE* socket_io pointer to the socket I/O instance
 * @retval None
 */
void socketio_destroy(CONCRETE_IO_HANDLE socket_io)
{
    if (socket_io != NULL)
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;

        tcpsocketconnection_destroy(socket_io_instance->tcp_socket_connection);

        /* clear all pending IOs */
        LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
        while (first_pending_io != NULL)
        {
            PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)singlylinkedlist_item_get_value(first_pending_io);
            if (pending_socket_io != NULL)
            {
                free(pending_socket_io->bytes);
                free(pending_socket_io);
            }

            (void)singlylinkedlist_remove(socket_io_instance->pending_io_list, first_pending_io);
            first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
        }

        singlylinkedlist_destroy(socket_io_instance->pending_io_list);
        free(socket_io_instance->hostname);
        free(socket_io);
    }
}

/**
 * @brief Concrete function that opens a socket I/O
 * @param CONCRETE_IO_HANDLE* socket_io pointer to the socket I/O instance
 * @param ON_IO_OPEN_COMPLETE on_io_open_complete callback to call when the socket will be opened
 * @param void* on_io_open_complete_context pointer to the on_io_open_complete callback
 * @param ON_BYTES_RECEIVED on_bytes_received callback to call when it will receives bytes
 * @param void* on_bytes_received_context pointer to the on_bytes_received callback
 * @param ON_IO_ERROR on_io_error callback to call when there will be one error
 * @param void* on_io_error_context pointer to the on_io_error callback
 * @retval int OK/Error
 */
int socketio_open(CONCRETE_IO_HANDLE socket_io, ON_IO_OPEN_COMPLETE on_io_open_complete, void* on_io_open_complete_context, ON_BYTES_RECEIVED on_bytes_received, void* on_bytes_received_context, ON_IO_ERROR on_io_error, void* on_io_error_context)
{
    int result;

    SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
    if (socket_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        socket_io_instance->tcp_socket_connection = tcpsocketconnection_create();
        if (socket_io_instance->tcp_socket_connection == NULL)
        {
            result = __LINE__;
        }
        else
        {
            if (tcpsocketconnection_connect(socket_io_instance->tcp_socket_connection, socket_io_instance->hostname, socket_io_instance->port) != 0)
            {
                tcpsocketconnection_destroy(socket_io_instance->tcp_socket_connection);
                socket_io_instance->tcp_socket_connection = NULL;
                result = __LINE__;
            }
            else
            {
                tcpsocketconnection_set_blocking(socket_io_instance->tcp_socket_connection, false, 0);

                socket_io_instance->on_bytes_received = on_bytes_received;
                socket_io_instance->on_bytes_received_context = on_bytes_received_context;

                socket_io_instance->on_io_error = on_io_error;
                socket_io_instance->on_io_error_context = on_io_error_context;

                socket_io_instance->io_state = IO_STATE_OPEN;

                result = 0;
            }
        }
    }
    
    if (on_io_open_complete != NULL)
    {
        on_io_open_complete(on_io_open_complete_context, result == 0 ? IO_OPEN_OK : IO_OPEN_ERROR);
    }

    return result;
}

/**
 * @brief Concrete function that closes a socket I/O
 * @param CONCRETE_IO_HANDLE* socket_io pointer to the socket I/O instance
 * @param ON_IO_CLOSE_COMPLETE on_io_close_complete callback to call when the socket will be closed
 * @param void* callback_context pointer to the on_io_close_complete callback
 * @retval int OK/Error
 */
int socketio_close(CONCRETE_IO_HANDLE socket_io, ON_IO_CLOSE_COMPLETE on_io_close_complete, void* callback_context)
{
    int result = 0;

    if (socket_io == NULL)
    {
        result = __LINE__;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;

        if ((socket_io_instance->io_state == IO_STATE_CLOSED) ||
            (socket_io_instance->io_state == IO_STATE_CLOSING))
        {
            result = __LINE__;
        }
        else
        {
            tcpsocketconnection_close(socket_io_instance->tcp_socket_connection);
            socket_io_instance->tcp_socket_connection = NULL;
            socket_io_instance->io_state = IO_STATE_CLOSED;

            if (on_io_close_complete != NULL)
            {
                on_io_close_complete(callback_context);
            }

            result = 0;
        }
    }

    return result;
}

/**
 * @brief Concrete function that sends bytes on socket I/O instance
 * @param CONCRETE_IO_HANDLE* socket_io pointer to the socket I/O instance
 * @param void* buffer pointer to the data to send
 * @param size_t size size of the data
 * @param ON_SEND_COMPLETE on_send_complete callback to call when all the data will be trasmitted
 * @param void* callback_context pointer to the on_send_complete callback
 * @retval int OK/Error
 */
int socketio_send(CONCRETE_IO_HANDLE socket_io, const void* buffer, size_t size, ON_SEND_COMPLETE on_send_complete, void* callback_context)
{
    int result;

    if ((socket_io == NULL) ||
        (buffer == NULL) ||
        (size == 0))
    {
        /* Invalid arguments */
        result = __LINE__;
    }
    else
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        if (socket_io_instance->io_state != IO_STATE_OPEN)
        {
            result = __LINE__;
        }
        else
        {
            LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            if (first_pending_io != NULL)
            {
                if (add_pending_io(socket_io_instance, buffer, size, on_send_complete, callback_context) != 0)
                {
                    result = __LINE__;
                }
                else
                {
                    result = 0;
                }
            }
            else
            {
                int send_result = tcpsocketconnection_send(socket_io_instance->tcp_socket_connection, buffer, size);
                if (send_result != size)
                {
                    if (send_result < 0)
                    {
                        send_result = 0;
                    }

                    /* queue data */
                    if (add_pending_io(socket_io_instance, (unsigned char*)buffer + send_result, size - send_result, on_send_complete, callback_context) != 0)
                    {
                        result = __LINE__;
                    }
                    else
                    {
                        result = 0;
                    }
                }
                else
                {
                    if (on_send_complete != NULL)
                    {
                        on_send_complete(callback_context, IO_SEND_OK);
                    }

                    result = 0;
                }
            }
        }
    }

    return result;
}

/**
 * @brief Concrete function that sends/receives bytes on socket I/O instance
 * @param CONCRETE_IO_HANDLE* socket_io pointer to the socket I/O instance
 * @retval None
 */
void socketio_dowork(CONCRETE_IO_HANDLE socket_io)
{
    if (socket_io != NULL)
    {
        SOCKET_IO_INSTANCE* socket_io_instance = (SOCKET_IO_INSTANCE*)socket_io;
        if (socket_io_instance->io_state == IO_STATE_OPEN)
        {
            unsigned char recv_bytes[STM32CUBE_RECEIVE_BYTES_VALUE];
            int received = 1;

            LIST_ITEM_HANDLE first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            while (first_pending_io != NULL)
            {
                PENDING_SOCKET_IO* pending_socket_io = (PENDING_SOCKET_IO*)singlylinkedlist_item_get_value(first_pending_io);
                if (pending_socket_io == NULL)
                {
                    socket_io_instance->io_state = IO_STATE_ERROR;
                    indicate_error(socket_io_instance);
                    break;
                }

                int send_result = tcpsocketconnection_send(socket_io_instance->tcp_socket_connection, (const char*)pending_socket_io->bytes, pending_socket_io->size);
                if (send_result != pending_socket_io->size)
                {
                    if (send_result < 0)
                    {
                        if (send_result < UNABLE_TO_COMPLETE)
                        {
                            // Bad error.  Indicate as much.
                            socket_io_instance->io_state = IO_STATE_ERROR;
                            indicate_error(socket_io_instance);
                        }
                        break;
                    }
                    else
                    {
                        /* send something, wait for the rest */
                        (void)memmove(pending_socket_io->bytes, pending_socket_io->bytes + send_result, pending_socket_io->size - send_result);
                    }
                }
                else
                {
                    if (pending_socket_io->on_send_complete != NULL)
                    {
                        pending_socket_io->on_send_complete(pending_socket_io->callback_context, IO_SEND_OK);
                    }

                    free(pending_socket_io->bytes);
                    free(pending_socket_io);
                    if (singlylinkedlist_remove(socket_io_instance->pending_io_list, first_pending_io) != 0)
                    {
                        socket_io_instance->io_state = IO_STATE_ERROR;
                        indicate_error(socket_io_instance);
                    }
                }

                first_pending_io = singlylinkedlist_get_head_item(socket_io_instance->pending_io_list);
            }

            while (received > 0)
            {
              received = tcpsocketconnection_receive(socket_io_instance->tcp_socket_connection, (char *)recv_bytes, STM32CUBE_RECEIVE_BYTES_VALUE);
              if (received > 0)
              {
                  if (socket_io_instance->on_bytes_received != NULL)
                  {
                      /* explictly ignoring here the result of the callback */
                      (void)socket_io_instance->on_bytes_received(socket_io_instance->on_bytes_received_context, recv_bytes, received);
                  }
              }
            }
        }
    }
}

/**
 * @brief Concrete function called for setting one option (not implementing any option)
 * @param CONCRETE_IO_HANDLE* socket_io pointer to the socket I/O instance
 * @param char* optionName option name
 * @param void* value pointer to the option value
 * @retval int (return !=0)
 */
int socketio_setoption(CONCRETE_IO_HANDLE socket_io, const char* optionName, const void* value)
{
    /* Not implementing any options */
    return __LINE__;
}

/**
 * @brief function that describes the Socket I/O interface
 * @param None
 * @retval IO_INTERFACE_DESCRIPTION* pointer to the Socket I/O description
 */
const IO_INTERFACE_DESCRIPTION* socketio_get_interface_description(void)
{
    return &socket_io_interface_description;
}
/******************* (C) COPYRIGHT 2017 STMicroelectronics *****END OF FILE****/

