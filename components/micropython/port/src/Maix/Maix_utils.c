#include <malloc.h>
#include "py/obj.h"
#include "py/runtime.h"
#include "py/mperrno.h"
#include "mpconfigboard.h"
#include "stdint.h"
#include "stdbool.h"
#include "stdlib.h"
#include "sipeed_mem.h"
#include "w25qxx.h"

STATIC mp_obj_t py_gc_heap_size(size_t n_args, const mp_obj_t *args) {
    config_data_t config;
    load_config_from_spiffs(&config);
    if(n_args == 0)
        return mp_obj_new_int(config.gc_heap_size);
    else if(n_args != 1)
        mp_raise_OSError(MP_EINVAL);
    config.gc_heap_size = mp_obj_get_int(args[0]);
    if( !save_config_to_spiffs(&config) )
        mp_raise_OSError(MP_EIO);
    return mp_const_none;
}

STATIC MP_DEFINE_CONST_FUN_OBJ_VAR_BETWEEN(py_gc_heap_size_obj, 0, 1, py_gc_heap_size);

// sys_free(): return the number of bytes of available sys heap RAM
STATIC mp_obj_t py_heap_free(void) {
    return MP_OBJ_NEW_SMALL_INT(get_free_heap_size2());
}
MP_DEFINE_CONST_FUN_OBJ_0(py_heap_free_obj, py_heap_free);

STATIC mp_obj_t py_sys_heap_info(void) {
    extern char _ram_start[];
    extern char _heap_start[];
    extern size_t _heap_cur;

    // an absurdly large amount of ram is used by the OS itself: more than
    // 3.5MB. some of this is micropython's heap and static allocations by
    // the platform, but yikes!
    uint32_t os_used = (uint32_t)((size_t)&_heap_start[0] - (size_t)&_ram_start[0]);

    // get_free_heap_size2() is effectively the difference between _heap_end
    // and _heap_cur (with _heap_end modified to account for space used by
    // _iomem_line).
    uint32_t heap_avail = (uint32_t)get_free_heap_size2();
    // heap_used should be the same as arena_size, but include it as a check.
    uint32_t heap_used = (uint32_t)(_heap_cur - (size_t)&_heap_start[0]);

    // here's a fun one: gcc fills in `mallinfo` as if each field is a u32.
    // but kendryte reads the struct as if each field is u64. we need to
    // cast back.
    struct mallinfo info = mallinfo();
    uint32_t *real_info = (uint32_t *)&info;
    uint32_t arena_size = real_info[0], arena_used = real_info[7], arena_free = real_info[8];

    // os_used + heap_used + heap_avail = total system memory (6MB)
    // arena_used + arena_free = arena_size = heap_used
    mp_obj_t items[6] = {
        mp_obj_new_int_from_uint(os_used),
        mp_obj_new_int_from_uint(heap_used),
        mp_obj_new_int_from_uint(heap_avail),
        mp_obj_new_int_from_uint(arena_size),
        mp_obj_new_int_from_uint(arena_used),
        mp_obj_new_int_from_uint(arena_free),
    };
    return mp_obj_new_tuple(6, items);
}
MP_DEFINE_CONST_FUN_OBJ_0(py_sys_heap_info_obj, py_sys_heap_info);

// STATIC mp_obj_t py_malloc(mp_obj_t arg) {
//     void malloc_stats(void);
//     malloc_stats();
//     void* p = malloc(mp_obj_get_int(arg));
//     return mp_obj_new_int((mp_int_t)p);
// }

// STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_malloc_obj, py_malloc);

// STATIC mp_obj_t py_free(mp_obj_t arg) {
//     free(mp_obj_get_int(arg));
//     return mp_const_none;
// }

// STATIC MP_DEFINE_CONST_FUN_OBJ_1(py_free_obj, py_free);


// STATIC mp_obj_t py_flash_write(mp_obj_t addr, mp_obj_t data_in) {
//     mp_buffer_info_t bufinfo;
//     mp_get_buffer_raise(data_in, &bufinfo, MP_BUFFER_READ);
//     w25qxx_status_t status = w25qxx_write_data_dma(mp_obj_get_int(addr), bufinfo.buf, (uint32_t)bufinfo.len);
//     return mp_obj_new_int(status); // (status != W25QXX_OK)
// }

// STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_flash_write_obj, py_flash_write);

STATIC mp_obj_t py_flash_read(mp_obj_t addr, mp_obj_t len_in) {
    size_t length = mp_obj_get_int(len_in);
    byte* data = m_new(byte, length);
    w25qxx_status_t status = w25qxx_read_data_dma(mp_obj_get_int(addr), data, (uint32_t)length, W25QXX_QUAD_FAST);
    if(status != W25QXX_OK)
    {
        mp_raise_OSError(MP_EIO);
    }
    return mp_obj_new_bytes(data, length);
}

STATIC MP_DEFINE_CONST_FUN_OBJ_2(py_flash_read_obj, py_flash_read);

static const mp_map_elem_t locals_dict_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__),        MP_OBJ_NEW_QSTR(MP_QSTR_utils) },
    { MP_ROM_QSTR(MP_QSTR_gc_heap_size),    (mp_obj_t)(&py_gc_heap_size_obj) },
    { MP_ROM_QSTR(MP_QSTR_heap_free),    (mp_obj_t)(&py_heap_free_obj) },
    // { MP_ROM_QSTR(MP_QSTR_malloc),    (mp_obj_t)(&py_malloc_obj) },
    // { MP_ROM_QSTR(MP_QSTR_free),    (mp_obj_t)(&py_free_obj) },
    { MP_ROM_QSTR(MP_QSTR_sys_heap_info),   (mp_obj_t)(&py_sys_heap_info_obj) },
    { MP_ROM_QSTR(MP_QSTR_flash_read),    (mp_obj_t)(&py_flash_read_obj) },
    // { MP_ROM_QSTR(MP_QSTR_flash_write),    (mp_obj_t)(&py_flash_write_obj) },
};
STATIC MP_DEFINE_CONST_DICT(locals_dict, locals_dict_table);

const mp_obj_type_t Maix_utils_type = {
    .base = { &mp_type_type },
    .name = MP_QSTR_utils,
    .locals_dict = (mp_obj_dict_t*)&locals_dict
};
