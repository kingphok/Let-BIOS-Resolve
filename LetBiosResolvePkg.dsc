## @file  LetBiosResolvePkg.dec
# Let BIOS Resolve package provide service and example for resolution which SW/HW have no resource to fix.
#
# Copyright (c) 2026, King-Phok Ngôo (Jingbo Wu).<BR>
# SPDX-License-Identifier: BSD-2-Clause-Patent
#

[Defines]
  PLATFORM_NAME            = LetBiosResolve
  PLATFORM_GUID            = 30EFF5CE-AD84-4318-9A23-B511D6700C06
  PLATFORM_VERSION         = 0.1
  DSC_SPECIFICATION        = 0x00010005
  OUTPUT_DIRECTORY         = Build/LetBiosResolve
  SUPPORTED_ARCHITECTURES  = X64
  BUILD_TARGETS            = DEBUG|RELEASE|NOOPT
  SKUID_IDENTIFIER         = DEFAULT

[Components.X64]

