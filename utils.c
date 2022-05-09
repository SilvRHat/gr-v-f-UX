// Multi_verse: RE(Rendering Engine)
// Gavin Zimmerman

// DEPENDENCIES
#include "utils.h"




// SOURCE

// ERROR HANDLERS
// ///////////////////////////////////////////

// Credit to Willem A. (Vlakkies) Schreuder
void Error(const char* format , ...) {
   va_list args;
   va_start(args,format);
   printf("ERROR: ");
   vfprintf(stderr,format,args);
   va_end(args);
   exit(-1);
}

void ErrCheck(const char* where) {
   int err = glGetError();
   char* desc = "No Error";
   switch (err) {
        case GL_NO_ERROR:
            break;
        case GL_INVALID_ENUM:
            desc = "GL_INVALID_ENUM: An unacceptable value is specified for an enumerated argument.";
            break;
        case GL_INVALID_VALUE:
            desc = "GL_INVALID_VALUE: A numeric argument is out of range.";
            break;
        case GL_INVALID_OPERATION:
            desc = "GL_INVALID_OPERATION: The specified operation is not allowed in the current state.";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            desc = "GL_INVALID_FRAMEBUFFER_OPERATION: The framebuffer object is not complete.";
            break;
        case GL_OUT_OF_MEMORY:
            desc = "GL_OUT_OF_MEMORY: There is not enough memory left to execute the command.";
            break;
        default:
            desc = "Unknown Error";
            break;
   }
   if (err) fprintf(stderr,"ERROR: %s \nFrom: (%s)\n",desc,where);
}








// SORTING
// ///////////////////////////////////////////


// Multidimensional Quicksort for unsigned integer vectors
// Partition
static int _uiVecQSPartition(int n, const unsigned int val[][n], int idx[],
            int first, int last
) {
    int pivot = idx[first];

    int lt_pt = first+1;    // Left Point
    int rt_pt = last;       // Right point

    // Sort
    while (1) {
        // Move left point
        while (lt_pt <= rt_pt) {
            // Check value
            int stop = 0;
            for (int i=0; i<n; i++) {
                int li = idx[lt_pt];
                if (val[li][i] == val[pivot][i]) 
                    continue;
                else if (val[li][i] > val[pivot][i]) 
                    stop = 1;
                break;
            }
            if (stop) break;
            lt_pt+=1;
        }

        // Move right point
        while (lt_pt <= rt_pt) {
            // Check value
            int stop = 0;
            for (int i=0; i<n; i++) {
                int ri = idx[rt_pt];
                if (val[ri][i] == val[pivot][i]) 
                    continue;
                else if (val[ri][i] < val[pivot][i]) 
                    stop = 1;
                break;
            }
            if (stop) break;
            rt_pt-=1;
        }
        
        // Partitioning done
        if (rt_pt<lt_pt) break;
        
        // Swap
        int temp = idx[rt_pt];
        idx[rt_pt] = idx[lt_pt];
        idx[lt_pt] = temp;
    }

    // Swap pivot and right point
    idx[first] = idx[rt_pt];
    idx[rt_pt] = pivot;
    return rt_pt;
}

static void _uiVecQSRecursive(int n, const unsigned int val[][n], int idx[], int first, int last);
static void _uiVecQSRecursive(int n, const unsigned int val[][n], int idx[],
            int first, int last
) {
    int split = _uiVecQSPartition(n, val, idx, first, last);
    if (first < split-1) _uiVecQSRecursive(n, val, idx, first, split-1);
    if (split+1 < last)  _uiVecQSRecursive(n, val, idx, split+1, last);
}

void uiVecQuickSort(int elements, int n, const unsigned int val[][n], int idx[]) {
    if (0 < elements-1) _uiVecQSRecursive(n, val, idx, 0, elements - 1);
}