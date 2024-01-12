#include <string.h>
#include <stdio.h>

int main(void)
{
	char* x = "Hello, ";
	char* y = "world!";

	size_t out_len = strlen(x) + strlen(y) + 1;
	char out[out_len];
	snprintf(out, out_len, "%s%s", x, y);
	puts(out);
	return 0;
}
