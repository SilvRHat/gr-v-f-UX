// Multi_verse: RE(Rendering Engine)
// Gavin Zimmerman

// UTILITY
#ifndef MV_RE_UTILS_H
#define MV_RE_UTILS_H

// DEPENDENCIES
#include <stdarg.h>
#include <stdio.h> 
#include <stdlib.h>
#include "graphics.h"


// METHODS
// Error Handlers
void Error(const char* format , ...);
void ErrCheck(const char* where);

// Sorting
void uiVecQuickSort(int elements, int n, const unsigned int val[][n], int idx[]);

#endif