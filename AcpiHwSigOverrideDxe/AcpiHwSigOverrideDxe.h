/** @file
  AcpiHwSigOverrideDxe.h

  Copyright (c) 2026, King-Phok Ngôo (Jingbo Wu).<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _ACPI_HW_SIG_OVERRIDE_DXE_H_
#define _ACPI_HW_SIG_OVERRIDE_DXE_H_

#include <Uefi.h>
#include <IndustryStandard/Acpi.h>

//
// Libraries
//
#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/BaseMemoryLib.h>
#include <Library/DebugLib.h>

//
// Consumed Protocols
//
#include <Protocol/AcpiSystemDescriptionTable.h>
#include <Protocol/AcpiTable.h>

//
// Produced Protocols
//
#include <Protocol/AcpiHwSigOverride.h>

//
// Guids
//

//
// Include files with function prototypes
//

//
// Internal Structure definition
//
typedef struct {
  LIST_ENTRY                             LinkList;
  UINT8                                  *AppendData;
  UINTN                                  AppendDataSize;
} APPEND_DATA_INSTANCE;

//
// Internal function prototypes definition
//

//
// Extension declaration
//

#endif // _ACPI_HW_SIG_OVERRIDE_DXE_H_