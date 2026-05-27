/** @file
  AcpiHwSigOverrideDxe.c

  Copyright (c) 2026, King-Phok Ngôo (Jingbo Wu).<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiHwSigOverrideDxe.h"

EFI_ACPI_SDT_PROTOCOL                    *mAcpiSdtProtocol = NULL;
LIST_ENTRY                               mDataLinkList;

/**
 Wrap original FreePool gBS call in order to decrease code length (with setting back Buffer to NULL).

 @param[in] Buffer  Pointer to the allocated memory address.
**/
VOID
SafeFreePool (
  IN VOID                               **Buffer
  )
{
  if (Buffer != NULL && *Buffer != NULL) {
    gBS->FreePool (*Buffer);
    *Buffer = NULL;
  }
}

/**
  Ready To Boot callback function.

  @param[in]  Event     Event whose notification function is being invoked
  @param[in]  Context   Pointer to the notification function's context
**/
VOID
EFIAPI
ReadyToBootCallback (
  IN EFI_EVENT  Event,
  IN VOID       *Context
  )
{
  EFI_STATUS                             Status;

  Status = EFI_SUCCESS;

  DEBUG ((DEBUG_INFO, "[LBR] ReadyToBootCallback Entry.\n"));

  //
  // The ready to boot callback function maybe called more than once,
  // so close the event after the first call to avoid redundant modify ACPI Hardware Signature.
  //
  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

  //
  // Get the ACPI SDT protocol
  //
  if (mAcpiSdtProtocol == NULL) {
    Status = gBS->LocateProtocol (
                  &gEfiAcpiSdtProtocolGuid,
                  NULL,
                  (VOID **) &mAcpiSdtProtocol
                  );
    if (EFI_ERROR (Status)) {
      DEBUG ((DEBUG_INFO, "[LBR] Locate gEfiAcpiSdtProtocolGuid %r\n", Status));
    }
  }

  //
  // Caculate the hardware signature and update it to ACPI FACS table if need.
  //

  //
  // Uninstall ACPI Hardware Signature Override protocol.
  //

  return;
}

/**
  Append data which will be calculated by CRC32 with original Hardware Signature,
  then update to ACPI FACS table in ready to boot callback function.

  @param[in]  Data         The pointer to the data buffer.
  @param[in]  DataSize     The size of the data buffer in bytes.
**/
EFI_STATUS
AcpiHwSigOverrideAppendData (
  IN UINT8                               *Data,
  IN UINTN                               DataSize
  )
{
  APPEND_DATA_INSTANCE                   *AppendDataInstance;

  if ((Data == NULL) || (DataSize == 0)) {
    DEBUG ((DEBUG_INFO, "[LBR] AppendData are invalid parameters.\n"));
    return EFI_INVALID_PARAMETER;
  }

  AppendDataInstance = AllocateZeroPool (sizeof (APPEND_DATA_INSTANCE));
  if (AppendDataInstance != NULL) {
    AppendDataInstance->AppendData = AllocateZeroPool (DataSize);
    if (AppendDataInstance->AppendData == NULL) {
      DEBUG ((DEBUG_INFO, "[LBR] AllocateZeroPool for AppendData out of resources.\n"));
      SafeFreePool ((VOID**) &AppendDataInstance);
      return EFI_OUT_OF_RESOURCES;
    }
  } else {
    DEBUG ((DEBUG_INFO, "[LBR] AllocateZeroPool for AppendDataInstance out of resources.\n"));
    return EFI_OUT_OF_RESOURCES;
  }

  CopyMem ((VOID *) AppendDataInstance->AppendData, (VOID *) Data, DataSize);
  AppendDataInstance->AppendDataSize = DataSize;
  InsertTailList (&mDataLinkList, &AppendDataInstance->LinkList);

  DEBUG ((DEBUG_INFO, "[LBR] AppendData be recorded.\n"));

  return EFI_SUCCESS;
}

/**
  The entry point for AcpiHwSigOverrideDxe.

  @param[in]  ImageHandle        The image handle of the driver.
  @param[in]  SystemTable        The system table.

  @retval EFI_ALREADY_STARTED    The driver already exists in system.
  @retval EFI_OUT_OF_RESOURCES   Fail to execute entry point due to lack of resources.
  @retval EFI_SUCCESS            All the related protocols are installed on the driver.
  @retval Others                 Fail to install protocols as indicated.
**/
EFI_STATUS
EFIAPI
AcpiHwSigOverrideDxeEntryPoint (
  IN EFI_HANDLE        ImageHandle,
  IN EFI_SYSTEM_TABLE  *SystemTable
  )
{
  EFI_STATUS                             Status;
  ACPI_HW_SIG_OVERRIDE_PROTOCOL          *AcpiHwSigOverrideProtocol;
  EFI_EVENT                              Event;

  Status       = EFI_SUCCESS;
  Event        = NULL;

  DEBUG ((DEBUG_INFO, "[LBR] AcpiHwSigOverrideDxe Entry\n"));

  //
  // Install ACPI Hardware Signature Override protocol
  //
  AcpiHwSigOverrideProtocol = AllocateZeroPool (sizeof (ACPI_HW_SIG_OVERRIDE_PROTOCOL));
  if (AcpiHwSigOverrideProtocol != NULL) {
    InitializeListHead (&mDataLinkList);
    AcpiHwSigOverrideProtocol->AppendData = AcpiHwSigOverrideAppendData;
    Status = gBS->InstallProtocolInterface (
                    &ImageHandle,
                    &gAcpiHwSigOverrideProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    AcpiHwSigOverrideProtocol
                    );
    DEBUG ((DEBUG_INFO, "[LBR] Install AcpiHwSigOverrideProtocol %r\n", Status));
  }

  //
  // Register Ready To Boot callback function
  //
  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             ReadyToBootCallback,
             NULL,
             &Event
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "[LBR] EfiCreateEventReadyToBootEx %r\n", Status));
  }

  return EFI_SUCCESS;
}