/** @file
  AcpiHwSigOverrideDxe.c

  Copyright (c) 2026, King-Phok Ngôo (Jingbo Wu).<BR>
  SPDX-License-Identifier: BSD-2-Clause-Patent
**/

#include "AcpiHwSigOverrideDxe.h"

ACPI_HW_SIG_OVERRIDE_PROTOCOL            *mAcpiHwSigOverrideProtocol = NULL;
EFI_ACPI_SDT_PROTOCOL                    *mAcpiSdtProtocol = NULL;
LIST_ENTRY                               mDataLinkList;

/**
  Wrap original FreePool gBS call in order to decrease code length (with setting back Buffer to NULL).

  @param[in]  Buffer  Pointer to the allocated memory address.
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
  Dump the data in hex format.

  @param[in]  Location  The pointer to the data buffer.
  @param[in]  Length    The size of the data buffer in bytes.
**/
EFI_STATUS
HexDump (
  IN VOID                                *Location,
  IN UINTN                               Length
  )
{
  UINTN                                  Index;

  DEBUG ((DEBUG_INFO,"  Memory Address %016LX 0x%X Bytes\n  ", Location, Length));

  for (Index = 0; Index < Length; Index++) {
    DEBUG ((DEBUG_INFO, "0x%02x ", ((UINT8 *) Location)[Index]));
  }
  DEBUG ((DEBUG_INFO, "\n"));

  return EFI_SUCCESS;
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
  LIST_ENTRY                             *Link;
  APPEND_DATA_INSTANCE                   *AppendDataInstance;

  Status             = EFI_SUCCESS;
  AppendDataInstance = NULL;

  DEBUG ((DEBUG_INFO, "[LBR] ReadyToBootCallback Entry.\n"));

  //
  // The ready to boot callback function maybe called more than once,
  // so close the event after the first call to avoid redundant modify ACPI Hardware Signature.
  //
  if (Event != NULL) {
    gBS->CloseEvent (Event);
  }

  Link = GetFirstNode (&mDataLinkList);
  if (IsNull (&mDataLinkList, Link)) {
    DEBUG ((DEBUG_INFO, "[LBR] No AppendData be recorded.\n"));
    goto SeviceComplete;
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

SeviceComplete:
  //
  // Free AppendData instances.
  //
  Link = &mDataLinkList;
  while (!IsListEmpty (Link)) {
    AppendDataInstance = BASE_CR (Link->ForwardLink, APPEND_DATA_INSTANCE, LinkList);
    DEBUG ((DEBUG_INFO, "[LBR] Free AppendDataInstance\n"));
    DEBUG_CODE (
      HexDump (AppendDataInstance->AppendData, AppendDataInstance->AppendDataSize);
    );
    RemoveEntryList (Link->ForwardLink);
    SafeFreePool ((VOID **) &AppendDataInstance->AppendData);
    SafeFreePool ((VOID **) &AppendDataInstance);
  }

  //
  // Uninstall ACPI Hardware Signature Override protocol.
  //
  Status = gBS->UninstallProtocolInterface (
                  Context, // ImageHandle
                  &gAcpiHwSigOverrideProtocolGuid,
                  (VOID *) mAcpiHwSigOverrideProtocol
                  );
  DEBUG ((DEBUG_INFO, "[LBR] Uninstall mAcpiHwSigOverrideProtocol %r\n", Status));

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
  EFI_EVENT                              Event;

  Status       = EFI_SUCCESS;
  Event        = NULL;

  DEBUG ((DEBUG_INFO, "[LBR] AcpiHwSigOverrideDxe Entry\n"));

  //
  // Install ACPI Hardware Signature Override protocol
  //
  mAcpiHwSigOverrideProtocol = AllocateZeroPool (sizeof (ACPI_HW_SIG_OVERRIDE_PROTOCOL));
  if (mAcpiHwSigOverrideProtocol != NULL) {
    InitializeListHead (&mDataLinkList);
    mAcpiHwSigOverrideProtocol->AppendData = AcpiHwSigOverrideAppendData;
    Status = gBS->InstallProtocolInterface (
                    &ImageHandle,
                    &gAcpiHwSigOverrideProtocolGuid,
                    EFI_NATIVE_INTERFACE,
                    mAcpiHwSigOverrideProtocol
                    );
    DEBUG ((DEBUG_INFO, "[LBR] Install mAcpiHwSigOverrideProtocol %r\n", Status));
  }

  //
  // Register Ready To Boot callback function
  //
  Status = EfiCreateEventReadyToBootEx (
             TPL_CALLBACK,
             ReadyToBootCallback,
             ImageHandle,
             &Event
             );
  if (EFI_ERROR (Status)) {
    DEBUG ((DEBUG_INFO, "[LBR] EfiCreateEventReadyToBootEx %r\n", Status));
  }

  return EFI_SUCCESS;
}