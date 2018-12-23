/**
 * \file asn1.h
 *
 * \brief Generic ASN.1 parsing
 */
/*
 *  Copyright (C) 2006-2015, ARM Limited, All Rights Reserved
 *  SPDX-License-Identifier: Apache-2.0
 *
 *  Licensed under the Apache License, Version 2.0 (the "License"); you may
 *  not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 *  WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 *
 *  This file is part of mbed TLS (https://tls.mbed.org)
 */

#include <stddef.h>
#include <string.h>

#define MBEDTLS_SHA256_C 1

/**
 * Type-length-value structure that allows for ASN1 using DER.
 */
typedef struct mbedtls_asn1_buf
{
    int tag;                /**< ASN1 type, e.g. MBEDTLS_ASN1_UTF8_STRING. */
    size_t len;             /**< ASN1 length, in octets. */
    unsigned char *p;       /**< ASN1 data, e.g. in ASCII. */
}
mbedtls_asn1_buf;


/**
 * Container for a sequence or list of 'named' ASN.1 data items
 */
typedef struct mbedtls_asn1_named_data
{
    mbedtls_asn1_buf oid;                   /**< The object identifier. */
    mbedtls_asn1_buf val;                   /**< The named value. */
    struct mbedtls_asn1_named_data *next;  /**< The next entry in the sequence. */
    unsigned char next_merged;      /**< Merge next item into the current one? */
}
mbedtls_asn1_named_data;


mbedtls_asn1_named_data *mbedtls_asn1_find_named_data( mbedtls_asn1_named_data *list,
                                       const char *oid, size_t len )
{
    while( list != NULL )
    {
        if( list->oid.len == len &&
            memcmp( list->oid.p, oid, len ) == 0 )
        {
            break;
        }

        list = list->next;
    }

    return( list );
}


typedef enum {
    MBEDTLS_MD_NONE=0,    /**< None. */
    MBEDTLS_MD_MD2,       /**< The MD2 message digest. */
    MBEDTLS_MD_MD4,       /**< The MD4 message digest. */
    MBEDTLS_MD_MD5,       /**< The MD5 message digest. */
    MBEDTLS_MD_SHA1,      /**< The SHA-1 message digest. */
    MBEDTLS_MD_SHA224,    /**< The SHA-224 message digest. */
    MBEDTLS_MD_SHA256,    /**< The SHA-256 message digest. */
    MBEDTLS_MD_SHA384,    /**< The SHA-384 message digest. */
    MBEDTLS_MD_SHA512,    /**< The SHA-512 message digest. */
    MBEDTLS_MD_RIPEMD160, /**< The RIPEMD-160 message digest. */
} mbedtls_md_type_t;

/**
 * Message digest information.
 * Allows message digest functions to be called in a generic way.
 */
struct mbedtls_md_info_t
{
    /** Digest identifier */
    mbedtls_md_type_t type;

    /** Name of the message digest */
    const char * name;

    /** Output length of the digest function in bytes */
    int size;

    /** Block length of the digest function in bytes */
    int block_size;

    /** Digest initialisation function */
    int (*starts_func)( void *ctx );

    /** Digest update function */
    int (*update_func)( void *ctx, const unsigned char *input, size_t ilen );

    /** Digest finalisation function */
    int (*finish_func)( void *ctx, unsigned char *output );

    /** Generic digest function */
    int (*digest_func)( const unsigned char *input, size_t ilen,
                        unsigned char *output );

    /** Allocate a new context */
    void * (*ctx_alloc_func)( void );

    /** Free the given context */
    void (*ctx_free_func)( void *ctx );

    /** Clone state from a context */
    void (*clone_func)( void *dst, const void *src );

    /** Internal use only */
    int (*process_func)( void *ctx, const unsigned char *input );
};

typedef struct mbedtls_md_info_t mbedtls_md_info_t;

const mbedtls_md_info_t mbedtls_sha224_info = {
    MBEDTLS_MD_SHA224,
    "SHA224",
    28,
    64,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

const mbedtls_md_info_t mbedtls_sha256_info = {
    MBEDTLS_MD_SHA256,
    "SHA256",
    32,
    64,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL,
};

#if defined(MBEDTLS_MD2_C)
extern const mbedtls_md_info_t mbedtls_md2_info;
#endif
#if defined(MBEDTLS_MD4_C)
extern const mbedtls_md_info_t mbedtls_md4_info;
#endif
#if defined(MBEDTLS_MD5_C)
extern const mbedtls_md_info_t mbedtls_md5_info;
#endif
#if defined(MBEDTLS_RIPEMD160_C)
extern const mbedtls_md_info_t mbedtls_ripemd160_info;
#endif
#if defined(MBEDTLS_SHA1_C)
extern const mbedtls_md_info_t mbedtls_sha1_info;
#endif
#if defined(MBEDTLS_SHA256_C)
extern const mbedtls_md_info_t mbedtls_sha224_info;
extern const mbedtls_md_info_t mbedtls_sha256_info;
#endif
#if defined(MBEDTLS_SHA512_C)
extern const mbedtls_md_info_t mbedtls_sha384_info;
extern const mbedtls_md_info_t mbedtls_sha512_info;
#endif



unsigned char mbedtls_md_get_size( const mbedtls_md_info_t *md_info )
{
    if( md_info == NULL )
        return( 0 );

    return md_info->size;
}



const mbedtls_md_info_t *mbedtls_md_info_from_type( mbedtls_md_type_t md_type )
{
    switch( md_type )
    {
#if defined(MBEDTLS_MD2_C)
        case MBEDTLS_MD_MD2:
            return( &mbedtls_md2_info );
#endif
#if defined(MBEDTLS_MD4_C)
        case MBEDTLS_MD_MD4:
            return( &mbedtls_md4_info );
#endif
#if defined(MBEDTLS_MD5_C)
        case MBEDTLS_MD_MD5:
            return( &mbedtls_md5_info );
#endif
#if defined(MBEDTLS_RIPEMD160_C)
        case MBEDTLS_MD_RIPEMD160:
            return( &mbedtls_ripemd160_info );
#endif
#if defined(MBEDTLS_SHA1_C)
        case MBEDTLS_MD_SHA1:
            return( &mbedtls_sha1_info );
#endif
#if defined(MBEDTLS_SHA256_C)
        case MBEDTLS_MD_SHA224:
            return( &mbedtls_sha224_info );
        case MBEDTLS_MD_SHA256:
            return( &mbedtls_sha256_info );
#endif
#if defined(MBEDTLS_SHA512_C)
        case MBEDTLS_MD_SHA384:
            return( &mbedtls_sha384_info );
        case MBEDTLS_MD_SHA512:
            return( &mbedtls_sha512_info );
#endif
        default:
            return( NULL );
    }
}

