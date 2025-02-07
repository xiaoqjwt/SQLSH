#ifndef __DEF_H
#define __DEF_H

typedef char Block[];

#define MIN(a, b)	(((a) < (b)) ? (a) : (b))
#define MAX(a, b)	(((a) > (b)) ? (a) : (b))

#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

const float E  = 2.7182818F;
const float PI = 3.141592654F;

const float FLOATZERO = 1e-6F;		// accuracy
const float MAXREAL   = 3.402823466e+38F;
const float MINREAL   = -MAXREAL;

const int   MAXINT    = 2147483647;	// max integer value
const int   MININT    = -MAXINT;	// min integer value

const int SIZEINT = (int)sizeof(int);
const int SIZECHAR = (int)sizeof(char);
const int SIZEFLOAT = (int)sizeof(float);
const int SIZEBOOL = (int)sizeof(bool);

const float base_R = 50.00;
const int INDEX_SIZE_LEAF_NODE = 4096;

const int BFHEAD_LENGTH = (int)(SIZEINT * 2);

#endif // __DEF_H
