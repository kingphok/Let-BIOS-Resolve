/** @file
  ACPI Hardware Signature Override Protocol definition.

  Copyright (c) 2026, King-Phok Ngôo (Jingbo Wu).<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#ifndef _ACPI_HW_SIG_OVERRIDE_H_
#define _ACPI_HW_SIG_OVERRIDE_H_

//
// Forward declaration
//
typedef struct _ACPI_HW_SIG_OVERRIDE_PROTOCOL ACPI_HW_SIG_OVERRIDE_PROTOCOL;

//
// Function prototypes
//
/**
  Append data which will be calculated by CRC32 with original Hardware Signature,
  then update to ACPI FACS table in ready to boot callback function.

  @param[in]  Data         The pointer to the data buffer.
  @param[in]  DataSize     The size of the data buffer in bytes.
**/
typedef
EFI_STATUS
(EFIAPI *APPEND_DATA) (
  IN UINT8                               *Data,
  IN UINTN                               DataSize
  );

//
// Protocol structure
//
typedef struct _ACPI_HW_SIG_OVERRIDE_PROTOCOL{
  APPEND_DATA                            AppendData;
} ACPI_HW_SIG_OVERRIDE_PROTOCOL;

extern EFI_GUID gAcpiHwSigOverrideProtocolGuid;

#endif // _ACPI_HW_SIG_OVERRIDE_H_