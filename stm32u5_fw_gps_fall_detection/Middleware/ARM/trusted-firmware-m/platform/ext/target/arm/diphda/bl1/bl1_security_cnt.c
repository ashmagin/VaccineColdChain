/*
 * Copyright (c) 2019-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "bootutil/security_cnt.h"
#include "tfm_plat_nv_counters.h"
#include "tfm_plat_defs.h"
#include "bootutil/fault_injection_hardening.h"
#include <stdint.h>

/* BL1 only loads 1 image. First 3 counters are used by PS service */
#define TFM_BOOT_NV_COUNTER    (3)

fih_int boot_nv_security_counter_init(void)
{
    fih_int fih_rc = FIH_FAILURE;

    fih_rc = fih_int_encode_zero_equality(tfm_plat_init_nv_counter());

    FIH_RET(fih_rc);
}

fih_int boot_nv_security_counter_get(uint32_t image_id, fih_int *security_cnt)
{
    fih_int fih_rc = FIH_FAILURE;
    uint32_t security_cnt_soft;

    /* Check if it's a null-pointer. */
    if (!security_cnt) {
        FIH_RET(FIH_FAILURE);
    }

    if (image_id != 0) {
        FIH_RET(FIH_FAILURE);
    }

    fih_rc = fih_int_encode_zero_equality(
             tfm_plat_read_nv_counter(TFM_BOOT_NV_COUNTER,
                                      sizeof(security_cnt_soft),
                                      (uint8_t *)&security_cnt_soft));
    *security_cnt = fih_int_encode(security_cnt_soft);

    FIH_RET(fih_rc);
}

int32_t boot_nv_security_counter_update(uint32_t image_id,
                                        uint32_t img_security_cnt)
{
    enum tfm_plat_err_t err;

    if (image_id != 0) {
        return -1;
    }

    err = tfm_plat_set_nv_counter(TFM_BOOT_NV_COUNTER, img_security_cnt);
    if (err != TFM_PLAT_ERR_SUCCESS) {
        return -1;
    }

    return 0;
}
