/*
 *  test.h
 *  ctools
 *
 *  Created by Trevor Fountain on 3/22/10.
 *  Copyright 2010 __MyCompanyName__. All rights reserved.
 *
 */

#include <stdio.h>
#include <assert.h>

#define DO_TEST(__NAME__,__FUNC__) printf("Testing %s...",__NAME__); __FUNC__(); printf("OK\n");
#define DEBUG 1