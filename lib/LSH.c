#include "LSH.h"
#include <stdlib.h>
#include <math.h>
#include <gsl/gsl_rng.h>

const gsl_rng_type * T;
gsl_rng * r;
void LSH_setup()
{
    gsl_rng_env_setup();
    
    T = gsl_rng_default;
    r = gsl_rng_alloc (T);
}

void LSH_teardown()
{
    gsl_rng_free(r);
}

// Random unit vector generator, from
// http://burtleburtle.net/bob/rand/unitvec.html
double *static_x=NULL;
double *static_y=NULL;
double *static_z=NULL;
unsigned int static_k=0;
void LSH_generator(unsigned int k)
{
    //copy x to z;
    memcpy(static_z,static_x,sizeof(double)*k);
    //x = x + y;
    //x = x / ||x||;
    double dot = 0;
    for (int i=0; i<k; i++) {
        static_x[i] += static_y[i];
        dot += static_x[i] * static_x[i];
    }
    dot = sqrt(dot);
    for (int i=0; i<k; i++) {
        static_x[i] /= dot;
    }
    //for (i=n;  --i;  )
    //{
    //    swap(x[i], x[rand()%i]);
    //    if (rand()%2) x[i] = -x[i];
    //}
    double temp;
    for (int i=k-1; i>0; i--) {
        unsigned int index = rand()%i;
        temp = static_x[i];
        static_x[i] = temp;
        static_x[index] = temp;
        if (rand()%2) {
            static_x[i] = -static_x[i];
        }
    }
    //y := z;
    //copy z to y;
    memcpy(static_y,static_z,sizeof(double)*k);
}

void LSH_generator_initialize(unsigned int k)
{
    for (int i=0; i<k; i++) {
        static_x[i] = gsl_rng_uniform(r);
        static_y[i] = gsl_rng_uniform(r);
    }
    double dot_x = 0,dot_y = 0;
    for (int i=0; i<k; i++) {
        dot_x += static_x[i] * static_x[i];
        dot_y += static_y[i] * static_y[i];
    }
    dot_x = sqrt(dot_x);
    dot_y = sqrt(dot_y);
    for (int i=0; i<k; i++) {
        static_x[i] /= dot_x;
        static_y[i] /= dot_y;
    }
}

double *LSH_gen_unit_vector(unsigned int k)
{
    if (k > static_k) {
        if (static_k != 0) {
            free(static_x);
            free(static_y);
            free(static_z);
        }
        static_x = (double *)malloc(sizeof(double)*k);
        static_y = (double *)malloc(sizeof(double)*k);
        static_z = (double *)malloc(sizeof(double)*k);
        static_k = k;
        
        // Initialize x,y to seed unit vectors
        LSH_generator_initialize(k);
    } else {
        // Need to run the generator a couple of times to get more randomness
        for (int i=0; i<10; i++) {
            LSH_generator(k);
        }
    }
    
    double *vector = (double *)malloc(sizeof(double)*k);
    memcpy(vector,static_y,sizeof(double)*k);
    return vector;
}

short LSH_h(unsigned_array *u, double *r, unsigned int k)
{
    double dot = 0;
    for (int i=0; i<k; i++) {
        dot += r[i] * unsigned_array_get_zero(u, i);
    }
    return (dot >= 0 ? 1 : 0);
}

unsigned int LSH_signature(unsigned_array *u, double **r, unsigned int k)
{
    unsigned int signature = 0;
    for (int i=0; i<32; i++) {
        signature = signature << 1;
        if (LSH_h(u,r[i],k) == 1) {
            signature++;
        }
    }
    return signature;
}

unsigned int LSH_distance(unsigned int x, unsigned int y)
{
    unsigned dist = 0, val = x ^ y;
    
    // Count the number of set bits
    while(val)
    {
        ++dist; 
        val &= val - 1;
    }
    
    return dist;
}

