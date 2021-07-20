
EFI_NOT_READY = 0x8000_0000_0000_0000 or 6

;; Based on https://wiki.osdev.org/Uefi.inc
macro struct name {
  virtual at 0
    name name
  end virtual
}

struc EFI_TABLE_HEADER 
{
    dq ?
    dd ?
    dd ?
    dd ?
    dd ?
}

struc EFI_SYSTEM_TABLE 
{
  .Hdr EFI_TABLE_HEADER
  .FirmwareVendor   dq ?        ; CHAR16*
  .FirmwareRevision dd ?        ; UINT32
  align 8
  .ConsoleInHandle  dq ?        ; EFI_HANDLE
  .ConIn            dq ?        ; EFI_SIMPLE_TEXT_INPUT_PROTOCOL*
  .ConsoleOutHandle dq ?        ; EFI_HANDLE
  .ConOut           dq ?        ; EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*
  .StandardErrorHandle  dq ?    ; EFI_HANDLE
  .StdErr           dq ?        ; EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL*
  .RuntimeServices  dq ?        ; EFI_RUNTIME_SERVICES*
  .BootServices     dq ?        ; EFI_BOOT_SERVICES*
  .NumberOfTblEntries dq ?      ; unsigned 64-bit integer
  .ConfigurationTable dq ?      ; EFI_CONFIGURATION_TABLE *
}
struct EFI_SYSTEM_TABLE

struc EFI_RUNTIME_SERVICES
{
    .Hdr EFI_TABLE_HEADER
    .GetTime        dq ?        ; EFI_GET_TIME
    .SetTime        dq ?        ; EFI_SET_TIME
    .GetWakeupTime  dq ?        ; EFI_GET_WAKEUP_TIME
    .SetWakeupTime  dq ?        ; EFI_SET_WAKEUP_TIME
    .SetVirtualAddressMap dq ?  ; EFI_SET_VIRTUAL_ADDRESS_MAP
    .ConvertPointer dq ?        ; EFI_CONVERT_POINTER
    .GetVariable    dq ?        ; EFI_GET_VARIABLE
    .GetNextVariableName dq ?   ; EFI_GET_NEXT_VARIABLE_NAME
    .SetVariable    dq ?        ; EFI_SET_VARIABLE
    .GetNextHighMonotonicCount dq ? ; EFI_GET_NEXT_HIGH_MONO_COUNT
    .ResetSystem    dq ?        ; EFI_RESET_SYSTEM
    .UpdateCapsule  dq ?        ; EFI_UPDATE_CAPSULE
    .QueryCapsuleCapabilities dq ? ; EFI_QUERY_CAPSULE_CAPABILITIES
    .QueryVariableInfo dq ?     ; EFI_QUERY_VARIABLE_INFO
}
struct EFI_RUNTIME_SERVICES

struc EFI_BOOT_SERVICES
{
    .Hdr EFI_TABLE_HEADER
    .RaiseTPL           dq ?
    .RestoreTPL         dq ?
    .AllocatePages      dq ?
    .FreePages          dq ?
    .GetMemoryMap       dq ?
    .AllocatePool       dq ?
    .FreePool           dq ?
    .CreateEvent        dq ?
    .SetTimer           dq ?
    .WaitForEvent       dq ?
    .SignalEvent        dq ?
    .CloseEvent         dq ?
    .CheckEvent         dq ?
    .InstallProtocolInterface dq ?
    .ReinstallProtocolInterface dq ?
    .UninstallProtocolInterface dq ?
    .HandleProtocol     dq ?
    .Reserved           dq ?
    .RegisterProtocolNotify dq ?
    .LocateHandle       dq ?
    .LocateDevicePath   dq ?
    .InstallConfigurationTable dq ?
    .LoadImage          dq ?
    .StartImage         dq ?
    .Exit               dq ?
    .UnloadImage        dq ?
    .ExitBootServices   dq ?
    .GetNextMonotonicCount dq ?
    .Stall              dq ?
    .SetWatchdogTimer   dq ?
    .ConnectController  dq ?
    .DisconnectController dq ?
    .OpenProtocol       dq ?
    .CloseProtocol      dq ?
    .OpenProtocolInformation dq ?
    .ProtocolsPerHandle dq ?
    .LocateHandleBuffer dq ?
    .LocateProtocol     dq ?
    .InstallMultipleProtocolInterfaces dq ?
    .UninstallMultipleProtocolInterfaces dq ?
    .CalculateCrc32     dq ?
    .CopyMem            dq ?
    .SetMem             dq ?
    .CreateEventEx      dq ?
}
struct EFI_BOOT_SERVICES

struc EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL {
  .Reset                dq ?    ; EFI_TEXT_RESET
  .OutputString         dq ?    ; EFI_TEXT_STRING
  .TestString           dq ?    ; EFI_TEXT_TEXT_STRING
  .QueryMode            dq ?    ; EFI_TEXT_QUERY_MODE
  .SetMode              dq ?    ; EFI_TEXT_SET_MODE
  .SetAttribute         dq ?    ; EFI_TEXT_SET_ATTRIBUTE
  .ClearScreen          dq ?    ; EFI_TEXT_CLEAR_SCREEN
  .SetCursorPosition    dq ?    ; EFI_TEXT_CLEAR_SCREEN
  .EnableCursor         dq ?    ; EFI_ENABLE_CURSOR
  ; ...
}
struct EFI_SIMPLE_TEXT_OUTPUT_PROTOCOL

struc EFI_SIMPLE_TEXT_INPUT_PROTOCOL {
  .Reset dq ? ; EFI_INPUT_RESET
  .ReadKeyStroke dq ? ; EFI_INPUT_READ_KEY
  ; ...
}
struct EFI_SIMPLE_TEXT_INPUT_PROTOCOL

struc EFI_INPUT_KEY {
  .ScanCode dw ? ; UINT16
  .UnicodeChar dw ? ; CHAR16
  align 8
}
struct EFI_INPUT_KEY
