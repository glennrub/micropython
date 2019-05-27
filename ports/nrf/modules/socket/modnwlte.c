/*
 * This file is part of the Micro Python project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Glenn Ruben Bakke
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include "py/mphal.h"
#include "py/stream.h"

#ifndef MICROPY_PY_LTE_SOCKET
#define MICROPY_PY_LTE_SOCKET 1
#endif

#ifndef MICROPY_PY_NETWORK
#define MICROPY_PY_NETWORK 1
#endif

#if MICROPY_PY_NETWORK && MICROPY_PY_LTE_SOCKET

#include "nrf_socket.h"
#include "nrf_apn_class.h"

#ifdef MAX
#undef MAX
#endif

#ifdef MIN
#undef MIN
#endif

#include "py/nlr.h"
#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mperrno.h"


#include "modnetwork.h"
#include "pin.h"
#include "genhdr/pins.h"
#include "bsd.h"
#include "bsd_os.h"

extern int nrf_errno;

void bsd_recoverable_error_handler(uint32_t error)
{
    printf("bsd_recoverable_error_handler: %lu\n", error);
}

void bsd_irrecoverable_error_handler(uint32_t error)
{
    printf("bsd_recoverable_error_handler: %lu\n", error);

//    hard_fault_trigger();
}

uint32_t inet_addr(uint8_t a, uint8_t b, uint8_t c, uint8_t d)
{
    uint32_t value = 0;
    value |= (uint32_t)((((uint8_t)(d)) << 24) & 0xFF000000);
    value |= (uint32_t)((((uint8_t)(c)) << 16) & 0x00FF0000);
    value |= (uint32_t)((((uint8_t)(b)) <<  8) & 0x0000FF00);
    value |= (uint32_t)((((uint8_t)(a)) <<  0) & 0x000000FF);

    return value;
}

/// \moduleref network

typedef struct _lte_nrf91_obj_t {
    mp_obj_base_t base;
} lte_nrf91_obj_t;

STATIC lte_nrf91_obj_t lte_nrf91_obj;

STATIC int lte_nrf91_gethostbyname(mp_obj_t nic, const char *name, mp_uint_t len, const char *interface, mp_uint_t interface_len, uint8_t *out_ip, uint8_t * out_family, uint8_t * out_proto) {
    struct nrf_addrinfo * p_info;
    struct nrf_addrinfo hints;
    struct nrf_addrinfo apn_hints;

    memset(&hints, 0, sizeof(struct nrf_addrinfo));
    hints.ai_flags = 0;
    hints.ai_family = NRF_AF_INET;
    if (out_family != NULL)
    {   if (*out_family == MOD_NETWORK_AF_INET) {
            hints.ai_family = NRF_AF_INET;
        } else if (*out_family == MOD_NETWORK_AF_INET6) {
            hints.ai_family = NRF_AF_INET6;
        }
    }    
    hints.ai_socktype = NRF_SOCK_STREAM;
    hints.ai_protocol = 0;

    if ((interface != NULL) && (interface_len > 0)) {
        apn_hints.ai_family = NRF_AF_LTE;
        apn_hints.ai_socktype = NRF_SOCK_MGMT;
        apn_hints.ai_protocol = NRF_PROTO_PDN;
        apn_hints.ai_canonname = (char *)interface;
        printf("DNS on APN: %s, %d\n", interface, interface_len);
        hints.ai_next = &apn_hints;
    }
/*char * p_buffer;
    p_buffer = m_new(char, len + 1);
    memset(p_buffer, 0, len + 1);
    memcpy(p_buffer, name, len);
    
    printf("resolving %s, len: %d\n", name, len);
*/
    int32_t retval = nrf_getaddrinfo(name /*p_buffer*/, NULL, &hints, &p_info);
    //printf("gethostbyname(): retval: %ld\n", retval);

//    m_free(p_buffer);

    if (retval == 0) {
        if (p_info->ai_addr->sa_family == NRF_AF_INET) {
            *out_family = MOD_NETWORK_AF_INET;
            struct nrf_sockaddr_in * p_addr = (struct nrf_sockaddr_in *)p_info->ai_addr;
            memcpy(out_ip, &p_addr->sin_addr.s_addr, sizeof(struct nrf_in_addr));
        } else if (p_info->ai_addr->sa_family == NRF_AF_INET6) {
            *out_family = MOD_NETWORK_AF_INET6;
            struct nrf_sockaddr_in6 * p_addr = (struct nrf_sockaddr_in6 *)p_info->ai_addr;
            memcpy(out_ip, p_addr->sin6_addr.s6_addr, sizeof(struct nrf_in6_addr));
        }

        nrf_freeaddrinfo(p_info);
        return 0;
    }

    return -1;
}

STATIC int lte_nrf91_socket_socket(mod_network_socket_obj_t *socket, int *_errno) {
    printf("creating socket: %u, %u, %u\n", socket->u_param.domain, socket->u_param.type, socket->u_param.proto);
    socket->u_param.fileno = nrf_socket(socket->u_param.domain, socket->u_param.type, socket->u_param.proto);
    printf("socket handle: %d\n", socket->u_param.fileno);

    if (socket->u_param.fileno < 0) {
        *_errno = MP_EINVAL;
        return -1;
    }

    return 0;
}

STATIC void lte_nrf91_socket_close(mod_network_socket_obj_t *socket) {
    (void)nrf_close(socket->u_param.fileno);

    // Clear it anyway.
    socket->u_param.fileno = -1;
}

STATIC int lte_nrf91_socket_bind(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno) {

    int res = -1;
    if (socket->u_param.domain == MOD_NETWORK_AF_INET) {
        printf("binding socket\n");
        struct nrf_sockaddr_in addr4;
        addr4.sin_len         = sizeof(struct nrf_sockaddr_in);
        addr4.sin_family      = NRF_AF_INET;
        addr4.sin_port        = NRF_HTONS(port);
        addr4.sin_addr.s_addr = (uint32_t)inet_addr(ip[0], ip[1], ip[2], ip[3]);
        res = nrf_bind(socket->u_param.fileno, (struct nrf_sockaddr * )&addr4, sizeof(struct nrf_sockaddr_in));
        printf("bind res: %d, %d\n", res, nrf_errno);
    } else if (socket->u_param.domain == MOD_NETWORK_AF_INET6) {
        struct nrf_sockaddr_in6 addr6;
        addr6.sin6_len         = sizeof(struct nrf_sockaddr_in6);
        addr6.sin6_family      = NRF_AF_INET6;
        addr6.sin6_port        = NRF_HTONS(port);
        memcpy((void *)addr6.sin6_addr.s6_addr, ip, sizeof(struct nrf_in6_addr));
        res = nrf_bind(socket->u_param.fileno, (struct nrf_sockaddr * )&addr6, sizeof(struct nrf_sockaddr_in6));
    }
    
    if (res != 0) {
        *_errno = nrf_errno;
        return -1;
    }

	return res;
}

STATIC int lte_nrf91_socket_listen(mod_network_socket_obj_t *socket, mp_int_t backlog, int *_errno) {
    int res = -1;
    printf("listen socket: %d, backlog: %d\n", socket->u_param.fileno, backlog);
    res = nrf_listen(socket->u_param.fileno, backlog);
    printf("listen res: %d, %d\n", res, nrf_errno);
    if (res != 0) {
        *_errno = nrf_errno;
        return -1;
    }

    return res;
}

STATIC int lte_nrf91_socket_accept(mod_network_socket_obj_t *socket, mod_network_socket_obj_t *socket2, byte *ip, mp_uint_t *port, int *_errno) {
    int res = -1;
    struct nrf_sockaddr_in addr4;
    struct nrf_sockaddr_in6 addr6;
    if (socket->u_param.domain == MOD_NETWORK_AF_INET) {
        nrf_socklen_t addr_len = sizeof(struct nrf_sockaddr_in);
        res = nrf_accept(socket->u_param.fileno, (struct nrf_sockaddr *)&addr4, &addr_len);
        printf("accept res: %d, %d\n", res, nrf_errno);
        if (res < 0) {
            *_errno = nrf_errno;
            return -1;
        }
        printf("Socket connected, on handle: %d\n", res);
    } else if (socket->u_param.domain == MOD_NETWORK_AF_INET6) {
        nrf_socklen_t addr_len = sizeof(struct nrf_sockaddr_in6);
        res = nrf_accept(socket->u_param.fileno, (struct nrf_sockaddr *)&addr6, &addr_len);
        
        if (res < 0) {
            *_errno = nrf_errno;
            return -1;
        }
    }
    
    socket2->u_param.fileno = res;

    if (socket->u_param.domain == MOD_NETWORK_AF_INET) {
        *port = NRF_NTOHS(addr4.sin_port);
        memcpy(ip, &addr4.sin_addr.s_addr, sizeof(struct nrf_in_addr));
    } else if (socket->u_param.domain == MOD_NETWORK_AF_INET6) {
        *port = NRF_NTOHS(addr6.sin6_port);
        memcpy(ip, (void *)addr6.sin6_addr.s6_addr, sizeof(struct nrf_in6_addr));
    }

    return res;
}

STATIC int lte_nrf91_socket_connect(mod_network_socket_obj_t *socket, byte *ip, mp_uint_t port, int *_errno) {
    int res = -1;
    if (socket->u_param.domain == MOD_NETWORK_AF_INET) {
        struct nrf_sockaddr_in addr4;
        addr4.sin_len         = sizeof(struct nrf_sockaddr_in);
        addr4.sin_family      = NRF_AF_INET;
        addr4.sin_port        = NRF_HTONS(port);
        addr4.sin_addr.s_addr = (uint32_t)inet_addr(ip[0], ip[1], ip[2], ip[3]);
        res = nrf_connect(socket->u_param.fileno, (struct nrf_sockaddr * )&addr4, sizeof(struct nrf_sockaddr_in));
    } else if (socket->u_param.domain == MOD_NETWORK_AF_INET6) {
        struct nrf_sockaddr_in6 addr6;
        addr6.sin6_len         = sizeof(struct nrf_sockaddr_in6);
        addr6.sin6_family      = NRF_AF_INET6;
        addr6.sin6_port        = NRF_HTONS(port);
        memcpy((void *)addr6.sin6_addr.s6_addr, ip, sizeof(struct nrf_in6_addr));
        res = nrf_connect(socket->u_param.fileno, (struct nrf_sockaddr * )&addr6, sizeof(struct nrf_sockaddr_in6));
    } else {
        // If AF_LTE, SOCK_MGMT, PROTO_PDN. Or, similar, pass it raw.
        res = nrf_connect(socket->u_param.fileno, ip, port);
    }

    if (res == -1) {
        *_errno = nrf_errno;
        return -1;
    }

	return res;
}

STATIC mp_uint_t lte_nrf91_socket_send(mod_network_socket_obj_t *socket, const byte *buf, mp_uint_t len, int *_errno) {
    mp_int_t res = nrf_send(socket->u_param.fileno, buf, len, 0);
    
    if (res < 0) {
        *_errno = nrf_errno;
        return -1;
    }

    return res;
}

STATIC mp_uint_t lte_nrf91_socket_recv(mod_network_socket_obj_t *socket, byte *buf, mp_uint_t len, int *_errno) {
    mp_int_t res = nrf_recv(socket->u_param.fileno, buf, len, 0);

    if (res < 0) {
        *_errno = nrf_errno;
        return -1;
    }

    return res;
}

STATIC mp_uint_t lte_nrf91_socket_sendto(mod_network_socket_obj_t *socket, const byte *buf, mp_uint_t len, byte *ip, mp_uint_t port, int *_errno) {

    struct nrf_sockaddr * p_servaddr;
    nrf_socklen_t   addrlen;
    struct nrf_sockaddr_in addr4;
    struct nrf_sockaddr_in6 addr6;

    // Make sure something is assigned.
    p_servaddr = (struct nrf_sockaddr *)&addr4;
    addrlen = sizeof(struct nrf_sockaddr_in);

    if (socket->u_param.domain == MOD_NETWORK_AF_INET) {    
        addr4.sin_len         = sizeof(struct nrf_sockaddr_in);
        addr4.sin_family      = NRF_AF_INET;
        addr4.sin_port        = NRF_HTONS(port);
        addr4.sin_addr.s_addr = (uint32_t)inet_addr(ip[0], ip[1], ip[2], ip[3]);
        p_servaddr = (struct nrf_sockaddr *)&addr4;
        addrlen = sizeof(struct nrf_sockaddr_in);
    } else if (socket->u_param.domain == MOD_NETWORK_AF_INET6) {
        addr6.sin6_len         = sizeof(struct nrf_sockaddr_in6);
        addr6.sin6_family      = NRF_AF_INET6;
        addr6.sin6_port        = NRF_HTONS(port);
        memcpy((void *)addr6.sin6_addr.s6_addr, ip, sizeof(struct nrf_in6_addr));
        p_servaddr = (struct nrf_sockaddr *)&addr6;
        addrlen = sizeof(struct nrf_sockaddr_in6);
/*
        printf("dest: %02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x:%02x%02x\n",
		       ip[0],  ip[1],  ip[2],  ip[3],
		       ip[4],  ip[5],  ip[6],  ip[7],
		       ip[8],  ip[9],  ip[10], ip[11],
		       ip[12], ip[13], ip[14], ip[15]);
*/
    }

    mp_int_t res = nrf_sendto(socket->u_param.fileno,
                              buf,
                              len,
                              0,
                              p_servaddr,
                              addrlen);
    
    if (res < 0) {
        *_errno = nrf_errno;
        return -1;
    }

    return res;
}

STATIC mp_uint_t lte_nrf91_socket_recvfrom(mod_network_socket_obj_t *socket, byte *buf, mp_uint_t len, byte *ip, mp_uint_t *port, int *_errno) {
    struct nrf_sockaddr * p_servaddr;
    nrf_socklen_t   addrlen;
    struct nrf_sockaddr_in addr4;
    struct nrf_sockaddr_in6 addr6;

    // Make sure something is assigned.
    p_servaddr = (struct nrf_sockaddr *)&addr4;

    if (socket->u_param.domain == MOD_NETWORK_AF_INET) {    
        p_servaddr = (struct nrf_sockaddr *)&addr4;
        addrlen = sizeof(struct nrf_sockaddr_in);
    } else if (socket->u_param.domain == MOD_NETWORK_AF_INET6) {
        p_servaddr = (struct nrf_sockaddr *)&addr6;
        addrlen = sizeof(struct nrf_sockaddr_in6);
    }

    mp_int_t res = nrf_recvfrom(socket->u_param.fileno,
                              buf,
                              len,
                              NRF_MSG_WAITALL,
                              p_servaddr,
                              &addrlen);
    if (res < 0) {
        *_errno = nrf_errno;
        return -1;
    }
    
    if (socket->u_param.domain == MOD_NETWORK_AF_INET) {    
        *port = NRF_NTOHS(addr4.sin_port);
        memcpy(ip, &addr4.sin_addr.s_addr, sizeof(struct nrf_in_addr));
    } else if (socket->u_param.domain == MOD_NETWORK_AF_INET6) {
        *port = NRF_NTOHS(addr6.sin6_port);
        memcpy(ip, (void *)addr6.sin6_addr.s6_addr, sizeof(struct nrf_in6_addr));
    }

    return res;
}

STATIC int lte_nrf91_socket_getsockopt(mod_network_socket_obj_t *socket, mp_uint_t level, mp_uint_t opt, void *optval, mp_uint_t *optlen, int *_errno) {
    nrf_socklen_t length = *optlen;
    printf("level: %u, opt: %u, len: %lu\n", level, opt, length);
    mp_int_t res = nrf_getsockopt(socket->u_param.fileno, level, opt, optval, &length);
    printf("res: %u, length: %lu, value: %lu\n", res, length, (uint32_t)optval);
    *optlen = length;

    if (res < 0) {
        *_errno = nrf_errno;
        return -1;
    }

    return res;
}

STATIC int lte_nrf91_socket_setsockopt(mod_network_socket_obj_t *socket, mp_uint_t level, mp_uint_t opt, const void *optval, mp_uint_t optlen, int *_errno) {
    printf("level: %u, opt: %u, len: %u\n", level, opt, optlen);
    printf("optval: %lu\n", *((uint32_t*)optval));
    printf("socket: %u\n", socket->u_param.fileno);
    mp_int_t res = nrf_setsockopt(socket->u_param.fileno, level, opt, optval, optlen);
    if (res < 0) {
        *_errno = nrf_errno;
        return -1;
    }

    return res;
}

STATIC int lte_nrf91_socket_settimeout(mod_network_socket_obj_t *socket, mp_uint_t timeout_ms, int *_errno) {
    printf("level: %u, opt: %u, len: %u\n", NRF_SOL_SOCKET, NRF_SO_RCVTIMEO, sizeof(mp_uint_t));
    printf("socket: %u\n", socket->u_param.fileno);
    mp_int_t res = nrf_setsockopt(socket->u_param.fileno, NRF_SOL_SOCKET, NRF_SO_RCVTIMEO, &timeout_ms, sizeof(mp_uint_t));
    if (res < 0) {
        *_errno = nrf_errno;
        return -1;
    }

    return res;
}

STATIC int lte_nrf91_socket_ioctl(mod_network_socket_obj_t *socket, mp_uint_t request, mp_uint_t arg, int *_errno) {
    switch (request) {
        case MP_STREAM_GET_FILENO:
            return socket->u_param.fileno;
        default:
            *_errno = MP_EINVAL;
            return -1;
    }
}

char * at_ltrim(char * str, char delimiter)
{
    char * result = str;
    result = strchr(result, delimiter);
    if (result != NULL)
    {
        return ++result;
    }
    return str;
}

uint8_t at_param_count_get(char * original_str) {
    char *result = original_str;
    char target = ',';
    uint8_t count = 0;
    result = strchr(result, target);
    while (result != NULL) {
        count++;
        result = strchr(result+1, target);
    }

    if (count > 0) {
        count += 1; // include last param after last comma.
    }

    return count;
}

char * at_strtok(char * str, uint16_t *len, char delimiter) {
    char *result = str;
    result = strchr(result, delimiter);
    if (result != NULL) {
        *len = (uint32_t)result - (uint32_t)str;
        return str;
    }
    return NULL;
}

char * at_token_get(uint8_t index, char * str, uint16_t *len)
{
    uint8_t count = at_param_count_get(str);

    if (count < 1) {
        return NULL;
    }

    uint8_t max_index = count - 1;

    uint16_t length = 0;
    char * token = str;
    for (uint8_t i = 0; i <= max_index; i++) {
        if ((i == 0) && (i == max_index))
        {
            // First token and last.
            token = at_ltrim(token, ':');
            token = at_ltrim(token, ' ');
            token = at_strtok(token, &length, '\r');
        } else if (i == 0)  {
            // First token.
            token = at_ltrim(token, ':');
            token = at_ltrim(token, ' ');
            token = at_strtok(token, &length, ',');
        } else if (i == max_index) {
            // Last token.
            token = at_ltrim(token + length, ',');
            token = at_strtok(token, &length, '\r');
        } else {
            token = at_ltrim(token + length, ',');
            token = at_strtok(token, &length, ',');
        }

        if (i == index) {
            *len = length;
            return token;
        }
    }
    
    return NULL;
}

/******************************************************************************/
// Micro Python bindings

/// \classmethod \constructor
/// Create and return a nrf91 LTE object.
STATIC mp_obj_t lte_nrf91_make_new(const mp_obj_type_t *type, mp_uint_t n_args, mp_uint_t n_kw, const mp_obj_t *args) {
    // check arguments
    //mp_arg_check_num(n_args, n_kw, 0, 0, false);

    // init the nrf91 LTE object
    lte_nrf91_obj.base.type = (mp_obj_type_t*)&mod_network_nic_type_nrf91;
    
    // blocking call to turn on LTE modem.
    bsd_init();

    // regiser NIC with network module
    mod_network_register_nic(&lte_nrf91_obj);

    // return nrf91 LTE object
    return &lte_nrf91_obj;
}

STATIC mp_obj_t lte_nrf91_mode(size_t n_args, const mp_obj_t *args) {
    mod_network_nic_type_t *self = args[0];
    (void)self;
    if (n_args == 1) {
        // return active mode.
    } else {
        // set requested mode.
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lte_nrf91_mode_obj, 1, 2, lte_nrf91_mode);

static char m_result_buffer[512];

STATIC bool send_at_command(int handle, uint8_t * cmd, uint16_t cmd_len)
{
    memset(m_result_buffer, 0, MP_ARRAY_SIZE(m_result_buffer));
//    printf("Issued %s \n", cmd);
    int written = 0;
    written = nrf_send(handle, cmd, cmd_len, 0);
    if (written != cmd_len) {
        return false;
    }
    
    int read = 0;
    while (true) {
        read = nrf_recv(handle, m_result_buffer, MP_ARRAY_SIZE(m_result_buffer), NRF_MSG_WAITALL);
        if (read >= 2) {        
            if (strstr(m_result_buffer, "OK\r\n\0") != NULL)
            {
//                printf("%s OK\n", cmd);
                break;
            }
        }
    }
    return true;
}

int send_at_command_with_result(int handle, uint8_t * cmd, uint16_t cmd_len, char * res_buffer, uint16_t res_buffer_len)
{
//    printf("Issued %s \n", cmd);
    int written = 0;
    written = nrf_send(handle, cmd, cmd_len, 0);
    if (written != cmd_len) {
        return false;
    }
    
    int read = 0;
    while (true) {
        read = nrf_recv(handle, res_buffer, res_buffer_len, NRF_MSG_WAITALL);
        if (read >= 2) {        
            if (strstr((char *)res_buffer, "OK\r\n\0") != NULL)
            {
//                printf("%s OK\n", cmd);
                break;
            }
            return -1;
        }
    }
    return read;
}

STATIC mp_obj_t lte_nrf91_connect(mp_obj_t self_in) {
    int handle = nrf_socket(NRF_AF_LTE, 0, NRF_PROTO_AT);
    if (handle < 0)
    {
        return mp_const_false;
    }

    bool success;

    int timeout_ms = 1000;
    int sock_opt_result = nrf_setsockopt(handle, NRF_SOL_SOCKET, NRF_SO_RCVTIMEO, &timeout_ms, sizeof(int));
    if (sock_opt_result < 0) {
        (void)nrf_close(handle);
        return mp_const_false;
    }

    static const char at_cfun4[] = "AT+CFUN=4";
    success = send_at_command(handle, (uint8_t *)at_cfun4, MP_ARRAY_SIZE(at_cfun4));
    if (!success) {
        printf("Error issuing AT+CFUN=4, errno: %d\n", nrf_errno);
        (void)nrf_close(handle);
        return mp_const_false;
    }

    static const char at_cereg[] = "AT+CEREG=2";
    success = send_at_command(handle, (uint8_t *)at_cereg, MP_ARRAY_SIZE(at_cereg));
    if (!success) {
        printf("Error issuing AT+CEREG=2, errno: %d\n", nrf_errno);
        (void)nrf_close(handle);
        return mp_const_false;
    }

    static const char at_cfun1[] = "AT+CFUN=1";
    success = send_at_command(handle, (uint8_t *)at_cfun1, MP_ARRAY_SIZE(at_cfun1));
    if (!success) {
        printf("Error issuing AT+CFUN=1, errno: %d\n", nrf_errno);
        (void)nrf_close(handle);
        return mp_const_false;
    }
    
    memset(m_result_buffer, 0, MP_ARRAY_SIZE(m_result_buffer));
    
    uint16_t num_of_tries = 1;

    while (true) {
        //printf("issue recv of +CEREG on handle %d\n", handle);
        int read = nrf_recv(handle, m_result_buffer, MP_ARRAY_SIZE(m_result_buffer), 0);
        //printf("read content\n");
        if (read >= 9) {
            if (strncmp(m_result_buffer, "+CEREG: 2", 9) == 0)
            {
                printf("\nCell Found\r\n");
                break;
            }
        }
        printf("Search for cell, tried for %d seconds\r", num_of_tries++);
    }

    (void)nrf_close(handle);

    return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lte_nrf91_connect_obj, lte_nrf91_connect);

STATIC mp_obj_t lte_nrf91_disconnect(mp_obj_t self_in) {
    int handle = nrf_socket(NRF_AF_LTE, 0, NRF_PROTO_AT);
    if (handle < 0)
    {
        return mp_const_false;
    }

    bool success;

    static const char at_cfun4[] = "AT+CFUN=4";
    success = send_at_command(handle, (uint8_t *)at_cfun4, MP_ARRAY_SIZE(at_cfun4));
    if (!success) {
        printf("Error issuing AT+CFUN=4, errno: %d\n", nrf_errno);
        (void)nrf_close(handle);
        return mp_const_false;
    }

    static const char at_cfun1[] = "AT+CFUN=0";
    success = send_at_command(handle, (uint8_t *)at_cfun1, MP_ARRAY_SIZE(at_cfun1));
    if (!success) {
        printf("Error issuing AT+CFUN=0, errno: %d\n", nrf_errno);
        (void)nrf_close(handle);
        return mp_const_false;
    }

    (void)nrf_close(handle);

    return mp_const_true;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lte_nrf91_disconnect_obj, lte_nrf91_disconnect);

STATIC mp_obj_t lte_nrf91_ifconfig(mp_uint_t n_args, const mp_obj_t *args) {
    int handle = nrf_socket(NRF_AF_LTE, 0, NRF_PROTO_AT);
    if (handle < 0)
    {
        return mp_const_false;
    }

    mp_obj_t ret_list = mp_obj_new_list(0, NULL);

    if (mp_obj_is_int(args[1]) && mp_obj_get_int(args[1]) == 1) {
        static const char at_cgcontrdp[] = "AT+CGCONTRDP";
        static char at_cgcontrdp_numbered[MP_ARRAY_SIZE(at_cgcontrdp) + 3];

        for (uint8_t cid = 0; cid < 12; cid++)
        {
            memset(m_result_buffer, 0, MP_ARRAY_SIZE(m_result_buffer));
            snprintf(at_cgcontrdp_numbered,
                    MP_ARRAY_SIZE(at_cgcontrdp_numbered),
                    "%s=%u",
                    at_cgcontrdp, cid);
            int len = send_at_command_with_result(handle, (uint8_t *)at_cgcontrdp_numbered, MP_ARRAY_SIZE(at_cgcontrdp_numbered), m_result_buffer, MP_ARRAY_SIZE(m_result_buffer));

            char * buffer = m_result_buffer;
            int remaining_len = len;

            while ((remaining_len > 10) && (buffer != NULL)) {
                uint16_t cid_length = 0;
                uint16_t apn_length = 0;
                uint16_t dns_prim_addr_length = 0;
                uint16_t dns_sec_addr_length = 0;
                uint16_t ipv4_mtu_size_length = 0;

                char * p_cid = at_token_get(0, buffer, &cid_length);
                char * p_apn = at_token_get(2, buffer, &apn_length);
                char * p_dns_prim_addr = at_token_get(5, buffer, &dns_prim_addr_length);
                char * p_dns_sec_addr = at_token_get(6, buffer, &dns_sec_addr_length);
                char * p_ipv4_mtu_size = at_token_get(11, buffer, &ipv4_mtu_size_length);

                mp_obj_tuple_t *tuple = mp_obj_new_tuple(5, NULL);
                if ((p_cid != NULL) && (cid_length > 0))  {
                    tuple->items[0] = mp_obj_new_str(p_cid, cid_length);
                }
                if ((p_apn != NULL) && (apn_length > 1)) {
                    tuple->items[1] = mp_obj_new_str(p_apn + 1, apn_length - 2); // Hack to get rid of quotes
                } else {
                    tuple->items[1] = mp_const_none;
                }
                if ((p_dns_prim_addr != NULL) && (dns_prim_addr_length > 7)) {
                    tuple->items[2] = mp_obj_new_str(p_dns_prim_addr + 1, dns_prim_addr_length - 2); // Hack to get rid of quotes
                } else {
                    tuple->items[2] = mp_const_none;
                }
                if ((p_dns_sec_addr != NULL) && (dns_sec_addr_length > 7)) {
                    tuple->items[3] = mp_obj_new_str(p_dns_sec_addr + 1, dns_sec_addr_length - 2); // Hack to get rid of quotes
                } else {
                    tuple->items[3] = mp_const_none;
                }
                if ((p_ipv4_mtu_size != NULL) && (ipv4_mtu_size_length > 0)) {
                    tuple->items[4] = mp_obj_new_int(atoi(p_ipv4_mtu_size));
                } else {
                    tuple->items[4] = mp_const_none;
                }
                mp_obj_list_append(ret_list, tuple);

                buffer = strstr(buffer, "\r\n");
                if (buffer != NULL) {
                    buffer += 2; // Increment \r\n.
                    uint32_t offset = ((uint32_t)buffer - (uint32_t)&m_result_buffer[0]);
                    remaining_len = len - offset;
                }
            }
        }
    } else {
        static const char at_cgpaddr[] = "AT+CGPADDR";
        static char at_cgpaddr_numbered[MP_ARRAY_SIZE(at_cgpaddr) + 3];

        for (uint8_t cid = 0; cid < 12; cid++)
        {
            memset(m_result_buffer, 0, MP_ARRAY_SIZE(m_result_buffer));
            snprintf(at_cgpaddr_numbered,
                    MP_ARRAY_SIZE(at_cgpaddr_numbered),
                    "%s=%u",
                    at_cgpaddr, cid);

            int len = send_at_command_with_result(handle, (uint8_t *)at_cgpaddr_numbered, MP_ARRAY_SIZE(at_cgpaddr_numbered), m_result_buffer, MP_ARRAY_SIZE(m_result_buffer));
            if (len > 6) {
                uint16_t cid_length = 0;
                uint16_t ipv4_length = 0;
                uint16_t ipv6_length = 0;

                char * p_cid = at_token_get(0, m_result_buffer, &cid_length);
                char * p_ipv4 = at_token_get(1, m_result_buffer, &ipv4_length);
                char * p_ipv6 = at_token_get(2, m_result_buffer, &ipv6_length);

                mp_obj_tuple_t *tuple = mp_obj_new_tuple(3, NULL);
                if ((p_cid != NULL) && (cid_length > 0))  {
                    tuple->items[0] = mp_obj_new_str(p_cid, cid_length);
                }
                if ((p_ipv4 != NULL) && (ipv4_length > 7)) {
                    tuple->items[1] = mp_obj_new_str(p_ipv4 + 1, ipv4_length - 2); // Hack to get rid of quotes
                } else {
                    tuple->items[1] = mp_const_none;
                }
                if ((p_ipv6 != NULL) && (ipv6_length > 4)) {
                    tuple->items[2] = mp_obj_new_str(p_ipv6 + 1, ipv6_length - 2); // Hack to get rid of quotes
                } else {
                    tuple->items[2] = mp_const_none;
                }

                mp_obj_list_append(ret_list, tuple);
            }
        }
    }
    (void)nrf_close(handle);

    return ret_list;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lte_nrf91_ifconfig_obj, 1, 2, lte_nrf91_ifconfig);

STATIC mp_obj_t lte_nrf91_version(mp_obj_t self_in) {
    int handle = nrf_socket(NRF_AF_LTE, 0, NRF_PROTO_AT);
    if (handle < 0)
    {
        return mp_const_false;
    }
    
    static const char at_cgmr[] = "AT+CGMR";
    int len = send_at_command_with_result(handle, (uint8_t *)at_cgmr, MP_ARRAY_SIZE(at_cgmr), m_result_buffer, MP_ARRAY_SIZE(m_result_buffer));
    
    (void)nrf_close(handle);

    if (len == -1) {
        return mp_const_false;
    }
    
    return mp_obj_new_str(m_result_buffer, len - 7);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(lte_nrf91_version_obj, lte_nrf91_version);

STATIC mp_obj_t lte_nrf91_apn_class(mp_uint_t n_args, const mp_obj_t *args) {
    mp_obj_t retval = mp_const_none;
    if (n_args == 2)
    {
        if (mp_obj_is_int(args[1])) {
            uint8_t * p_buffer;
            uint16_t buffer_len = 64;

            p_buffer = m_new(uint8_t, buffer_len);
            memset(p_buffer, 0, buffer_len);

            int result = nrf_apn_class_read(mp_obj_get_int(args[1]), p_buffer, &buffer_len);

            if ((result == 0) && (buffer_len > 0)) {
                retval = mp_obj_new_str((char *)p_buffer, buffer_len);
            }

            m_free(p_buffer);
        }
    } else if (n_args == 3) {
        if (mp_obj_is_int(args[1]) && mp_obj_is_str(args[2])) {
            mp_uint_t apn_name_len;
            const char *apn_name = mp_obj_str_get_data(args[2], &apn_name_len);
            int result = nrf_apn_class_update(mp_obj_get_int(args[1]), (uint8_t *)apn_name, apn_name_len);
            if (result == 0) {
                retval = mp_const_true;
            } else {
                retval = mp_const_false;
            }
        }
    }
    return retval;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(lte_nrf91_apn_class_obj, 2, 3, lte_nrf91_apn_class);

STATIC const mp_rom_map_elem_t lte_nrf91_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_mode),                MP_ROM_PTR(&lte_nrf91_mode_obj) },
    { MP_ROM_QSTR(MP_QSTR_connect),             MP_ROM_PTR(&lte_nrf91_connect_obj) },
    { MP_ROM_QSTR(MP_QSTR_disconnect),          MP_ROM_PTR(&lte_nrf91_disconnect_obj) },
    { MP_ROM_QSTR(MP_QSTR_ifconfig),            MP_ROM_PTR(&lte_nrf91_ifconfig_obj) },
    { MP_ROM_QSTR(MP_QSTR_version),             MP_ROM_PTR(&lte_nrf91_version_obj) },
    { MP_ROM_QSTR(MP_QSTR_apn_class),           MP_ROM_PTR(&lte_nrf91_apn_class_obj) },
};
STATIC MP_DEFINE_CONST_DICT(lte_nrf91_locals_dict, lte_nrf91_locals_dict_table);


const mod_network_nic_type_t mod_network_nic_type_nrf91 = {
    .base = {
        { &mp_type_type },
        .name = MP_QSTR_LTE,
        .make_new = lte_nrf91_make_new,
        .locals_dict = (mp_obj_t)&lte_nrf91_locals_dict,
    },
    .gethostbyname = lte_nrf91_gethostbyname,
    .socket = lte_nrf91_socket_socket,
    .close = lte_nrf91_socket_close,
    .bind = lte_nrf91_socket_bind,
    .listen = lte_nrf91_socket_listen,
    .accept = lte_nrf91_socket_accept,
    .connect = lte_nrf91_socket_connect,
    .send = lte_nrf91_socket_send,
    .recv = lte_nrf91_socket_recv,
    .sendto = lte_nrf91_socket_sendto,
    .recvfrom = lte_nrf91_socket_recvfrom,
    .getsockopt = lte_nrf91_socket_getsockopt,
    .setsockopt = lte_nrf91_socket_setsockopt,
    .settimeout = lte_nrf91_socket_settimeout,
    .ioctl = lte_nrf91_socket_ioctl,
};

#endif
