#ifndef MATRIX_H
#define MATRIX_H

#include <math.h>

void rotate_matrix(double angle, double x, double y, double z, float *R);
void perspective_matrix(double fovy, double aspect, double znear, double zfar, float *P);
void multiply_matrix(float *A, float *B, float *C);

#ifndef M_PI
	#define M_PI 3.14159265358979323846
#endif /* M_PI */

#endif
