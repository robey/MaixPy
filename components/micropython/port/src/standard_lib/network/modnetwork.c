/*
 * This file is part of the MicroPython project, http://micropython.org/
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2014 Damien P. George
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
#include <string.h>

#include "py/objlist.h"
#include "py/runtime.h"
#include "py/mphal.h"
#include "shared/netutils/netutils.h"
#include "modnetwork.h"

#if MICROPY_PY_NETWORK

#if MICROPY_PY_LWIP

#include "lwip/netif.h"
#include "lwip/timeouts.h"
#include "lwip/dns.h"
#include "lwip/dhcp.h"

u32_t sys_now(void) {
    return mp_hal_ticks_ms();
}

#endif

// module network - network configuration
// This module provides network drivers and routing configuration.

void mod_network_init(void) {

      MP_STATE_PORT(modnetwork_nic) = NULL;
}

void mod_network_deinit(void) {
}
void mod_network_register_nic(mp_obj_t nic) {
      MP_STATE_PORT(modnetwork_nic) = MP_OBJ_TO_PTR(nic);
}

mp_obj_t mod_network_find_nic(const uint8_t *ip) {
    //TODO: find a NIC that is suited to given IP address
      if(MP_STATE_PORT(modnetwork_nic) != NULL)
    {
              return MP_OBJ_FROM_PTR(MP_STATE_PORT(modnetwork_nic));
    }
    nlr_raise(mp_obj_new_exception_msg(&mp_type_OSError, "no available NIC"));
}

STATIC mp_obj_t network_route(void) {
    return MP_OBJ_FROM_PTR(MP_STATE_PORT(modnetwork_nic));
}
STATIC MP_DEFINE_CONST_FUN_OBJ_0(network_route_obj, network_route);

STATIC const mp_rom_map_elem_t mp_module_network_globals_table[] = {
    { MP_ROM_QSTR(MP_QSTR___name__), MP_ROM_QSTR(MP_QSTR_network) },
    { MP_ROM_QSTR(MP_QSTR_ESP8285), MP_ROM_PTR(&mod_network_nic_type_esp8285) },

    { MP_ROM_QSTR(MP_QSTR_ESP32_SPI), MP_ROM_PTR(&mod_network_nic_type_esp32) },
#if CONFIG_MAIXPY_WIZNET5K_ENABLE
    { MP_ROM_QSTR(MP_QSTR_WIZNET5K), MP_ROM_PTR(&mod_network_nic_type_wiznet5k) },
#endif
    { MP_ROM_QSTR(MP_QSTR_route), MP_ROM_PTR(&network_route_obj) },
};

STATIC MP_DEFINE_CONST_DICT(mp_module_network_globals, mp_module_network_globals_table);

const mp_obj_module_t network_module = {
    .base = { &mp_type_module },
    .globals = (mp_obj_dict_t*)&mp_module_network_globals,
};

#endif  // MICROPY_PY_NETWORK
