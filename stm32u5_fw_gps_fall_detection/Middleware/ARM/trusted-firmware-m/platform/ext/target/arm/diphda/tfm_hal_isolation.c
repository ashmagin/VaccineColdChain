/*
 * Copyright (c) 2020-2021, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "tfm_api.h"
#include "tfm_hal_defs.h"
#include "tfm_multi_core.h"

enum tfm_hal_status_t tfm_hal_set_up_static_boundaries(void)
{
    /* Setup of isolation HW not implemented yet. */
    return TFM_HAL_SUCCESS;
}

enum tfm_hal_status_t tfm_hal_memory_has_access(uintptr_t base,
                                                size_t size,
                                                uint32_t attr)
{
    enum tfm_status_e status;

    status = tfm_has_access_to_region((const void *)base, size, attr);
    if (status != TFM_SUCCESS) {
         return TFM_HAL_ERROR_MEM_FAULT;
    }

    return TFM_HAL_SUCCESS;
}
