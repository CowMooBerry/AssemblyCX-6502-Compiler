#include "Compiler.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

int main(int argc, char** argv)
{
	unsigned int flag = 0;
	unsigned long long int argsSize = 0;
	char** args = (char**)malloc(sizeof(char*));
	args[0] = '\0';

	// Flag 0x01 is help flag
	// Flag 0x02 is binary flag
	for (int i = 1; i < argc; i++) {
		if (strncmp(argv[i], "-", 1) == 0) {
			// char* arg = argv[i];
			argv[i]++;

			if (strcmp(argv[i], "h") == 0) {
				flag |= 0x01;
			} else if (strcmp(argv[i], "bin") == 0) {
				flag |= 0x02;
			}
		} else if (strcmp(argv[i], "help") == 0) {
			flag |= 0x01;
		} else {// that are not flags.
			argsSize++;
			args = (char**)realloc(args, argsSize * sizeof(char*));
			args[argsSize - 1] = (char*)malloc((strlen(argv[i]) + 1) * sizeof(char));
			args[argsSize - 1][strlen(argv[i])] = '\0';

			strcpy(args[argsSize - 1], argv[i]);
		}
	}

	if (flag & 0x01) { // If help flag is set.
		if (args[0] == NULL) {
			printf("Commands:\n");
			printf("  compile   Compiles assembly 6502.\n");
			printf("  hexdump   Show hexdump from a file.\n");
			printf("  help    Shows list of commands.\n");
		} else {
			if (strcmp(args[0], "compile") == 0) {
				printf("compile   Compiles assembly 6502.\n    compiles to binary file\n    -hex compiles to Intel hex file\n");
			}
		}
	} else if (args[0] != NULL) { // Runs commands here
		if (strcmp(args[0], "compile") == 0) {
			if (flag & 0x02) // -bin flag
				if (args[1] != NULL) {
					int err = ASMCX6502_Compile(args[1], args[2], 0);
					if (err)
						printf("Compile Error: %d", err);
				} else {
					printf("Error: no file name given.\n");
				}
			else // if there no flag then compiles to intel hex format.
				ASMCX6502_Compile(args[1], args[2], 1);
		} else if (strcmp(args[0], "hexdump") == 0) {
			if (args[1] != NULL)
				ASMCX6502_Hexdump(args[1]);
			else
				printf("Error: no file name given.\n");
		} else {
			printf("Unknown command. Type -h for help\n");
		}
	}

	for (int i = 0; i < argsSize; i++)
		free(args[i]);

	free(args);
	return 0;
}