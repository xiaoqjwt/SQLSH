#include "stdlib.h"
#include "stdio.h"

int read_and_write_set(						// read (data or query) set from disk
	int n,								// number of data points
	int d,								// dimensionality
	char* set,							// address of dataset
	int** points)						// data or queries (return)
{
	int i = 0;
	int j = 0;
	FILE* fp = NULL;

	fp = fopen(set, "r");			// open data file
	if (!fp) {
		printf("I could not open %s.\n", set);
		return 1;
	}

	i = 0;
	while (!feof(fp) && i < n) {	// read data file
		fscanf(fp, "%d", &j);
		for (j = 0; j < d; j++) {
			fscanf(fp, " %d", &points[i][j]);
		}
		fscanf(fp, "\n");

		i++;
	}
	if (!feof(fp) && i == n) {		// check the size of set
		printf("The size of set is larger than you input, i is %d\n", i);
	}
	else if (feof(fp) && i < n) {
		printf("Set the size of dataset to be %d. ", i);
	}
	fclose(fp);						// close data file

	fp = fopen(set, "w");
	if(!fp)
    {
        printf("Could not open %s\n", set);
        return 1;
    }

    for(int i=0; i<n; i++)
    {
        for(int j=0; j<d; j++)
        {
            fprintf(fp, "%d ", points[i][j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);

	return 0;
}


int main()
{
    int num = 60000;
    int dim = 50;
    char *file_path = "./Mnist.q";
    int **data = new int*[num];
    for(int i=0; i<num; i++)
    {
        data[i] = new int[dim];
    }

    read_and_write_set(num, dim, file_path, data);

    if(data != NULL)
    {
        for(int i=0; i<num; i++)
        {
            delete[] data[i];
            data[i] = NULL;
        }
        delete[] data;
        data = NULL;
    }
}
