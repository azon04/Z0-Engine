#ifndef __ZE_STRING_FUNC_H__
#define __ZE_STRING_FUNC_H__

#include "PrimitiveTypes.h"

#define STRING_FUNC_BUFFER_SIZE 255

namespace StringFunc {
	
	extern char Buffer[STRING_FUNC_BUFFER_SIZE];

	void WriteTo(char* to, const char* from, size_t size);

	// Length of char* string, including null terminated char
	int Length(const char* string);

	// Compare two string (char*), return 0 if the same
	int Compare(const char* string1, const char* string2);

	int CompareCount(const char* string1, const char* string2, int count);

	int FindLast(const char* string1, const char* string2);

	void Concat(const char* string1, const char* string2, char* res, size_t resSize);

	void PrintToString(char* string, int stringSize, const char* formatText, ...);

	// Calculate Hash value for a string.
	// Currently using LookUp3
	ZE::UInt32 Hash(const char* string, size_t size);

};
#endif 
