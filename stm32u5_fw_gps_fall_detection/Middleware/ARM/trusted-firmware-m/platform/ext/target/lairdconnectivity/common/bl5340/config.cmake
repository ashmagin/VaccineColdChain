#-------------------------------------------------------------------------------
# Copyright (c) 2020, Nordic Semiconductor ASA.
# Copyright (c) 2020-2021, Arm Limited. All rights reserved.
# Copyright (c) 2021, Laird Connectivity.
#
# SPDX-License-Identifier: BSD-3-Clause
#
#-------------------------------------------------------------------------------

set(SECURE_UART1                        ON                   CACHE BOOL      "Enable secure UART1")
set(SECURE_QSPI                         ON                   CACHE BOOL      "Enable secure QSPI")
set(ITS_NUM_ASSETS                      "5"                  CACHE STRING    "The maximum number of assets to be stored in the Internal Trusted Storage area")
set(MCUBOOT_UPGRADE_STRATEGY            "SWAP"               CACHE STRING    "Enable using scratch flash section for swapping images")
