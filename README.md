# TinyOS V2 - a tiny operating system using the UEFI subsystem

Proof of concept only. Don't expect this to ever be finished.

## Microsoft x64 calling convention

The Microsoft x64 calling convention is followed on Windows and pre-boot UEFI (for long mode on x86-64). The first four arguments are placed onto the registers. That means RCX, RDX, R8, R9 for integer, struct or pointer arguments (in that order), and XMM0, XMM1, XMM2, XMM3 for floating point arguments. Additional arguments are pushed onto the stack (right to left). Integer return values (similar to x86) are returned in RAX if 64 bits or less. Floating point return values are returned in XMM0. Parameters less than 64 bits long are not zero extended; the high bits are not zeroed.

Structs and unions with sizes that match integers are passed and returned as if they were integers. Otherwise they are replaced with a pointer when used as an argument. When an oversized struct return is needed, another pointer to a caller-provided space is prepended as the first argument, shifting all other arguments to the right by one place.

When compiling for the x64 architecture in a Windows context (whether using Microsoft or non-Microsoft tools), stdcall, thiscall, cdecl, and fastcall all resolve to using this convention.

In the Microsoft x64 calling convention, it is the caller's responsibility to allocate 32 bytes of "shadow space" on the stack right before calling the function (regardless of the actual number of parameters used), and to pop the stack after the call. The shadow space is used to spill RCX, RDX, R8, and R9, but must be made available to all functions, even those with fewer than four parameters.

The registers RAX, RCX, RDX, R8, R9, R10, R11 are considered volatile (caller-saved).

The registers RBX, RBP, RDI, RSI, RSP, R12, R13, R14, and R15 are considered nonvolatile (callee-saved).

For example, a function taking 5 integer arguments will take the first to fourth in registers, and the fifth will be pushed on top of the shadow space. So when the called function is entered, the stack will be composed of (in ascending order) the return address, followed by the shadow space (32 bytes) followed by the fifth parameter.

In x86-64, Visual Studio 2008 stores floating point numbers in XMM6 and XMM7 (as well as XMM8 through XMM15); consequently, for x86-64, user-written assembly language routines must preserve XMM6 and XMM7 (as compared to x86 wherein user-written assembly language routines did not need to preserve XMM6 and XMM7). In other words, user-written assembly language routines must be updated to save/restore XMM6 and XMM7 before/after the function when being ported from x86 to x86-64.

Starting with Visual Studio 2013, Microsoft introduced the __vectorcall calling convention which extends the x64 convention. 
