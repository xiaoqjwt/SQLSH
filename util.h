#ifndef __UTIL_H
#define __UTIL_H

int read_set(int n, int d, char* set, float** points);

float calc_l2_dist(float* p1, float* p2, int dim);

int compfloats(float v1, float v2);

void error(char* msg, bool is_exit);

int write_data_new_form(int n, int d, int B, float** data, char* output_path);

void get_data_filename(int data_id, char* data_path, char* fname);

void write_data_to_buffer(int d, int left, int right, float** data, char* buffer);

int write_buffer_to_page(int B, char* fname, char* buffer);

int read_data(int id, int d, int B, float* data, char* output_path);

int read_buffer_from_page(int B, char* fname, char* buffer);

void read_data_from_buffer(int index, int d, float* data, char* buffer);

#endif // __UTIL_H
