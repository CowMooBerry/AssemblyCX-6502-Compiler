#include "Compiler.h"
#include <stdio.h>

int CompileError = 0;


int strToInt(char* str)
{
	int val = 0;

	char* p = str;
	while (*p)
	{
		if (*p >= '0' && *p <= '9')
			val = (val * 10) + *p - '0';

		p++;
	}

	return val;
}

char hexToInt8(char* str)
{
	char val = 0;
	char* p = str;

	while (*p)
	{
		char c = *p;

		val <<= 4;
		if (c >= '0' && c <= '9') {
			val += c & 0x0F;
		} else if (c >= 'A' && c <= 'F') {
			val += (c & 0x7) + 9;
		}

		p++;
	}

	return val;
}

const char* IntToHex(unsigned long long int val, unsigned char size) // if dataSize is two then it's = four hexadecimal characters
{
	int i = 0;
	unsigned int bitsize = size * 4;

	static char hex[17] = "0000000000000000";

	while (i < size) {
		unsigned char dec = (val >> (bitsize - 4)) & 15;
		if (dec < 10) {
			hex[i] = dec + 0x30;
		} else {
			hex[i] = dec + 0x37;
		}

		val <<= 4;
		i++;
	}
	hex[i] = '\0';

	return hex;
}

// Converts binary string to hex string
// This function needs to be improved.
const char* binToHex(char* str, unsigned char size)
{
	static char hex[17] = "0000000000000000";

	char* p = str;
	unsigned long long int val = 0;

	while (*p)
	{
		char c = *p;
		val *= 2;
		if (c == '1') {
			val++;
		} else if (c != '0') {
			*str = '\0';
			break;
		}

		p++;
	}

	strcpy(hex, IntToHex(val, size));
	hex[p - str] = '\0';

	return hex;
}

const char* hexDumpToBinDump(char* hexDump, unsigned long long int* binDumpSize)
{
	*binDumpSize = 0;
	char* binDump = (char*)malloc((strlen(hexDump) / 2) * sizeof(char));

	char* p = hexDump;
	while (*p != NULL) {
		char hex[3] = "";
		strncpy(hex, p, 2);

		(*binDumpSize)++;
		binDump[(*binDumpSize) - 1] = hexToInt8(hex);
		p += 2;
	}

	return binDump;
}

const char* hexDumpToIntelHexData(char* hexDump, unsigned long long int bytes)
{
	unsigned long long int hexDataSize = 0;
	char* hexData = '\0';
	

	if (bytes <= 4294967295) { // 64KB Address
		int byteCount = 16;
		// NOTE: 1 byte is per character not per hex value
		// Start Line + (Data Line * (Address Size / Data part of the line per hexadecimal per byte)) + Extended Address Line + End Line

		// DATALINE: (44 * (size / byteCount))	includes with new line character and null character.
		// EXTENDED ADDRESS LINE: ((16 bytes per line) * (Counts how many extended address line there will be))
		hexDataSize = 16 + (44 * (bytes / byteCount)) + (16 * (((bytes - 1) / 65536) + 1)) + 13;
		hexData = (char*)malloc(hexDataSize * sizeof(char)); 
		hexData[hexDataSize - 1] = '\0';

		strcpy(hexData, ":020000040000FA\n");

		unsigned short addressH = 0;
		unsigned short addressL = 0;

		char* p = hexDump;

		while (*p != NULL) {
			char checkSum;
			// Adds Extended Address Line when address = 0 and pointer is changed.
			if (addressL == 0 && (p - hexDump) != 0) {
				addressH++;
				strcat(hexData, ":02000004");
				strcat(hexData, IntToHex(addressH, 4));

				// Calculates Check sum
				checkSum = 6 + addressH;
				checkSum = ~checkSum;
				checkSum++;
				strcat(hexData, IntToHex(checkSum, 2));
				strcat(hexData, "\n");
			}

			// Adds Data Line
			unsigned long long int hexChunkSize = 0;
			char hexChunk[33] = "";
			char buffer[16] = ""; // used for itoa function

			for (int i = 0; i < 32; i++) {
				hexChunk[i] = *p;
				if (*p == NULL)
					break;

				p++;
			}

			hexChunkSize = strlen(hexChunk);


			// Calculates Check sum
			checkSum = hexChunkSize / 2;
			checkSum += addressL;
			// checkSum += 0; record type is value of zero no needed to be added
			for (int i = 0; i < hexChunkSize; i += 2) {
				char hex[3] = "";
				strncpy(hex, (hexChunk + i), 2);

				checkSum += hexToInt8(hex);
			}
			
			checkSum = ~checkSum;
			checkSum++;

			// Now it copies the data in this line to hexData

			// Start Code
			strcat(hexData, ":");

			// Byte Count
			strcpy(buffer, IntToHex(hexChunkSize / 2, 2));//itoa(byteCount, buffer, 16);
			strcat(hexData, buffer);

			// Address
			strcpy(buffer, IntToHex(addressL, 4));
			strcat(hexData, buffer);

			// Record Type
			strcat(hexData, "00");

			// Data
			strcat(hexData, hexChunk);

			// Check sum
			strcpy(buffer, IntToHex(checkSum, 2));
			strcat(hexData, buffer);

			// new line
			strcat(hexData, "\n");

			addressL += 16;
		}

		strcat(hexData, ":00000001FF\n");
	}

	return hexData; // returned value must be free to prevent memory leak.
}

char isHex(char* str)
{
	char* p = str;
	while (*p)
	{
		char c = *p;
		if (!(c >= '0' && c <= '9' || c >= 'A' && c <= 'F')) // If the character doesn't match any hex value it returns false
			return 0;

		p++;
	}

	return 1;
}

char cmp(char* str1, char* str2) // Created this to make the strcmp code shorter to use
{
	if (strcmp(str1, str2) == 0)
		return 1; // true

	return 0; // false
}

const char* toUpperCase(char* str)
{
	for (int i = 0; *(str + i) != '\0'; i++)
	{
		if (*(str + i) >= 'a' && *(str + i) <= 'z')
			*(str + i) = *(str + i) ^ ' ';
	}

	return str;
}

const char* toLowerCase(char* str)
{
	for (int i = 0; *(str + i) != '\0'; i++)
	{
		if (*(str + i) >= 'A' && *(str + i) <= 'Z')
			*(str + i) = *(str + i) ^ ' ';
	}

	return str;
}

const char* removeComment(char* str)
{
	char* p = str;

	while (*p) {
		if (*p == ';') {
			*p = '\0';
			break;
		}
		p++;
	};

	return str;
}

// Converts any format number to hexadecimal values so compiler can convert it to binary or hexadecimal.
const char* formatNumber(char* str)
{
	char s[3] = "";
	if (strncmp(str, "#$", 2) != 0) {
		if (strncmp(str, "#", 1) == 0) {
			char val = (char)strToInt(str + 1);
			strcpy(s, IntToHex(val, 2));
		} else if (strncmp(str, "%", 1) == 0) {
			strcpy(s, binToHex((str + 1), 2));
		}

		strcpy(str, s);
	}

	return str;
}

const char* opToHex(char* opcode)
{
	// I use strdup so strtok won't cause undefined behavior
	char* op = strdup(removeComment(opcode));
	if (*op == '\0') // If the opcode it returns
		return '\0';

	// I uppercase the opcode so it works with lowercase and uppercase.
	toUpperCase(op);
	// Define binary value of opcode
	// instr stands for instruction op code and all instructions use 3 characters.
	// adData stands for addressData which stores any 
	char* hex = '\0';
	char addrHex[6];
	char instr[4] = "";
	char* adData = (char*)malloc(1);
	adData[0] = '\0';
	char adMode = 0;

	// Sets Instruction value
	char* p = strtok(op, " ");
	if (strlen(p) > 3) {
		CompileError = 1; // Syntax Error
		return hex;
	}

	strcpy(instr, p);

	// Sets Addresss Data value
	p = strtok(NULL, " ");

	while (p != NULL) {
		adData = realloc(adData, strlen(p) + strlen(adData) + 1);

		strcat(adData, p);
		p = strtok(NULL, " ");
	}

	// Finds the address mode

	if (strlen(adData) <= 0) { // Instructions that use 1 byte and has no address data
		adMode = 0;
	}
	else if (strncmp(adData, "$", 1) == 0) {
		char* a = strtok(adData, "$, "); // address data
		if (!isHex(a)) {
			CompileError = 1; // Syntax Error
			return hex;
		}

		char* indexType = strtok(NULL, ","); // Index type value has x or y values

		if (indexType != NULL) {
			char* p = strtok(NULL, "");

			if (p != NULL || *indexType != 'X' && *indexType != 'Y') {
				CompileError = 1; // Syntax Error
				return hex;
			}
		}

		if (*(a + 2) == NULL) {
			if (indexType != NULL) {
				if (*indexType == 'X')
					adMode = 1; // $LL,X
				else if (*indexType == 'Y')
					adMode = 2; // $LL,Y
			} else {
				adMode = 3; // $LL
			}

			// Sets addr in binary value
			//addrHex = (char*)realloc(addrHex, 3);
			strcpy(addrHex, a);
		}
		else if (*(a + 4) == NULL) {
			if (indexType != NULL) {
				if (*indexType == 'X')
					adMode = 4; // $LLHH,X
				else if (*indexType == 'Y')
					adMode = 5; // $LLHH,Y
			}
			else {
				adMode = 6; // $LLHH
			}

			// Sets address in binary value
			char H[3] = { *a, *(a + 1), '\0' };
			char L[3] = { *(a + 2), *(a + 3), '\0' };

			//addrHex = (char*)realloc(addrHex, 6);
			strcpy(addrHex, L);
			strcat(addrHex, H);
		}
		else {
			CompileError = 2; // Can not address above FFFF.
			return hex;
		}
	}
	else if (strncmp(adData, "#$", 2) == 0) { // #$BB
		adMode = 7;
		char* a = strtok(adData, "#$"); // address data

		if (!isHex(a)) {
			CompileError = 1; // Syntax Error
			return hex;
		}

		if (*(a + 2) != NULL) {
			CompileError = 2; // Can not set register value higher than FF
		}

		//addrHex = (char*)realloc(addrHex, 3);
		strcpy(addrHex, a);

	}
	else if (strncmp(adData, "(", 1) == 0) {
		char* a = strtok(adData, "($, "); // address data

		char* indexType = strtok(NULL, ","); // Index type value has x or y values

		if (indexType != NULL) {
			char* p = strtok(NULL, "");
			if (p != NULL || *indexType != 'X' && *indexType != 'Y' || *indexType == 'X' && *(indexType + 1) != ')' || *indexType == 'Y' && *(indexType - 2) != ')') {
				CompileError = 41; // Syntax Error
				return hex;
			}
		}

		// Changes ) to \0. So it can ignore the )

		p = a;
		while (*p) {
			if (*p == ')')
				*p = '\0';

			p++;
		}

		if (!isHex(a)) {
			CompileError = 1; // Syntax Error
			return hex;
		}


		if (*(a + 2) == NULL) {
			if (indexType != NULL) {
				if (*indexType == 'X')
					adMode = 8; // ($LL,X)
				else if (*indexType == 'Y')
					adMode = 9; // ($LL),Y
			}
			else {
				CompileError = 1; // Syntax Error
				return hex;
			}

			//addrHex = (char*)realloc(addrHex, 3);
			strcpy(addrHex, a);
		}
		else if (*(a + 4) == NULL) {
			if (indexType == NULL) {
				adMode = 10; // ($LLHH)
			}
			else {
				CompileError = 1; // Syntax Error
				return hex;
			}

			char H[3] = { *a, *(a + 1), '\0' };
			char L[3] = { *(a + 2), *(a + 3), '\0' };

			//addrHex = (char*)realloc(addrHex, 6);
			strcpy(addrHex, L);
			strcat(addrHex, H);
		}
	} 
	else {
		if (*adData != NULL) {
			// Converts number formats 
			formatNumber(adData);

			// Continues the code to get address data here
			adMode = 7;
			char* a = strtok(adData, "#$"); // address data

			if (!isHex(a)) {
				CompileError = 1; // Syntax Error
				return hex;
			}

			if (*(a + 2) != NULL) {
				CompileError = 2; // Can not set register value higher than FF
			}

			//addrHex = (char*)realloc(addrHex, 3);
			strcpy(addrHex, a);
		}
		else {
			adMode = -1;
			CompileError = 1; // Syntax Error
			return hex;
		}
	}

	//realloc hex size
	if (adMode == 0) {
		hex = (char*)realloc(hex, 3);
	} else if (adMode <= 3 || adMode >= 7 && adMode <= 9) {
		hex = (char*)realloc(hex, 5);
	} else {
		hex = (char*)realloc(hex, 7);
	}

	// // // // // // //
  // Address Mode Value //
	// // // // // // //
	// 0 OPC          // Accumulator or implied
	// 1 OPC $BB,X    // zeropage, X-indexed	
	// 2 OPC $BB,Y    // zeropage, Y-indexed	
	// 3 OPC $BB      // relative or zeropage
	// 4 OPC $BBBB,X  // absolute, X-indexed
	// 5 OPC $BBBB,Y  // absolute, Y-indexed
	// 6 OPC $BBBB    // absolute
	// 7 OPC #$BB     // immediate
	// 8 OPC ($BB,X)  // X-indexed, indirect
	// 9 OPC ($BB),Y  // indirect, Y-indexed
	// 10 OPC ($BBBB) // indirect
	// // // // // // //

	// Looks for instruction converts it to hex and combines with address hex.
	if (cmp(instr, "BRK") && adMode == 0) {
		strcpy(hex, "00");
	} else if (cmp(instr, "ORA") && adMode == 8) {
		strcpy(hex, "01");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ORA") && adMode == 3) {
		strcpy(hex, "05");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ASL") && adMode == 3) {
		strcpy(hex, "06");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ASL") && adMode == 3) {
		strcpy(hex, "06");
		strcat(hex, addrHex);
	} else if (cmp(instr, "PHP") && adMode == 0) {
		strcpy(hex, "08");
	} else if (cmp(instr, "ORA") && adMode == 7) {
		strcpy(hex, "09");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ASL") && adMode == 0) {
		strcpy(hex, "0A");
	} else if (cmp(instr, "ASL") && adMode == 3) {
		strcpy(hex, "06");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ORA") && adMode == 6) {
		strcpy(hex, "0D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ASL") && adMode == 6) {
		strcpy(hex, "0E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BPL") && adMode == 3) {
		strcpy(hex, "10");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ORA") && adMode == 9) {
		strcpy(hex, "11");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ORA") && adMode == 1) {
		strcpy(hex, "15");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ASL") && adMode == 1) {
		strcpy(hex, "16");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CLC") && adMode == 0) {
		strcpy(hex, "18");
	} else if (cmp(instr, "ORA") && adMode == 5) {
		strcpy(hex, "19");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ORA") && adMode == 4) {
		strcpy(hex, "1D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ASL") && adMode == 4) {
		strcpy(hex, "1E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "JSR") && adMode == 6) {
		strcpy(hex, "20");
		strcat(hex, addrHex);
	} else if (cmp(instr, "AND") && adMode == 8) {
		strcpy(hex, "21");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BIT") && adMode == 3) {
		strcpy(hex, "24");
		strcat(hex, addrHex);
	} else if (cmp(instr, "AND") && adMode == 3) {
		strcpy(hex, "25");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROL") && adMode == 3) {
		strcpy(hex, "26");
		strcat(hex, addrHex);
	} else if (cmp(instr, "PLP") && adMode == 0) {
		strcpy(hex, "28");
	} else if (cmp(instr, "AND") && adMode == 7) {
		strcpy(hex, "29");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROL") && adMode == 0) {
		strcpy(hex, "2A");
	} else if (cmp(instr, "BIT") && adMode == 6) {
		strcpy(hex, "2C");
		strcat(hex, addrHex);
	} else if (cmp(instr, "AND") && adMode == 6) {
		strcpy(hex, "2D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROL") && adMode == 6) {
		strcpy(hex, "2E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BMI") && adMode == 3) {
		strcpy(hex, "30");
		strcat(hex, addrHex);
	} else if (cmp(instr, "AND") && adMode == 9) {
		strcpy(hex, "31");
		strcat(hex, addrHex);
	} else if (cmp(instr, "AND") && adMode == 1) {
		strcpy(hex, "35");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROL") && adMode == 1) {
		strcpy(hex, "36");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SEC") && adMode == 0) {
		strcpy(hex, "38");
	} else if (cmp(instr, "AND") && adMode == 5) {
		strcpy(hex, "39");
		strcat(hex, addrHex);
	} else if (cmp(instr, "AND") && adMode == 4) {
		strcpy(hex, "3D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROL") && adMode == 4) {
		strcpy(hex, "3E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "RTI") && adMode == 0) {
		strcpy(hex, "40");
	} else if (cmp(instr, "EOR") && adMode == 8) {
		strcpy(hex, "41");
		strcat(hex, addrHex);
	} else if (cmp(instr, "EOR") && adMode == 3) {
		strcpy(hex, "45");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LSR") && adMode == 3) {
		strcpy(hex, "46");
		strcat(hex, addrHex);
	} else if (cmp(instr, "PHA") && adMode == 0) {
		strcpy(hex, "48");
	} else if (cmp(instr, "EOR") && adMode == 7) {
		strcpy(hex, "49");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LSR") && adMode == 0) {
		strcpy(hex, "4A");
	} else if (cmp(instr, "JMP") && adMode == 6) {
		strcpy(hex, "4C");
		strcat(hex, addrHex);
	} else if (cmp(instr, "EOR") && adMode == 6) {
		strcpy(hex, "4D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LSR") && adMode == 6) {
		strcpy(hex, "4E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BVC") && adMode == 3) {
		strcpy(hex, "50");
		strcat(hex, addrHex);
	} else if (cmp(instr, "EOR") && adMode == 9) {
		strcpy(hex, "51");
		strcat(hex, addrHex);
	} else if (cmp(instr, "EOR") && adMode == 1) {
		strcpy(hex, "55");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LSR") && adMode == 1) {
		strcpy(hex, "56");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CLI") && adMode == 0) {
		strcpy(hex, "58");
	} else if (cmp(instr, "EOR") && adMode == 5) {
		strcpy(hex, "59");
		strcat(hex, addrHex);
	} else if (cmp(instr, "EOR") && adMode == 4) {
		strcpy(hex, "5D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LSR") && adMode == 4) {
		strcpy(hex, "5E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "RTS") && adMode == 0) {
		strcpy(hex, "60");
	} else if (cmp(instr, "ADC") && adMode == 8) {
		strcpy(hex, "61");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ADC") && adMode == 3) {
		strcpy(hex, "65");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROR") && adMode == 3) {
		strcpy(hex, "66");
		strcat(hex, addrHex);
	} else if (cmp(instr, "PLA") && adMode == 0) {
		strcpy(hex, "68");
	} else if (cmp(instr, "ADC") && adMode == 7) {
		strcpy(hex, "69");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROR") && adMode == 0) {
		strcpy(hex, "6A");
	} else if (cmp(instr, "JMP") && adMode == 10) {
		strcpy(hex, "6C");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ADC") && adMode == 6) {
		strcpy(hex, "6D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROR") && adMode == 6) {
		strcpy(hex, "6E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BVS") && adMode == 3) {
		strcpy(hex, "70");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ADC") && adMode == 9) {
		strcpy(hex, "71");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ADC") && adMode == 1) {
		strcpy(hex, "75");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROR") && adMode == 1) {
		strcpy(hex, "76");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SEI") && adMode == 0) {
		strcpy(hex, "78");
	} else if (cmp(instr, "ADC") && adMode == 5) {
		strcpy(hex, "79");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ADC") && adMode == 1) {
		strcpy(hex, "7D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "ROR") && adMode == 1) {
		strcpy(hex, "7E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STA") && adMode == 8) {
		strcpy(hex, "81");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STY") && adMode == 3) {
		strcpy(hex, "84");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STA") && adMode == 3) {
		strcpy(hex, "85");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STX") && adMode == 3) {
		strcpy(hex, "86");
		strcat(hex, addrHex);
	} else if (cmp(instr, "DEY") && adMode == 0) {
		strcpy(hex, "88");
	} else if (cmp(instr, "TXA") && adMode == 0) {
		strcpy(hex, "8A");
	} else if (cmp(instr, "STY") && adMode == 6) {
		strcpy(hex, "8C");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STA") && adMode == 6) {
		strcpy(hex, "8D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STX") && adMode == 6) {
		strcpy(hex, "8E");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BCC") && adMode == 3) {
		strcpy(hex, "90");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STA") && adMode == 9) {
		strcpy(hex, "91");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STY") && adMode == 1) {
		strcpy(hex, "94");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STA") && adMode == 1) {
		strcpy(hex, "95");
		strcat(hex, addrHex);
	} else if (cmp(instr, "STX") && adMode == 2) {
		strcpy(hex, "96");
		strcat(hex, addrHex);
	} else if (cmp(instr, "TYA") && adMode == 0) {
		strcpy(hex, "98");
	} else if (cmp(instr, "STA") && adMode == 5) {
		strcpy(hex, "99");
		strcat(hex, addrHex);
	} else if (cmp(instr, "TXS") && adMode == 0) {
		strcpy(hex, "9A");
	} else if (cmp(instr, "STA") && adMode == 4) {
		strcpy(hex, "9D");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDY") && adMode == 7) {
		strcpy(hex, "A0");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDA") && adMode == 8) {
		strcpy(hex, "A1");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDX") && adMode == 7) {
		strcpy(hex, "A2");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDY") && adMode == 3) {
		strcpy(hex, "A4");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDA") && adMode == 3) {
		strcpy(hex, "A5");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDX") && adMode == 3) {
		strcpy(hex, "A6");
		strcat(hex, addrHex);
	} else if (cmp(instr, "TAY") && adMode == 0) {
		strcpy(hex, "A8");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDA") && adMode == 7) {
		strcpy(hex, "A9");
		strcat(hex, addrHex);
	} else if (cmp(instr, "TAX") && adMode == 0) {
		strcpy(hex, "AA");
	} else if (cmp(instr, "LDY") && adMode == 6) {
		strcpy(hex, "AC");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDA") && adMode == 6) {
		strcpy(hex, "AD");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDX") && adMode == 6) {
		strcpy(hex, "AE");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BCS") && adMode == 3) {
		strcpy(hex, "B0");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDA") && adMode == 9) {
		strcpy(hex, "B1");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDY") && adMode == 1) {
		strcpy(hex, "B4");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDA") && adMode == 1) {
		strcpy(hex, "B5");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDX") && adMode == 1) {
		strcpy(hex, "B6");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CLV") && adMode == 0) {
		strcpy(hex, "B8");
	} else if (cmp(instr, "LDA") && adMode == 5) {
		strcpy(hex, "B9");
		strcat(hex, addrHex);
	} else if (cmp(instr, "TSX") && adMode == 0) {
		strcpy(hex, "BA");
	} else if (cmp(instr, "LDY") && adMode == 4) {
		strcpy(hex, "BC");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDA") && adMode == 4) {
		strcpy(hex, "BD");
		strcat(hex, addrHex);
	} else if (cmp(instr, "LDX") && adMode == 4) {
		strcpy(hex, "BE");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CPY") && adMode == 7) {
		strcpy(hex, "C0");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CMP") && adMode == 8) {
		strcpy(hex, "C1");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CPY") && adMode == 3) {
		strcpy(hex, "C4");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CMP") && adMode == 3) {
		strcpy(hex, "C5");
		strcat(hex, addrHex);
	} else if (cmp(instr, "DEC") && adMode == 3) {
		strcpy(hex, "C6");
		strcat(hex, addrHex);
	} else if (cmp(instr, "INY") && adMode == 0) {
		strcpy(hex, "C8");
	} else if (cmp(instr, "CMP") && adMode == 7) {
		strcpy(hex, "C9");
		strcat(hex, addrHex);
	} else if (cmp(instr, "DEX") && adMode == 0) {
		strcpy(hex, "CA");
	} else if (cmp(instr, "CPY") && adMode == 6) {
		strcpy(hex, "CC");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CMP") && adMode == 6) {
		strcpy(hex, "CD");
		strcat(hex, addrHex);
	} else if (cmp(instr, "DEC") && adMode == 6) {
		strcpy(hex, "CE");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BNE") && adMode == 3) {
		strcpy(hex, "D0");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CMP") && adMode == 9) {
		strcpy(hex, "D1");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CMP") && adMode == 1) {
		strcpy(hex, "D5");
		strcat(hex, addrHex);
	} else if (cmp(instr, "DEC") && adMode == 1) {
		strcpy(hex, "D6");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CLD") && adMode == 0) {
		strcpy(hex, "D8");
	} else if (cmp(instr, "CMP") && adMode == 5) {
		strcpy(hex, "D9");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CMP") && adMode == 4) {
		strcpy(hex, "DD");
		strcat(hex, addrHex);
	} else if (cmp(instr, "DEC") && adMode == 4) {
		strcpy(hex, "DE");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CPX") && adMode == 7) {
		strcpy(hex, "E0");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SBC") && adMode == 8) {
		strcpy(hex, "E1");
		strcat(hex, addrHex);
	} else if (cmp(instr, "CPX") && adMode == 3) {
		strcpy(hex, "E4");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SBC") && adMode == 3) {
		strcpy(hex, "E5");
		strcat(hex, addrHex);
	} else if (cmp(instr, "INC") && adMode == 3) {
		strcpy(hex, "E6");
		strcat(hex, addrHex);
	} else if (cmp(instr, "INX") && adMode == 0) {
		strcpy(hex, "E8");
	} else if (cmp(instr, "SBC") && adMode == 7) {
		strcpy(hex, "E9");
		strcat(hex, addrHex);
	} else if (cmp(instr, "NOP") && adMode == 0) {
		strcpy(hex, "EA");
	} else if (cmp(instr, "CPX") && adMode == 6) {
		strcpy(hex, "EC");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SBC") && adMode == 6) {
		strcpy(hex, "ED");
		strcat(hex, addrHex);
	} else if (cmp(instr, "INC") && adMode == 6) {
		strcpy(hex, "EE");
		strcat(hex, addrHex);
	} else if (cmp(instr, "BEQ") && adMode == 3) {
		strcpy(hex, "F0");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SBC") && adMode == 9) {
		strcpy(hex, "F1");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SBC") && adMode == 1) {
		strcpy(hex, "F5");
		strcat(hex, addrHex);
	} else if (cmp(instr, "INC") && adMode == 1) {
		strcpy(hex, "F6");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SED") && adMode == 0) {
		strcpy(hex, "F8");
	} else if (cmp(instr, "SBC") && adMode == 5) {
		strcpy(hex, "F9");
		strcat(hex, addrHex);
	} else if (cmp(instr, "SBC") && adMode == 4) {
		strcpy(hex, "FD");
		strcat(hex, addrHex);
	} else if (cmp(instr, "INC") && adMode == 4) {
		strcpy(hex, "FE");
		strcat(hex, addrHex);
	} else {
		CompileError = 3; // Instruction doesn't exist
		return hex;
	}

	// Cleans up
	free(adData);

	// Returns string of hex values
	return hex;
}

// It only can use hexdump only on binary files
int ASMCX6502_Hexdump(char* src)
{
	char* fileData;

	// Reads the file.
	FILE* fp;
	fp = fopen(src, "rb");
	if (fp == NULL) {
		printf("File not found.\n");
		return -1;
	}

	fseek(fp, 0L, SEEK_END);

	// Get File Size
	unsigned long long int fileSize = ftell(fp);
	if (fileSize == 0) {
		printf("File has no data.");
		return -2;
	}

	fileData = (char*)malloc(fileSize + 1);
	rewind(fp);

	// Read File
	fread(fileData, fileSize, 1, fp);
	fclose(fp);
	fileData[fileSize] = '\0';

	// Print Hexdump
	int address = 0;

	char printAsterisk = 0; // used to check if it printed * once.
	char* p = fileData;
	char line[16];
	for (long long int i = 0; i < fileSize; i++)
	{
		// If current line is the same as the next line then don't print
		if (address % 16 == 0) {
			char str[16] = "";
			for (char i = 0; i < 16; i++)
				str[i] = *(p + 15);

			if (strncmp(str, (p + 16), 16) == 0 || strncmp(str, (p - 16), 16) == 0) {
				if (i != 0 && i + 16 < fileSize) {
					if (printAsterisk == 0) {
						printf("*\n");
						printAsterisk = 1;
					}
				}
			} else {
				printAsterisk = 0;
			}
		}

		line[i % 16] = *p;

		if (printAsterisk == 0) {
			// Prints address for every line
			if (address % 16 == 0)
			{
				printf("%s  ", IntToHex(address, 8));
			}

			printf("%s ", IntToHex(*p, 2));

			// Prints space between 16 hexadecimal values per line.
			if (address % 16 == 8)
				printf(" ");

			// Prints new line after the last hexadecimal value.
			// Prints ascii.
			if (address % 16 == 15) {
				printf(" |");
				for (char i = 0; i < 16; i++) {
					if (line[i] > 0x1F && line[i] < 0x7F)
						printf("%c", line[i]);
					else
						printf(".");
				}
				printf("|\n");
			}
		}

		address++;
		p++;
	}

	//If the last line isn't 16 bytes then it will print ascii and prints extra spaces.
	if (address % 16 != 0) {
		for (char i = 15 - (address % 16); i >= 0; i--)
		{
			//Prints spaces to align the ascii with the other lines above.
			printf("   ");

			// Prints space between 16 hexadecimal values per line.
			if (i == 8)
				printf(" ");
		}

		printf(" |");
		for (char i = 0; i < 16; i++) {
			if (line[i] > 0x1F && i < address % 16)
				printf("%c", line[i]);
			else
				printf(".");
		}
		printf("|\n");
	}
}
// format 0 is binary
// format 1 is intel hex
int ASMCX6502_Compile(char* src, char* dest, char format) 
{
	CompileError = 0;

	// Reads the file
	char* fileData;

	FILE* fp;
	fp = fopen(src, "rb");
	if (fp == NULL) {
		printf("File not found!\n");
		return -1;
	}

	// Calculates the size of the file.
	fseek(fp, 0L, SEEK_END);
	unsigned long long int fileSize = (unsigned long long int)ftell(fp);
	if (fileSize == 0) {
		printf("File has no data.");
		return -2;
	}

	fileData = (char*)malloc(fileSize + 1);
	rewind(fp);

	fread(fileData, fileSize, 1, fp);
	fclose(fp);

	fileData[fileSize] = '\0';
// where compiled code will go to.
//fopen(&fp, dest, "w");

	// Splits every line of fileData and saves to string** array;
	unsigned long long int dataSize = 0;

	char** data = (char**)malloc(1 * sizeof(char*));

	char* p = strtok(fileData, "\n");
	while (p != NULL) {
		//free(data);
		data = (char**)realloc(data, (dataSize + 1) * sizeof(char*));

		//data = (char**)malloc((dataSize + 1) * sizeof(char*));
		data[dataSize] = (char*)malloc((strlen(p) + 1) * sizeof(char));

		strcpy(data[dataSize], p);

		if (data[dataSize][strlen(p) - 1] == 0x0D) // If it's a \r it replaces with null character
		  data[dataSize][strlen(p) - 1] = '\0';

		p = strtok(NULL, "\n");
		dataSize++;
	}

	unsigned long long int hexDumpSize = 0;
	char* hexDump = (char*)malloc(1);
	hexDump[0] = '\0';
	// Compiles here
	for (long long int i = 0; i < dataSize; i++) {
		char* hex = opToHex(data[i]);
		//printf("%s", hex);

		if (CompileError == 0) {// If there is no errors the continue converting opcode to hexadecimal values
			if (hex) {// If it has hex data.
				hexDumpSize += strlen(hex) + 1;

				hexDump = (char*)realloc(hexDump, (hexDumpSize + 1) * sizeof(char));
				strcat(hexDump, hex);
			}
		} else {
			break;
		}

		free(hex);
	}

	// Prints hex dump
	//if (CompileError == 0)
		//printf("%s\n", hexDump);

	if (format == 0) {
		// Converts hex to binary and puts in a new file.
		unsigned long long int binDumpSize = 0;
		char* binDump = hexDumpToBinDump(hexDump, &binDumpSize);

		// puts binDump into a new file
		fp = fopen(dest, "wb");
		fwrite(binDump, binDumpSize, 1, fp);
		fclose(fp);

		free(binDump);
	} else if (format == 1) {
		// Converts hex to intel hex format and puts in a new file.
		char* intelHexData = hexDumpToIntelHexData(hexDump, hexDumpSize);

		// puts intelHexData into a new file
		fp = fopen(dest, "wb");
		fwrite(intelHexData, strlen(intelHexData), 1, fp);
		fclose(fp);

		free(intelHexData);
	}

	// Cleans rest of the data
	free(hexDump);

	for (long long int i = 0; i < dataSize; i++)
		free(data[i]);

	free(data);

	free(fileData);

	return CompileError;
}
