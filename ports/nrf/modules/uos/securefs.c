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

#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>

#include "py/nlr.h"
#include "py/obj.h"
#include "py/stream.h"
#include "py/runtime.h"
#include "py/parsenum.h"
#include "mpconfigport.h"

#if MICROPY_SECURITY_FS

#include "nrf_inbuilt_key.h"
#include "securefs.h"
#include "extmod/vfs.h"

#define DEBUG_FILE 0
#if DEBUG_FILE
#define DEBUG printf
#else
#define DEBUG(...)
#endif

#define FILE_NOT_FOUND ((uint8_t)-1)

extern int nrf_errno;

typedef struct _mp_obj_uos_secfs_t {
    mp_obj_base_t base;
    vstr_t root;
    size_t root_len;
    uint8_t dir;
    bool readonly;
} mp_obj_uos_secfs_t;

typedef struct _file_descriptor_obj {
    mp_obj_base_t base;
    nrf_sec_tag_t tag;
    nrf_key_mgnt_cred_type_t type;
    uint16_t seek;
    uint16_t size;
    bool open;
    bool writable;
    vstr_t content;
} file_descriptor_obj;

STATIC const char *uos_secfs_get_path_str(mp_obj_uos_secfs_t *self, mp_obj_t path) {
    if (self->root_len == 0) {
        return mp_obj_str_get_str(path);
    } else {
        self->root.len = self->root_len;
        vstr_add_str(&self->root, mp_obj_str_get_str(path));
        return vstr_null_terminated_str(&self->root);
    }
}

void securefs_filesystem_init(void) {
}

STATIC mp_uint_t securefs_file_read(mp_obj_t obj, void *buf, mp_uint_t size, int *errcode) {
    file_descriptor_obj *self = (file_descriptor_obj *)obj;
    printf("seek: %u, size: %u\n", self->seek, self->size); 
    if (self->seek >= self->size) {
        //*errcode = EBADF;
        //return MP_STREAM_ERROR;
        return 0;
    }
    
    char * content = vstr_null_terminated_str(&self->content);
    content += self->seek;

    uint32_t to_read = MIN(strlen(content), size);
    memcpy(buf, content, to_read);
    self->seek += to_read;
    return to_read;
}

STATIC mp_uint_t securefs_file_write(mp_obj_t obj, const void *buf, mp_uint_t size, int *errcode) {
    file_descriptor_obj *self = (file_descriptor_obj *)obj;
    if (self->writable) {
        if (self->size == 0) {
            vstr_init(&self->content, size);
            vstr_add_str(&self->content, buf);
            self->size += size;
        } else {
            char * p = vstr_extend(&self->content, size);
            memcpy(p, buf, size);
        }
    }
    DEBUG("writing file\n");
    return size;
}

STATIC void securefs_file_close(file_descriptor_obj *self) {
    if (self->writable) {
        int res = nrf_inbuilt_key_write(self->tag,
                                        self->type,
                                        (uint8_t *)self->content.buf,
                                        self->size);
        (void)res;
        DEBUG("wrote file: %lu, size: %u, res: %d\n", self->tag, self->size, res);
    }
    self->open = false;
}

STATIC mp_obj_t uos_secfs_file_name(mp_obj_t self_in) {
    file_descriptor_obj *self = (file_descriptor_obj*)self_in;
    (void)self;
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uos_secfs_file_name_obj, uos_secfs_file_name);

STATIC mp_obj_t uos_secfs_file_close(mp_obj_t self_in) {
    file_descriptor_obj *self = (file_descriptor_obj*)self_in;
    securefs_file_close(self);
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uos_secfs_file_close_obj, uos_secfs_file_close);

#if MICROPY_VFS
STATIC mp_obj_t uos_secfs_remove(mp_obj_t self, mp_obj_t name) {
    return mp_const_none; // securefs_remove(name);
}
MP_DEFINE_CONST_FUN_OBJ_2(uos_secfs_remove_obj, uos_secfs_remove);
#else
STATIC mp_obj_t uos_secfs_remove(mp_obj_t name) {
    return mp_const_none; // securefs_remove(name);
}
MP_DEFINE_CONST_FUN_OBJ_1(uos_secfs_remove_obj, uos_secfs_remove);
#endif

typedef struct _uos_secfs_ilistdir_it_t {
    mp_obj_base_t base;
    mp_fun_1_t iternext;
    bool is_str;
    uint8_t index;
    uint8_t dir;
    uint8_t level;
} uos_secfs_ilistdir_it_t;

const char* secfs_dirs_arr[] = {"ca", "pub-cert", "priv-cert", "psk", "identity", "pub-key"};

extern int send_at_command_with_result(int handle, uint8_t * cmd, uint16_t cmd_len, char * res_buffer, uint16_t res_buffer_len);

uint8_t num_of_entries(char * buffer, uint16_t buffer_len) {
    const char * str = buffer;
    const char * entry = NULL;
    uint8_t count = 0;
    do {
        entry = strstr(str, "%CMNG");
        if (entry != NULL) {
            count++;
            entry++; // increment once to proceed.
            str = entry;
        }
    } while ((entry != NULL) && ((uint32_t)str < (uint32_t)buffer + buffer_len));
    return count;
}

extern char * at_token_get(uint8_t index, char * str, uint16_t *len);

char * sec_tag_get(uint8_t index, char * buffer, uint16_t buffer_len, uint16_t * out_len) {
    const char * str = buffer;
    char * entry = NULL;
    uint8_t count = 0;
    do {
        entry = strstr(str, "%CMNG");
        if (entry != NULL) {
            if (count == index) {
                uint16_t length = 0;
                char * result = at_token_get(0, entry, &length);
                *out_len = length;
                return result;
            }
            count++;
            entry++; // increment once to proceed.
            str = entry;

        }
    } while ((entry != NULL) && ((uint32_t)str < (uint32_t)buffer + buffer_len));
    return NULL;
}

STATIC mp_obj_t uos_secfs_ilistdir_it_iternext(mp_obj_t self_in) {
    uos_secfs_ilistdir_it_t *self = MP_OBJ_TO_PTR(self_in);
//    printf("iterating, level %u, index %u, dir %u\n", self->level, self->index, self->dir);
//    printf("0x%lx\n\r", (uint32_t)self);

    for (;;) {
        if (self->level == 0)
        {
            if (self->index >= 6)
            {
                break;
            }
            mp_obj_t name = mp_obj_new_str(secfs_dirs_arr[self->index], strlen(secfs_dirs_arr[self->index]));

            // make 3-tuple with info about this entry
            mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(3, NULL));
            t->items[0] = name;
            t->items[1] = MP_OBJ_NEW_SMALL_INT(MP_S_IFDIR); // all entries are directories
            t->items[2] = MP_OBJ_NEW_SMALL_INT(0); // no inode number
            self->index++;
            return MP_OBJ_FROM_PTR(t);
        } else if (self->level == 1) {
            int handle = nrf_socket(NRF_AF_LTE, 0, NRF_PROTO_AT);
            if (handle < 0)
            {
                printf("COuld not create socket for AT: %d\n", nrf_errno);
                return MP_OBJ_STOP_ITERATION;
            }

            static const char at_cmng_list[] = "AT%CMNG=1,,";
            static char m_result_buffer[256];
            static char at_cmng_list_numbered[MP_ARRAY_SIZE(at_cmng_list) + 11];

            memset(m_result_buffer, 0, MP_ARRAY_SIZE(m_result_buffer));
            snprintf(at_cmng_list_numbered,
                    MP_ARRAY_SIZE(at_cmng_list_numbered),
                    "%s%d",
                    at_cmng_list, self->dir);
            
            int len = send_at_command_with_result(handle, (uint8_t *)at_cmng_list_numbered, MP_ARRAY_SIZE(at_cmng_list_numbered), m_result_buffer, MP_ARRAY_SIZE(m_result_buffer));
            (void)nrf_close(handle);
            uint8_t result_count = num_of_entries(m_result_buffer, len);
//            printf("result_count: %u\n", result_count);
            for (;;) {
                (void)len;
                if (self->index == result_count)
                {
                    break;
                }
                uint16_t result_token_length = 0;
                char * result_token = sec_tag_get(self->index, m_result_buffer, strlen(m_result_buffer), &result_token_length);

                //printf("key %u: %s\n", self->index, p_keys[self->index]);
                mp_obj_t name = mp_obj_new_str(result_token, result_token_length);

                // make 3-tuple with info about this entry
                mp_obj_tuple_t *t = MP_OBJ_TO_PTR(mp_obj_new_tuple(3, NULL));
                t->items[0] = name;
                t->items[1] = MP_OBJ_NEW_SMALL_INT(MP_S_IFREG); // all entries are directories
                t->items[2] = MP_OBJ_NEW_SMALL_INT(0); // no inode number
                self->index++;
                return MP_OBJ_FROM_PTR(t);
            }
            break;
        } else {
            break;
        }
    }
    return MP_OBJ_STOP_ITERATION;
}

STATIC mp_obj_t uos_secfs_ilistdir(mp_obj_t self_in, mp_obj_t path_in) {
    mp_obj_uos_secfs_t *self = MP_OBJ_TO_PTR(self_in);
    uos_secfs_ilistdir_it_t *iter = m_new_obj(uos_secfs_ilistdir_it_t);
    iter->base.type = &mp_type_polymorph_iter;
    iter->iternext = uos_secfs_ilistdir_it_iternext;
    iter->is_str = mp_obj_get_type(path_in) == &mp_type_str;
    const char *path = uos_secfs_get_path_str(self, path_in);
//    printf("self->dir = %u\n", self->dir);
    // Trim any slash in front of the path to make it uniform.
    if (path[0] == '/') {
        path++;
    }
//    printf("PATH: %s, len: %u", path, strlen(path));
    iter->index = 0;
    iter->level = 0;
    if (strlen(path) > 0) {
//        printf("type needs to be checked,.\n");
        iter->level = 1;
    }

    iter->dir = 0;
    
    if (self->dir > 0)
    {
//        printf("setting dir to : %u\n", self->dir);
        // current working directory can also be on folder level.
        iter->level = 1;
        iter->dir = self->dir - 1;
    }
    
/*    if (iter->dir == NULL) {
        mp_raise_OSError(1337);
    }
*/
    return MP_OBJ_FROM_PTR(iter);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(uos_secfs_ilistdir_obj, uos_secfs_ilistdir);
/*
STATIC mp_obj_t securefs_file_writable(mp_obj_t self) {
    return mp_obj_new_bool(((file_descriptor_obj *)self)->writable);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(securefs_file_writable_obj, securefs_file_writable);
*/



STATIC const mp_map_elem_t uos_secfs_file_locals_dict_table[] = {
    { MP_OBJ_NEW_QSTR(MP_QSTR_close), (mp_obj_t)&uos_secfs_file_close_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_name), (mp_obj_t)&uos_secfs_file_name_obj },
    /* Stream methods */
    { MP_OBJ_NEW_QSTR(MP_QSTR_read), (mp_obj_t)&mp_stream_read_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readinto), (mp_obj_t)&mp_stream_readinto_obj },
    { MP_OBJ_NEW_QSTR(MP_QSTR_readline), (mp_obj_t)&mp_stream_unbuffered_readline_obj},
    { MP_OBJ_NEW_QSTR(MP_QSTR_write), (mp_obj_t)&mp_stream_write_obj},
};
STATIC MP_DEFINE_CONST_DICT(uos_secfs_file_locals_dict, uos_secfs_file_locals_dict_table);

STATIC const mp_stream_p_t textio_stream_p = {
    .read = securefs_file_read,
    .write = securefs_file_write,
    .is_text = true,
};

const mp_obj_type_t uos_secfs_textio_type = {
    { &mp_type_type },
    .name = MP_QSTR_TextIO,
    .protocol = &textio_stream_p,
    .locals_dict = (mp_obj_dict_t*)&uos_secfs_file_locals_dict,
};

STATIC mp_obj_t uos_secfs_open(mp_obj_t self_in, mp_obj_t path_in, mp_obj_t mode_in) {
    mp_obj_uos_secfs_t *self = MP_OBJ_TO_PTR(self_in);
    const char *mode = mp_obj_str_get_str(mode_in);
    
    const char *path = uos_secfs_get_path_str(self, path_in);
    printf("PATH: %s, len: %u\n", path, strlen(path));
    printf("dir: %u\n", self->dir);
    
    nrf_key_mgnt_cred_type_t cred_type = (nrf_key_mgnt_cred_type_t)(self->dir - 1);

    if (MP_OBJ_IS_STR(path_in)) {
        // a string, parse it
        size_t l;
        const char *s = mp_obj_str_get_data(path_in, &l);
        mp_obj_t number = mp_parse_num_integer(s, l, 0, NULL);
        int32_t tag_id = mp_obj_get_int(number);
        if (strchr(mode, 'w') != NULL) {
            file_descriptor_obj *o = m_new_obj(file_descriptor_obj);
            o->base.type = &uos_secfs_textio_type;
            
            o->tag = tag_id;
            o->type = cred_type;
            o->writable = true;
            o->open = true;
            o->seek = 0;
            o->size = 0;
            return MP_OBJ_FROM_PTR(o);
        } else {
            uint16_t len = 4096;
            uint8_t * p_buffer = m_new(uint8_t, len);
            memset(p_buffer, 0, len);
            int res = nrf_inbuilt_key_read(tag_id,
                                        cred_type,
                                        p_buffer,
                                        &len);
            

            file_descriptor_obj *o = m_new_obj(file_descriptor_obj);
            o->base.type = &uos_secfs_textio_type;

            if (res == 0) {          
                vstr_init(&o->content, len);
                vstr_add_str(&o->content, (char *)p_buffer);
            }
            printf("buffer: %s\n", p_buffer);
            m_free(p_buffer);
            
            printf("size: %u\n", len);
            o->tag = tag_id;
            o->type = cred_type;
            o->writable = false;
            o->open = true;
            o->seek = 0;
            o->size = len;
            return MP_OBJ_FROM_PTR(o);
        }
    }
    
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(uos_secfs_open_obj, uos_secfs_open);

#if MICROPY_VFS
STATIC mp_obj_t uos_secfs_stat(mp_obj_t self, mp_obj_t filename) {
#else
STATIC mp_obj_t uos_secfs_stat(mp_obj_t filename) {
#endif
   return mp_const_none;
}
#if MICROPY_VFS
MP_DEFINE_CONST_FUN_OBJ_2(uos_secfs_stat_obj, uos_secfs_stat);
#else
MP_DEFINE_CONST_FUN_OBJ_1(uos_secfs_stat_obj, uos_secfs_stat);
#endif

#if MICROPY_VFS
STATIC mp_obj_t uos_secfs_chdir(mp_obj_t self_in, mp_obj_t path_in)
{
    mp_obj_uos_secfs_t *self = MP_OBJ_TO_PTR(self_in);
//    DEBUG("chdir\r\n");
    const char *path = uos_secfs_get_path_str(self, path_in);

    // Trim any slash in front of the path to make it uniform.
    if (path[0] == '/') {
        path++;
    }

    const char *start_path = strstr(path, "secure");
    if (start_path == path) {
        path = path + strlen("secure");
        if (path[0] == '/') {
            path++;
        }
    }
//    printf("PATH: %s, len: %u\n", path, strlen(path));

    if (strlen(path) == 0) {
        self->dir = 0;
    } else {
        for (int i = 0; i < 6; i++) {
            const char *folder = strstr(path, secfs_dirs_arr[i]);
            if (folder == path) {
                self->dir = i + 1;
                break;
            }
        }
    }

    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_2(uos_secfs_chdir_obj, uos_secfs_chdir);
#endif

#if MICROPY_VFS
STATIC mp_obj_t uos_secfs_getcwd(mp_obj_t self_in)
{
    mp_obj_uos_secfs_t *self = MP_OBJ_TO_PTR(self_in);
    DEBUG("getcwd\r\n");
    
    if (self->dir == 0) {
        return mp_obj_new_str("/", 1);
    } else {
        vstr_t vstr;
        vstr_init(&vstr, strlen(secfs_dirs_arr[self->dir - 1]) + 1);
        vstr_add_strn(&vstr, "/", 1);
        vstr_add_str(&vstr, secfs_dirs_arr[self->dir - 1]);
    
        return mp_obj_new_str_from_vstr(&mp_type_str, &vstr);
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_1(uos_secfs_getcwd_obj, uos_secfs_getcwd);
#endif

#if MICROPY_VFS

STATIC mp_obj_t uos_secfs_make_new(const mp_obj_type_t *type, size_t n_args, size_t n_kw, const mp_obj_t *args) {
    mp_arg_check_num(n_args, n_kw, 0, 1, false);

    mp_obj_uos_secfs_t *vfs = m_new_obj(mp_obj_uos_secfs_t);
    vfs->base.type = type;
    vstr_init(&vfs->root, 0);
    if (n_args == 1) {
        vstr_add_str(&vfs->root, mp_obj_str_get_str(args[0]));
        vstr_add_char(&vfs->root, '/');
    }
    vfs->root_len = vfs->root.len;
    vfs->readonly = false;

    return MP_OBJ_FROM_PTR(vfs);
}

STATIC mp_obj_t uos_secfs_mount(mp_obj_t self_in, mp_obj_t readonly, mp_obj_t mkfs) {
    mp_obj_uos_secfs_t *self = MP_OBJ_TO_PTR(self_in);
    if (mp_obj_is_true(readonly)) {
        self->readonly = true;
    }
    if (mp_obj_is_true(mkfs)) {
        mp_raise_OSError(MP_EPERM);
    }
    return mp_const_none;
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(uos_secfs_mount_obj, uos_secfs_mount);


STATIC const mp_rom_map_elem_t uos_secfs_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_mount), MP_ROM_PTR(&uos_secfs_mount_obj) },
    { MP_ROM_QSTR(MP_QSTR_ilistdir), MP_ROM_PTR(&uos_secfs_ilistdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_open), MP_ROM_PTR(&uos_secfs_open_obj) },
    { MP_ROM_QSTR(MP_QSTR_remove), MP_ROM_PTR(&uos_secfs_remove_obj) },
    { MP_ROM_QSTR(MP_QSTR_stat), MP_ROM_PTR(&uos_secfs_stat_obj) },
    { MP_ROM_QSTR(MP_QSTR_chdir), MP_ROM_PTR(&uos_secfs_chdir_obj) },
    { MP_ROM_QSTR(MP_QSTR_getcwd), MP_ROM_PTR(&uos_secfs_getcwd_obj) },
};
STATIC MP_DEFINE_CONST_DICT(uos_secfs_locals_dict, uos_secfs_locals_dict_table);

STATIC mp_import_stat_t mp_uos_secfs_import_stat(void *self_in, const char *path) {
    mp_obj_uos_secfs_t *self = self_in;
    (void)self;
    return MP_IMPORT_STAT_NO_EXIST;
}

STATIC const mp_vfs_proto_t uos_secfs_proto = {
    .import_stat = mp_uos_secfs_import_stat,
};

const mp_obj_type_t uos_secfs_type = {
    { &mp_type_type },
    .name = MP_QSTR_SecurityFS,
    .make_new = uos_secfs_make_new,
    .protocol = &uos_secfs_proto,
    .locals_dict = (mp_obj_dict_t*)&uos_secfs_locals_dict,
};

const uos_secfs_obj_t uos_secfs_obj = {
    { &uos_secfs_type },
};

#endif // MICROPY_VFS

#endif // MICROPY_SECURITY_FS
