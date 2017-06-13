#include "header.h"

int n = 60000;
int qn = 1000;
int d = 50;
int B = 4096;
int maxk = 100;

float appro_ratio = 1.2;
int alter = 20;

char data_set[200] = "./data/Mnist/Mnist.ds";
char query_set[200] = "./data/Mnist/Mnist.q";
char truth_set[200] = "./data/Mnist/Mnist.tr";
char output_folder[200] = "./data/Mnist/results/";

void ground_truth(int n, int qn, int d, int maxk, char* data_set, char* query_set, char* truth_set)
{
    clock_t start_time, end_time;

    start_time = clock();
    float** data = new float*[n];
    for(int i=0; i<n; i++)
    {
        data[i] = new float[d];
    }
    if(read_set(n, d, data_set, data))
    {
        printf("Reading data set Error!\n");
    }

    float** query = new float*[qn];
    for(int i=0; i<qn; i++)
    {
        query[i] = new float[d];
    }
    if(read_set(qn, d, query_set, query))
    {
        printf("Reading query set Error!\n");
    }
    end_time = clock();

    printf("Read Dataset and Query Set: %.3f MilliSeconds\n\n", (float)end_time-start_time);

    // using linear scan to get ground truth
    start_time = clock();
    float dist = -1.0f;
    float* knndist = new float[maxk];
    int* ids = new int[maxk];

    fp = fopen(truth_set, "w");
    if(!fp)
    {
        printf("Could not open truth_set %s.\n", truth_set);
        return 1;
    }

    fprintf(fp, "qn : %d\t maxk : %d\n", qn, maxk);
    for(int i=0; i<qn; i++)
    {
        for(int j=0; j<maxk; j++)
        {
            knndist[j] = MAXREAL;
            ids = -1;
        }

        for(int k=0; k<n; k++)
        {
            dist = calc_l2_dist(data[k], query[i], d);

            int ii, jj;
            for(jj=0; jj<maxk; jj++)
            {
                if(compfloats(dist, knndist[jj]) == -1)
                {
                    break;
                }
            }
            if(jj < maxk)
            {
                for(ii = maxk-1; ii>=jj+1; ii--)
                {
                    knndist[ii] = knndist[ii-1];
                }
                knndist[jj] = dist;
                ids[jj] = k;
            }
        }
        fprintf(fp, "%d", i+1;);
        for(int j=0; j<maxk; j++)
        {
            fprintf(fp, " %8d(%.6f)\t", ids[j], knndist[j]);
        }
        fprintf(fp, "\n");
    }
    fclose(fp);
    end_time = clock();
    printf("Generate Ground Truth : %.6f MilliSeconds\n\n", (float)end_time-start_time);

    // Release space
    if(data != NULL)
    {
        for(int i=0; i<n; i++)
        {
            delete[] data[i];
            data[i] = NULL;
        }
        delete[] data;
        data = NULL;
    }

    if(query != NULL)
    {
        for(int i=0; i<qn; i++)
        {
            delete[] query[i];
            query[i] = NULL;
        }
        delete[] query;
        query = NULL;
    }

    if(knndist != NULL)
    {
        delete[] knndist;
        knndist = NULL;
    }

    if(ids != NULL)
    {
        delete[] ids;
        ids = NULL;
    }

}

void indexing(int n, int d, int B, float appro_ratio, int alter,int maxk, char* data_set, char* output_folder)
{
    // Read data set
    float** data = new float*[n];
    for(int i=0; i<n; i++)
    {
        data[i] = new float[d];
    }
    if(read_set(n, d, data_set, data))
    {
        printf("Reading data set Error!\n");
    }

    SQLsh* lsh =  new SQLsh();
    lsh->init(n, d, B, alter, maxk, appro_ratio, output_folder, data);
    lsh->bulkload(data);

    write_data_new_form(n, d, B, data, output_folder);

    // Release the space
    if(data != NULL)
    {
        for(int i=0; i<n; i++)
        {
            delete[] data[i];
            data[i] = NULL;
        }
        delete[] data;
        data = NULL;
    }

    if(lsh != NULL)
    {
        delete lsh;
        lsh = NULL;
    }
}

int lshknn(int qn, int d, char* query_set, char* truth_set, char* output_folder)
{
    float** query = new float[qn];
    for(int i=0; i<qn; i++)
        query[i] = new float[d];
    if(read_set(qn, d, query_set, query))
    {
        error("Reading Query set error!\n", true);
    }

    float* truth_dist = new float[qn * maxk];
    int* truth_id = new int[qn * maxk];
    FILE* fp = fopen(truth_set, "r");
    if(!fp)
    {
        printf("Could not open the ground truth file.\n");
        return 1;
    }

    fscanf(fp, "qn : %d\t maxk : %d\n", &qn, &maxk);
    int tmp_id = 0;
    for(int i=0; i<qn; i++)
    {
        fscanf(fp, "%d", &tmp_id);
        for(int j=0; j<maxk; j++)
        {
            fscanf(fp, " %8d(%.6f)\t", truth_id[i*maxk + j], truth_dist[i*maxk + j]);
        }
        fscanf(fp, "\n");
    }
    fclose(fp);

    ResultItem* rslt = new ResultItem[maxk];
    for(int i=0; i<maxk; i++)
    {
        rslt[i].id_ = -1;
        rslt[i].dist_ = MAXREAL;
    }

    SQLsh* lsh = new SQLsh();
    if(lsh->restore(output_folder))
    {
        error("Could not restore SQLSH\n", true);
    }

    char output_set[200];
    strcpy(output_set, output_folder);
    strcat(output_set, "lsh.out");
    File* fp = fopen(output_set, "w");
    if(!fp)
    {
        printf("Could not create the output file\n");
        return 1;
    }
    printf("KNN search:\n");
    int thisIO = 0;
    int allIO = 0;
    float thisRatio = 0.0;
    float allRatio = 0.0;
    for(int i=0; i<qn; i++)
    {
        thisIO = lsh->knn(query[i], maxk, rslt, output_folder);
    }
}


int main()
{
    ground_truth(n, qn, d,maxk, data_set, query_set, truth_set);

    indexing(n, d, B, appro_ratio, alter, data_set, output_folder);

    return 0;
}
