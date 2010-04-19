#include <stdlib.h>

typedef struct _Instance
{
	unsigned int w_i;
	unsigned int z_i;
	unsigned int d_i;
	unsigned int index;
	char *word;
	struct _Instance *next;
} Instance;

Instance *Instance_new(unsigned int w_i,unsigned int z_i, unsigned int d_i, unsigned int index,char *word);
void Instance_free(Instance *list);
