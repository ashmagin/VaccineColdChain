/*
 * Copyright (c) 2020, Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 *
 */

#include "cmsis.h"
#include "Driver_Common.h"
#include "mpu_armv8m_drv.h"
#include "region.h"
#include "target_cfg.h"
#include "tfm_hal_isolation.h"
#include "tfm_plat_defs.h"
#ifdef FLOW_CONTROL

#include "target_flowcontrol.h"
#else
/* dummy definitions */
extern volatile uint32_t uFlowStage;
#define FLOW_CONTROL_STEP(C,B,A) ((void)0)
#define FLOW_CONTROL_CHECK(B,A) ((void)0)
#define FLOW_STAGE_CFG          (0x0)
#define FLOW_STAGE_CHK          (0x0)
#endif
#include "region_defs.h"
#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
#if TFM_LVL == 3
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Base);
REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Limit);
REGION_DECLARE(Image$$, PT_RO_START, $$Base);
REGION_DECLARE(Image$$, PT_RO_END, $$Base);
REGION_DECLARE(Image$$, PT_PRIV_RWZI_START, $$Base);
REGION_DECLARE(Image$$, PT_PRIV_RWZI_END, $$Base);

static uint32_t g_static_region_cnt;

static struct mpu_armv8m_region_cfg_t isolation_regions[] = {
    {
        0, /* will be updated before using */
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Base),
        (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Limit),
        MPU_ARMV8M_MAIR_ATTR_CODE_IDX,
        MPU_ARMV8M_XN_EXEC_OK,
        MPU_ARMV8M_AP_RO_PRIV_UNPRIV,
        MPU_ARMV8M_SH_NONE,
    },
    {
        0, /* will be updated before using */
        (uint32_t)&REGION_NAME(Image$$, PT_RO_START, $$Base),
        (uint32_t)&REGION_NAME(Image$$, PT_RO_END, $$Base),
        MPU_ARMV8M_MAIR_ATTR_CODE_IDX,
        MPU_ARMV8M_XN_EXEC_OK,
        MPU_ARMV8M_AP_RO_PRIV_UNPRIV,
        MPU_ARMV8M_SH_NONE,
    },
    /* For isolation Level 3, set up static isolation for privileged data.
     * Unprivileged data is dynamically set during Partition sheduling.
     */
    {
        0, /* will be updated before using */
        (uint32_t)&REGION_NAME(Image$$, PT_PRIV_RWZI_START, $$Base),
        (uint32_t)&REGION_NAME(Image$$, PT_PRIV_RWZI_END, $$Base),
        MPU_ARMV8M_MAIR_ATTR_DATA_IDX,
        MPU_ARMV8M_XN_EXEC_NEVER,
        MPU_ARMV8M_AP_RW_PRIV_ONLY,
        MPU_ARMV8M_SH_NONE,
    },
};
#else /* TFM_LVL == 3 */

#define MPU_REGION_TFM_UNPRIV_CODE   0
#define MPU_REGION_TFM_UNPRIV_DATA   1
#define MPU_REGION_TFM_PRIV_CODE     2
#define MPU_REGION_TFM_PRIV_DATA     3

#define PARTITION_REGION_PERIPH      6
#define PARTITION_REGION_NV_DATA     7

#define MPU_NONSECURE_ALIAS_FLASH    4
#define MPU_NONSECURE_ALIAS_RAM      5

REGION_DECLARE(Image$$, TFM_UNPRIV_CODE, $$RO$$Base);
REGION_DECLARE(Image$$, TFM_UNPRIV_CODE, $$RO$$Limit);
REGION_DECLARE(Image$$, TFM_UNPRIV_DATA, $$RW$$Base);
REGION_DECLARE(Image$$, TFM_UNPRIV_DATA, $$ZI$$Limit);

REGION_DECLARE(Image$$, TFM_APP_RW_STACK_START, $$Base);
REGION_DECLARE(Image$$, TFM_APP_RW_STACK_END, $$Base);
REGION_DECLARE(Image$$, ARM_LIB_STACK, $$ZI$$Base);
REGION_DECLARE(Image$$, ARM_LIB_STACK, $$ZI$$Limit);

REGION_DECLARE(Load$$LR$$, LR_VENEER, $$Limit);

const struct mpu_armv8m_region_cfg_t region_cfg_init_s[] = {
           /* TFM Core unprivileged code region */
           {
               MPU_REGION_TFM_UNPRIV_CODE,
               (uint32_t)&REGION_NAME(Image$$, TFM_UNPRIV_CODE, $$RO$$Base),
               (uint32_t)&REGION_NAME(Load$$LR$$, LR_VENEER, $$Limit) - 1,
               MPU_ARMV8M_MAIR_ATTR_CODE_IDX,
               MPU_ARMV8M_XN_EXEC_OK,
               MPU_ARMV8M_AP_RO_PRIV_UNPRIV,
               MPU_ARMV8M_SH_NONE,
#ifdef FLOW_CONTROL
               FLOW_STEP_MPU_S_EN_R0,
               FLOW_CTRL_MPU_S_EN_R0,
               FLOW_STEP_MPU_S_CH_R0,
               FLOW_CTRL_MPU_S_CH_R0,
#endif /* FLOW_CONTROL */
           },
           /* TFM Core unprivileged data region */
           {
               MPU_REGION_TFM_UNPRIV_DATA,
               (uint32_t)&REGION_NAME(Image$$, ARM_LIB_STACK, $$ZI$$Base),
               (uint32_t)&REGION_NAME(Image$$, TFM_APP_RW_STACK_END, $$Base) - 1,
               MPU_ARMV8M_MAIR_ATTR_DATA_IDX,
               MPU_ARMV8M_XN_EXEC_NEVER,
               MPU_ARMV8M_AP_RW_PRIV_UNPRIV,
               MPU_ARMV8M_SH_NONE,
#ifdef FLOW_CONTROL
               FLOW_STEP_MPU_S_EN_R1,
               FLOW_CTRL_MPU_S_EN_R1,
               FLOW_STEP_MPU_S_CH_R1,
               FLOW_CTRL_MPU_S_CH_R1,
#endif /* FLOW_CONTROL */
           },
           /* TFM_Core privileged code region   */
           {
               MPU_REGION_TFM_PRIV_CODE,
               S_CODE_START,
               (uint32_t)&REGION_NAME(Image$$, TFM_UNPRIV_CODE, $$RO$$Base) - 1,
               MPU_ARMV8M_MAIR_ATTR_CODE_IDX,
               MPU_ARMV8M_XN_EXEC_OK,
               MPU_ARMV8M_AP_RO_PRIV_ONLY,
               MPU_ARMV8M_SH_NONE,
#ifdef FLOW_CONTROL
               FLOW_STEP_MPU_S_EN_R2,
               FLOW_CTRL_MPU_S_EN_R2,
               FLOW_STEP_MPU_S_CH_R2,
               FLOW_CTRL_MPU_S_CH_R2,
#endif /* FLOW_CONTROL */
           },
           /* TFM_Core privileged data region   */
           {
               MPU_REGION_TFM_PRIV_DATA,
               (uint32_t)&REGION_NAME(Image$$, TFM_APP_RW_STACK_END, $$Base),
               S_DATA_LIMIT,
               MPU_ARMV8M_MAIR_ATTR_DATA_IDX,
               MPU_ARMV8M_XN_EXEC_NEVER,
               MPU_ARMV8M_AP_RW_PRIV_ONLY,
               MPU_ARMV8M_SH_NONE,
#ifdef FLOW_CONTROL
               FLOW_STEP_MPU_S_EN_R3,
               FLOW_CTRL_MPU_S_EN_R3,
               FLOW_STEP_MPU_S_CH_R3,
               FLOW_CTRL_MPU_S_CH_R3,
#endif /* FLOW_CONTROL */
           },
           /* peripheral for AppROT partition */
           {
               PARTITION_REGION_PERIPH,
               PERIPH_BASE_S,
               PERIPH_BASE_S+0x0fffffff,
               MPU_ARMV8M_MAIR_ATTR_DATANOCACHE_IDX,
               MPU_ARMV8M_XN_EXEC_NEVER,
               MPU_ARMV8M_AP_RW_PRIV_UNPRIV,
               MPU_ARMV8M_SH_NONE,
#ifdef FLOW_CONTROL
               FLOW_STEP_MPU_S_EN_R4,
               FLOW_CTRL_MPU_S_EN_R4,
               FLOW_STEP_MPU_S_CH_R4,
               FLOW_CTRL_MPU_S_CH_R4,
#endif /* FLOW_CONTROL */
           },
           /* TFM Non volatile data region (NVCNT/SST/ITS) */
           {
               PARTITION_REGION_NV_DATA,
               TFM_NV_DATA_START,
               TFM_NV_DATA_LIMIT,
               MPU_ARMV8M_MAIR_ATTR_DATANOCACHE_IDX,
               MPU_ARMV8M_XN_EXEC_NEVER,
               MPU_ARMV8M_AP_RW_PRIV_ONLY,
               MPU_ARMV8M_SH_NONE,
#ifdef FLOW_CONTROL
               FLOW_STEP_MPU_S_EN_R5,
               FLOW_CTRL_MPU_S_EN_R5,
               FLOW_STEP_MPU_S_CH_R5,
               FLOW_CTRL_MPU_S_CH_R5,
#endif /* FLOW_CONTROL */
           },
           /* RAM Non Secure Alias Not Executable */
           {
               MPU_NONSECURE_ALIAS_RAM,
               SRAM1_BASE_NS,
               SRAM4_BASE_NS + SRAM4_SIZE -1,
               MPU_ARMV8M_MAIR_ATTR_DATA_IDX,
               MPU_ARMV8M_XN_EXEC_NEVER,
               MPU_ARMV8M_AP_RW_PRIV_ONLY,
               MPU_ARMV8M_SH_NONE,
#ifdef FLOW_CONTROL
               FLOW_STEP_MPU_S_EN_R6,
               FLOW_CTRL_MPU_S_EN_R6,
               FLOW_STEP_MPU_S_CH_R6,
               FLOW_CTRL_MPU_S_CH_R6,
#endif /* FLOW_CONTROL */
           },
           /* FLASH Non Secure Alias Not Executable, read only */
           {
               MPU_NONSECURE_ALIAS_FLASH,
               FLASH_BASE_NS,
               FLASH_BASE_NS+FLASH_TOTAL_SIZE-1,
               MPU_ARMV8M_MAIR_ATTR_DATANOCACHE_IDX,
               MPU_ARMV8M_XN_EXEC_NEVER,
               MPU_ARMV8M_AP_RO_PRIV_ONLY,
               MPU_ARMV8M_SH_NONE,
#ifdef FLOW_CONTROL
               FLOW_STEP_MPU_S_EN_R7,
               FLOW_CTRL_MPU_S_EN_R7,
               FLOW_STEP_MPU_S_CH_R7,
               FLOW_CTRL_MPU_S_CH_R7,
#endif /* FLOW_CONTROL */
           },
};

static enum tfm_hal_status_t mpu_init(void)
{
  struct mpu_armv8m_dev_t dev_mpu_s = { MPU_BASE };
  uint32_t i;

  /* configuration stage */
  if (uFlowStage == FLOW_STAGE_CFG)
  {
    mpu_armv8m_clean(&dev_mpu_s);

    /* configure secure MPU regions */
    for (i = 0; i < ARRAY_SIZE(region_cfg_init_s); i++)
    {
      if (mpu_armv8m_region_enable(&dev_mpu_s,
        (struct mpu_armv8m_region_cfg_t *)&region_cfg_init_s[i]) != MPU_ARMV8M_OK)
      {
        return TFM_HAL_ERROR_GENERIC;
      }
      else
      {
        /* Execution stopped if flow control failed */
        FLOW_CONTROL_STEP(uFlowProtectValue, region_cfg_init_s[i].flow_step_enable,
                                             region_cfg_init_s[i].flow_ctrl_enable);
      }
    }

    /* enable secure MPU */
    mpu_armv8m_enable(&dev_mpu_s, PRIVILEGED_DEFAULT_ENABLE, HARDFAULT_NMI_ENABLE);
    FLOW_CONTROL_STEP(uFlowProtectValue, FLOW_STEP_MPU_S_EN, FLOW_CTRL_MPU_S_EN);
  }
  /* verification stage */
  else
  {
    /* check secure MPU regions */
    for (i = 0; i < ARRAY_SIZE(region_cfg_init_s); i++)
    {
      if (mpu_armv8m_region_enable_check(&dev_mpu_s,
        (struct mpu_armv8m_region_cfg_t *)&region_cfg_init_s[i]) != MPU_ARMV8M_OK)
      {
        Error_Handler();
      }
      else
      {
        /* Execution stopped if flow control failed */
        FLOW_CONTROL_STEP(uFlowProtectValue, region_cfg_init_s[i].flow_step_check,
                                             region_cfg_init_s[i].flow_ctrl_check);
      }
    }

    /* check secure MPU */
    if (mpu_armv8m_check(&dev_mpu_s, PRIVILEGED_DEFAULT_ENABLE,
                      HARDFAULT_NMI_ENABLE) != MPU_ARMV8M_OK)
    {
      Error_Handler();
    }
    else
    {
      /* Execution stopped if flow control failed */
      FLOW_CONTROL_STEP(uFlowProtectValue, FLOW_STEP_MPU_S_CH, FLOW_CTRL_MPU_S_CH);
    }

    /* Lock MPU config */
    __HAL_RCC_SYSCFG_CLK_ENABLE();
/* in TFM_DEV_MODE MPU is not locked, this allows to see MPU configuration */
#ifndef TFM_DEV_MODE
    SYSCFG->CSLCKR |= SYSCFG_CSLCKR_LOCKSMPU;
#endif
    FLOW_CONTROL_STEP(uFlowProtectValue, FLOW_STEP_MPU_S_LCK, FLOW_CTRL_MPU_S_LCK);
#ifndef TFM_DEV_MODE
    if (!(SYSCFG->CSLCKR & SYSCFG_CSLCKR_LOCKSMPU))
        Error_Handler();
#endif
    FLOW_CONTROL_STEP(uFlowProtectValue, FLOW_STEP_MPU_S_LCK_CH, FLOW_CTRL_MPU_S_LCK_CH);
  }

  return TFM_HAL_SUCCESS;
}
#endif
#endif
#if defined(FLOW_CONTROL)
/* Global variable for Flow Control state */
volatile uint32_t uFlowProtectValue = FLOW_CTRL_INIT_VALUE;
#endif /* FLOW_CONTROL */
volatile uint32_t uFlowStage = FLOW_STAGE_CFG;


enum tfm_hal_status_t tfm_hal_set_up_static_boundaries(void)
{

  /* STM32U5xx HAL library initialization:
       - Systick timer is configured by default as source of time base, but user
             can eventually implement his proper time base source (a general purpose
             timer for example or other time source), keeping in mind that Time base
             duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
             handled in milliseconds basis.
       - Set NVIC Group Priority to 3
       - Low Level Initialization
     */
  HAL_Init();

  /* Configures non-secure memory spaces in the target */
  uFlowStage = FLOW_STAGE_CFG;
  gtzc_init_cfg();
  sau_and_idau_cfg();
  pinmux_init_cfg();

  /* Check configurations with Flow control to resist to basic HW attacks */
  uFlowStage = FLOW_STAGE_CHK;
  gtzc_init_cfg();
  sau_and_idau_cfg();

  /* Start HW randomization */
  if (RNG_Init()){
      Error_Handler();
  };


    /* Set up static isolation boundaries inside SPE */
#ifdef CONFIG_TFM_ENABLE_MEMORY_PROTECT
 

#if TFM_LVL == 3
    uint32_t cnt;
    int32_t i;
    struct mpu_armv8m_dev_t dev_mpu_s = { MPU_BASE };

    mpu_armv8m_clean(&dev_mpu_s);

    /* Update MPU region numbers. The numbers start from 0 and are continuous */
    cnt = sizeof(isolation_regions) / sizeof(isolation_regions[0]);
    g_static_region_cnt = cnt;
    for (i = 0; i < cnt; i++) {
        /* Update region number */
        isolation_regions[i].region_nr = i;
        /* Enable regions */
        if (mpu_armv8m_region_enable(&dev_mpu_s, &isolation_regions[i])
                                                             != MPU_ARMV8M_OK) {
            return TFM_HAL_ERROR_GENERIC;
        }
    }
        /* Enable MPU */
    if (mpu_armv8m_enable(&dev_mpu_s,
                          PRIVILEGED_DEFAULT_ENABLE,
                          HARDFAULT_NMI_ENABLE) != MPU_ARMV8M_OK) {
        return TFM_HAL_ERROR_GENERIC;
    }

#else /* TFM_LVL == 3 */
    uFlowStage = FLOW_STAGE_CFG;
    mpu_init();
    uFlowStage = FLOW_STAGE_CHK;
    mpu_init();

#endif /* TFM_LVL == 3 */

#endif /* CONFIG_TFM_ENABLE_MEMORY_PROTECT */

    return TFM_HAL_SUCCESS;
}

#if TFM_LVL == 3
enum tfm_hal_status_t tfm_hal_mpu_update_partition_boundary(uintptr_t start,
                                                            uintptr_t end)
{
    struct mpu_armv8m_region_cfg_t cfg;
    enum mpu_armv8m_error_t mpu_err;
    struct mpu_armv8m_dev_t dev_mpu_s = { MPU_BASE };

    /* Partition boundary regions is right after static regions */
    cfg.region_nr = g_static_region_cnt;
    cfg.region_base = start;
    cfg.region_limit = end;
    cfg.region_attridx = MPU_ARMV8M_MAIR_ATTR_DATA_IDX;
    cfg.attr_access = MPU_ARMV8M_AP_RW_PRIV_UNPRIV;
    cfg.attr_exec = MPU_ARMV8M_XN_EXEC_NEVER;
    cfg.attr_sh = MPU_ARMV8M_SH_NONE;
    mpu_err = mpu_armv8m_region_enable(&dev_mpu_s, &cfg);
    if (mpu_err != MPU_ARMV8M_OK) {
        return TFM_HAL_ERROR_GENERIC;
    }
    return TFM_HAL_SUCCESS;
}
#endif /* TFM_LVL == 3 */
