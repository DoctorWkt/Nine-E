#include <string.h>

void *memset(void *dest, int data, size_t len)
{
	char *p = (char *)dest;
	char v = (char)data;

	while(len--)
		*p++ = v;
	return dest;
}
