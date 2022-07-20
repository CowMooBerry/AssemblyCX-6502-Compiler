#ifndef NULL
#define NULL 0
#endif

#ifndef ASMCX6502_Compiler
#define _CRT_SECURE_NO_WARNINGS 1
#define ASMCX6502_Compiler
int ASMCX6502_Compile(char* src, char* dest, char format);
int ASMCX6502_Hexdump(char* src);

#endif

