#include "unsigned_array.h"
#include "cosine.h"

double *LSH_gen_unit_vector(unsigned int k);
short LSH_h(unsigned_array *u, double *r, unsigned int k);
unsigned int LSH_signature(unsigned_array *u, double **r, unsigned int k);
unsigned int LSH_distance(unsigned int x, unsigned int y);
void LSH_generator(unsigned int k);
void LSH_generator_initialize(unsigned int k);
void LSH_setup();
void LSH_teardown();

