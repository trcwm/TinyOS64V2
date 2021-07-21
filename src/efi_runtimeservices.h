#pragma once

#include <cstdint>
#include "efi_base.h"

typedef void* EFI_GET_TIME;
typedef void* EFI_SET_TIME;
typedef void* EFI_GET_WAKEUP_TIME;
typedef void* EFI_SET_WAKEUP_TIME;

typedef void* EFI_SET_VIRTUAL_ADDRESS_MAP;
typedef void* EFI_CONVERT_POINTER;

typedef void* EFI_GET_VARIABLE;
typedef void* EFI_GET_NEXT_VARIABLE_NAME;
typedef void* EFI_SET_VARIABLE;

typedef void* EFI_GET_NEXT_HIGH_MONO_COUNT;
typedef void* EFI_RESET_SYSTEM;

typedef void* EFI_UPDATE_CAPSULE;
typedef void* EFI_QUERY_CAPSULE_CAPABILITIES;
typedef void* EFI_QUERY_VARIABLE_INFO;

typedef EFI_STATUS (*EFI_CONVERT_POINTER)(
    uint64_t   debugDisposition,
    void       **address
);

struct EFI_RUNTIME_SERVICE
{  
    EFI_TABLE_HEADER    m_header;                    
    
    // Time Services
    EFI_GET_TIME        m_getTime;
    EFI_SET_TIME        m_setTime;                     
    EFI_GET_WAKEUP_TIME m_getWakeupTime;               
    EFI_SET_WAKEUP_TIME m_setWakeupTime;                
    
    // Virtual Memory Services
    EFI_SET_VIRTUAL_ADDRESS_MAP    SetVirtualAddressMap;       
    EFI_CONVERT_POINTER             ConvertPointer;              
    
    // Variable Services
    EFI_GET_VARIABLE               GetVariable;                
    EFI_GET_NEXT_VARIABLE_NAME     GetNextVariableName;        
    EFI_SET_VARIABLE               SetVariable;                 
    
    // Miscellaneous Services 
    EFI_GET_NEXT_HIGH_MONO_COUNT   GetNextHighMonotonicCount; 
    EFI_RESET_SYSTEM    m_resetSystem; 
 
    // UEFI 2.0 Capsule Services //   
    EFI_UPDATE_CAPSULE              UpdateCapsule;
    EFI_QUERY_CAPSULE_CAPABILITIES  QueryCapsuleCapabilities;  
 
    // Miscellaneous UEFI 2.0 Service  
    EFI_QUERY_VARIABLE_INFO         QueryVariableInfo;         
};