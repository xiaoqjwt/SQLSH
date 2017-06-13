#include "header.h"

int read_set(int n, int d, char* set, float** points)
{
    FILE* fp = fopen(set, "r");
    if(!fp)
    {
        printf("Could not open %s.\n", set);
        return 1;
    }

    int i=0;
    while(!feof(fp) && i<n)
    {
        for(int j=0; j<d; j++)
        {
            fscanf(fp, " %f", &points[i][j]);
        }
        fscanf(fp, "\n");
        i++;
    }
    if(!feof(fp) && i==n)
    {
        printf("The size of set is larger than you input\n");
    }
    else if(feof(fp) && i<n)
    {
        printf("Set the size of dataset to be %d. ", i);
    }
    fclose(fp);
    return 0;
}

float calc_l2_dist(float* p1, float* p2, int dim)
{
    float diff = 0.0f;
    float ret = 0.0f;
    for(int i=0; i<dim; i++)
    {
        diff = p1[i] - p2[i];
        ret += diff * diff;
    }
    return sqrt(ret);
}

int compfloats(float v1, float v2)
{
    if(v1 - v2 < -FLOATZERO)
        return -1;
    else if(v1 - v2 > FLOATZERO)
        return 1;
    else
        return 0;
}

void error(char* msg, bool is_exit)
{
    printf("%s\n", msg);
    if(is_exit)
        exit(1);
}

int write_data_new_form(int n, int d, int B, float** data, char* output_path)
{
    int num = (int)floor((float)B / (d * SIZEFLOAT));
    int total_file = (int)ceil((float)n / num);

    if(total_file == 0)
    {
        return 1;
    }

    char* data_path = new char[200];
    strcpy(data_path, output_path);
    strcat(data_path, "data/");

    #ifdef LINUX_
    int len = (int)strlen(data_path);
    for(int i=0; i<len; i++)
    {
        if(data_path[i] == '/')
        {
            char ch = data_path[i+1];
            data_path[i+1] = '\0';

            int ret = access(data_path, F_OK);
            if(ret != 0)
            {
                ret = mkdir(data_path, 0755);
                if(ret != 0)
                {
                    printf("Could not create directory %s\n", data_path);
                    error("Write_data_new_form error\n", true);
                }
            }
            data_path[i+1] = ch;
        }
    }
    #endif // LINUX_

    char* fname = new char[200];
    char* buffer = new char[B];
    for(int i=0; i<B; i++)
    {
        buffer[i] = 0;
    }

    int left = 0;
    int right = 0;
    for(int i=0; i<total_file; i++)
    {
        get_data_filename(i, data_path, fname);

        left = i * num;
        right = left + num;
        if(right > n)
            right = n;
        write_data_to_buffer(d, left, right, data, buffer);

        if(write_buffer_to_page(B, fname, buffer) == 1)
        {
            error("Write_data_new_form error to write a page", true);
        }
    }

    if(buffer != NULL)
    {
        delete[] buffer;
        buffer = NULL;
    }

    if(data_path != NULL)
    {
        delete[] data_path;
        data_path = NULL;
    }

    if(fname != NULL)
    {
        delete[] fname;
        fname = NULL;
    }

    return 0;
}

void get_data_filename(int file_id, char* data_path, char* fname)
{
    char c[20];
    strcpy(fname, data_path);
    sprintf(c, "%d", file_id);
    strcat(fname, c);
    strcat(fname, ".data");
}

void write_data_to_buffer(int d, int left, int right, float** data, char* buffer)
{
    int c = 0;
    for(int i=left; i<right; i++)
    {
        for(int j=0; j<d; j++)
        {
            memcpy(&buffer[c], &data[i][j], SIZEFLOAT);
            c += SIZEFLOAT;
        }
    }
}

int write_buffer_to_page(int B, char* fname, char* buffer)
{
    if(fname == NULL || buffer == NULL)
        return 1;

    FILE* fp = fopen(fname, "wb");
    if(!fp)
    {
        printf("Could not create %s.\n", fname);
        return 1;
    }

    fwrite(buffer, B, 1, fp);
    fclose(fp);
    return 0;
}

int read_data(int id, int d, int B, float* data, char* output_path)
{
    char* fname = new char[200];
    char* data_path = new char[200];

    strcpy(data_path, output_path);
    strcat(data_path, "data/");

    int num = (int)floor((float)B / (d*SIZEFLOAT));
    int file_id = (int)floor((float)id / num);

    get_data_filename(file_id, data_path, fname);

    char* buffer = new char[B];
    for(int i=0; i<B; i++)
    {
        buffer[i] = 0;
    }
    if(read_buffer_from_page(B, fname, buffer) == 1)
    {
        error("read_data() error to read a page", true);
    }

    int index = id % num;
    read_data_from_buffer(index, d, data, buffer);

    if(buffer != NULL)
    {
        delete[] buffer;
        buffer = NULL;
    }

    if(data_path != NULL && fname != NULL)
    {
        delete[] data_path; data_path = NULL;
        delete[] fname; fname = NULL;
    }

    return 0;
}

int read_buffer_from_page(int B, char* fname, char* buffer)
{
    if(fname == NULL || buffer == NULL)
        return 1;

    FILE* fp = NULL;
    fp = fopen(fname, "rb");
    if(!fp)
    {
        printf("read_buffer_from_page could not open %s.\n", fname);
        return 1;
    }

    fread(buffer, B, 1, fp);
    fclose(fp);
    return 0;
}

void read_data_from_buffer(int index, int d, float* data, char* buffer)
{
    int c = index *d * SIZEFLOAT;
    for(int i=0; i<d; i++)
    {
        memcpy(&data[i], &buffer[c], SIZEFLOAT);
        c += SIZEFLOAT;
    }
}
