/** @file
  SyncWindowsFastStartupDxe.c

  Example DXE driver that appends BIOS Setup state into the ACPI Hardware Signature
  using the ACPI Hardware Signature Override Protocol.

  This helps detect unsynchronized configuration changes when Windows Fast Startup
  is enabled and the OS state is expected to match BIOS setup.

  Copyright (c) 2026, King-Phok Ngôo (Jingbo Wu).<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include <Uefi.h>

#include <Library/UefiLib.h>
#include <Library/UefiBootServicesTableLib.h>
#include <Library/UefiRuntimeServicesTableLib.h>
#include <Library/MemoryAllocationLib.h>
#include <Library/DebugLib.h>

#include <Protocol/AcpiHwSigOverride.h>

#define SYNC_WINDOWS_FAST_STARTUP_VARIABLE_NAME  L"SyncWindowsFastStartup"

STATIC EFI_GUID mSyncWindowsFastStartupVarGuid = {
  0x9f62ab24, 0x1f9a, 0x4c32, {0x92, 0x71, 0xd2, 0x8f, 0x46, 0x12, 0xb8, 0xe3}
};

/**
  Entry point for the DXE driver.

  @param[in]  ImageHandle  The firmware allocated handle for the EFI image.
  @param[in]  SystemTable  A pointer to the EFI System Table.

  @retval EFI_SUCCESS           The driver initialized successfully.
  @retval EFI_NOT_FOUND         The ACPI Hardware Signature Override protocol was not found.
  @retval EFI_OUT_OF_RESOURCES  Memory allocation failed.
**/
EFI_STATUS
EFIAPI
SyncWindowsFastStartupDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                       Status;
  ACPI_HW_SIG_OVERRIDE_PROTOCOL    *HwSigOverrideProtocol;
  VOID                             *SetupVarData;
  UINTN                            SetupVarSize;

  SetupVarData = NULL;
  SetupVarSize = 0;
  HwSigOverrideProtocol = NULL;

  DEBUG ((DEBUG_INFO, "[LBR] SyncWindowsFastStartupDxe Entry\n"));

  Status = gBS->LocateProtocol (
                  &gAcpiHwSigOverrideProtocolGuid,
                  NULL,
                  (VOID **) &HwSigOverrideProtocol
                  );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[LBR] ACPI HW Sig Override Protocol not found: %r\n", Status));
    goto Done;
  }

  Status = GetVariable2 (
             SYNC_WINDOWS_FAST_STARTUP_VARIABLE_NAME,
             &mSyncWindowsFastStartupVarGuid,
             &SetupVarData,
             &SetupVarSize
             );
  if (EFI_ERROR (Status) || (SetupVarSize == 0) || (SetupVarData == NULL)) {
    DEBUG ((DEBUG_INFO, "[LBR] Sync variable '%a' not present or empty: %r\n", \
      "SyncWindowsFastStartup", Status));
    goto Done;
  }

  DEBUG ((DEBUG_INFO,
    "[LBR] Appending BIOS Setup sync variable '%a' to Hardware Signature (size %u)\n",
    "SyncWindowsFastStartup",
    SetupVarSize
    ));

  Status = HwSigOverrideProtocol->AppendData (SetupVarData, SetupVarSize);
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_ERROR, "[LBR] Failed to append setup variable to HW signature: %r\n", Status));
  }

Done:
  if (SetupVarData != NULL) {
    FreePool (SetupVarData);
  }

  return EFI_SUCCESS;
}
