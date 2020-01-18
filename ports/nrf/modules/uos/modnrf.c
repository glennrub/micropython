/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ayke van Laethem
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
#include <stdint.h>
#include "py/runtime.h"
#include "py/mperrno.h"
#include "mpconfigboard.h"

#if MICROPY_HW_ENABLE_STORAGE

#include "modnrf.h"
#include "flash.h"
#include "extmod/vfs.h"

extern byte _fs_start[];
extern byte _fs_end[];

#define FLASH_BLOCK_START (((uint32_t)_fs_start + (FLASH_PAGESIZE-1)) / FLASH_PAGESIZE)

#if MICROPY_VFS_FAT
static int nrf_flashbdev_readblocks_native(uint8_t *buf, size_t block_num, size_t num_blocks) {
    mp_int_t address = (FLASH_BLOCK_START + block_num) * FLASH_PAGESIZE;
    byte *p = (byte*)address;
    for (int i = 0; i < num_blocks * FLASH_PAGESIZE; i++) {
        buf[i] = p[i];
    }
    return 0;
}

static int nrf_flashbdev_writeblocks_native(const uint8_t *buf, size_t block_num, size_t num_blocks) {
    for (int i = 0; i < num_blocks; i++)
    {
        mp_int_t address = (FLASH_BLOCK_START + i) * FLASH_PAGESIZE;

        flash_page_erase(address);

        flash_write_bytes(address, &buf[i * FLASH_PAGESIZE], FLASH_PAGESIZE);
    }
    return 0;
}
#endif

mp_obj_t nrf_flashbdev_readblocks(size_t n_args, const mp_obj_t * args) {
    uint32_t block_num = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_WRITE);

    mp_int_t address = (FLASH_BLOCK_START + block_num) * FLASH_PAGESIZE;

    #if defined(MICROPY_HW_BDEV_READBLOCKS_EXT)
    if (n_args == 4) {
        uint32_t offset = mp_obj_get_int(args[3]);
        address += offset;
    }
    #endif

    byte *buf = bufinfo.buf;
    byte *p = (byte *)address;
    for (int i = 0; i < bufinfo.len; i++) {
        buf[i] = p[i];
    }

    return MP_OBJ_NEW_SMALL_INT(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(nrf_flashbdev_readblocks_obj, 3, 4, nrf_flashbdev_readblocks);

mp_obj_t nrf_flashbdev_writeblocks(size_t n_args, const mp_obj_t * args) {
    uint32_t block_num = mp_obj_get_int(args[1]);
    mp_buffer_info_t bufinfo;
    mp_get_buffer_raise(args[2], &bufinfo, MP_BUFFER_WRITE);

    mp_int_t address = (FLASH_BLOCK_START + block_num) * FLASH_PAGESIZE;

    #if defined(MICROPY_HW_BDEV_READBLOCKS_EXT)
    if (n_args == 4) {
        uint32_t offset = mp_obj_get_int(args[3]);
        address += offset;
    }
    #endif

    flash_write_bytes(address, bufinfo.buf, bufinfo.len);

    return MP_OBJ_NEW_SMALL_INT(0);
}
STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(nrf_flashbdev_writeblocks_obj, 3, 4, nrf_flashbdev_writeblocks);

mp_obj_t nrf_flashbdev_ioctl(mp_obj_t self_in, mp_obj_t op_in, mp_obj_t arg_in) {
    mp_int_t op = mp_obj_get_int(op_in);
    switch (op) {
        case MP_BLOCKDEV_IOCTL_INIT: {
            return MP_OBJ_NEW_SMALL_INT(0);
        }

        case MP_BLOCKDEV_IOCTL_DEINIT: {
            return MP_OBJ_NEW_SMALL_INT(0);
        }

        case MP_BLOCKDEV_IOCTL_SYNC: {
            return MP_OBJ_NEW_SMALL_INT(0);
        }
        case MP_BLOCKDEV_IOCTL_BLOCK_COUNT: {
            mp_int_t block_start = FLASH_BLOCK_START;
            mp_int_t block_end = (uint32_t)_fs_end / FLASH_PAGESIZE;
            mp_int_t num_blocks = block_end - block_start;
            return MP_OBJ_NEW_SMALL_INT(num_blocks);
        }

        case MP_BLOCKDEV_IOCTL_BLOCK_SIZE: {
            return MP_OBJ_NEW_SMALL_INT(FLASH_PAGESIZE);
        }

        case MP_BLOCKDEV_IOCTL_BLOCK_ERASE: {
            mp_int_t block_num = mp_obj_get_int(arg_in);
            mp_int_t address = (FLASH_BLOCK_START + block_num) * FLASH_PAGESIZE;

            if ((address & 0x3) || (address % FLASH_PAGESIZE != 0)) {
                return MP_OBJ_NEW_SMALL_INT(-MP_EIO);
            }

            flash_page_erase(address);
            return MP_OBJ_NEW_SMALL_INT(0);
        }

        default: return mp_const_none;
    }
}
STATIC MP_DEFINE_CONST_FUN_OBJ_3(nrf_flashbdev_ioctl_obj, nrf_flashbdev_ioctl);


STATIC const mp_rom_map_elem_t nrf_flashbdev_locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR_readblocks), MP_ROM_PTR(&nrf_flashbdev_readblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_writeblocks), MP_ROM_PTR(&nrf_flashbdev_writeblocks_obj) },
    { MP_ROM_QSTR(MP_QSTR_ioctl), MP_ROM_PTR(&nrf_flashbdev_ioctl_obj) },
};

STATIC MP_DEFINE_CONST_DICT(nrf_flashbdev_locals_dict, nrf_flashbdev_locals_dict_table);

static const mp_obj_type_t nrf_flashbdev_type = {
    { &mp_type_type },
    .name = MP_QSTR_FlashBdev,
    .locals_dict = (mp_obj_dict_t*)&nrf_flashbdev_locals_dict,
};

const mp_obj_type_t nrf_flashbdev_obj = {
    { &nrf_flashbdev_type },
};


STATIC const mp_rom_map_elem_t nrf_module_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),         MP_ROM_QSTR(MP_QSTR_nrf) },
    { MP_ROM_QSTR(MP_QSTR_flashbdev),        MP_ROM_PTR(&nrf_flashbdev_obj) },
};


STATIC MP_DEFINE_CONST_DICT(nrf_module_globals, nrf_module_globals_table);

const mp_obj_module_t nrf_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&nrf_module_globals,
};

#if MICROPY_VFS_FAT
void flash_init_vfs_fat(fs_user_mount_t *vfs) {
    vfs->base.type = &mp_fat_vfs_type;

    vfs->blockdev.flags |= MP_BLOCKDEV_FLAG_HAVE_IOCTL;
    vfs->blockdev.flags |= MP_BLOCKDEV_FLAG_NO_FILESYSTEM;
//    vfs->blockdev.flags |= MP_BLOCKDEV_FLAG_NATIVE;

    vfs->fatfs.drv = vfs;
#if MICROPY_FATFS_MULTI_PARTITION
    vfs->fatfs.part = 1; // flash filesystem lives on first partition.
#endif

    vfs->blockdev.readblocks[0] = MP_OBJ_FROM_PTR(&nrf_flashbdev_readblocks_obj);
    vfs->blockdev.readblocks[1] = MP_OBJ_FROM_PTR(&nrf_flashbdev_obj);
    vfs->blockdev.readblocks[2] = MP_OBJ_FROM_PTR(nrf_flashbdev_readblocks_native); // native version
    vfs->blockdev.writeblocks[0] = MP_OBJ_FROM_PTR(&nrf_flashbdev_writeblocks_obj);
    vfs->blockdev.writeblocks[1] = MP_OBJ_FROM_PTR(&nrf_flashbdev_obj);
    vfs->blockdev.writeblocks[2] = MP_OBJ_FROM_PTR(nrf_flashbdev_writeblocks_native); // native version
    vfs->blockdev.u.ioctl[0] = MP_OBJ_FROM_PTR(&nrf_flashbdev_ioctl_obj);
    vfs->blockdev.u.ioctl[1] = MP_OBJ_FROM_PTR(&nrf_flashbdev_obj);
}
#endif

#endif // MICROPY_PY_NRF
