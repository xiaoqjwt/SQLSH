#ifndef __SQLSH_H
#define __SQLSH_H

class BNode;
class BTree;
class BLeafNode;
class BIndexNode;

struct HashValue{
    int id_;
    float proj_;
}HashValue;

struct ResultItem{
    int id_;
    float dist_;

    void setto(ResultItem* item)
    {
        id_ = item->id_;
        dist_ = item->dist_;
    }
}ResultItem;

struct PageBuffer{
    BLeafNode* leaf_node_;
    int index_pos_;
    int leaf_pos_;
    int size_;
}PageBuffer;

class SQLsh{
public:
    int n_pts_;         // 数据集大小
    int dim_;           // 数据维度
    int B_;             // page大小
    float appro_ratio_; // 论文中的c，即半径扩大倍数
    int alter_;
    int maxk_;

    float w_;           // 切割间隔
    float p1_;          // locality中的概率p1
    float p2_;          // locality中的概率p2

    float alpha_;       // 论文中的 l = alpha * m
    float beta_;        // 论文中的 beta_*n + k -1
    float delta_;       // delta controls the success rate of any LSH-based method for c-ANN search,文中说为 error probability

    int m_;             // 哈希函数的个数
    int l_;             // 阈值，超过该阈值即可认为是潜在近邻

    char index_path_[200];

    float* a_array_;    // 投影函数
    float** productResult;  // 内积
    int* decision;          // 点放在指定半径下的索引
    BTree** trees_;

    std::list<int>* radius_index_;

    int page_io_;
    int dist_io_;

    SQLsh();
    ~SQLsh();

    void init(int n, int d, int B, int alter, int maxk, float appro_ratio, char* output_folder, float** data);
    void calc_params();
    float calc_l2_prob(float value);
    void gen_hash_func();
    void gen_product_value(float** data);
    float calc_hash_value(int table_id, float* point);
    void construct_decision();
    void bulkload(float** data);
    int write_para_file(char* fname);
    void get_tree_filename(int alter_id, int function_id, char* fname);

    int restore(char* output_folder);
    int read_para_file(char* fname);
    int knn(float* query, int maxk, ResultItem* rslt, char* output_folder);
    void init_buffer(int alter_num, PageBuffer* lptr, PageBuffer* rptr, float* q_val, float* query);
    float calc_proj_dist(const PageBuffer* ptr, float q_val);
    float update_result(ResultItem* rslt, ResultItem* item, int maxk);
    void update_left_buffer(PageBuffer* lptr, const PageBuffer* rptr);
    void update_right_buffer(const PageBuffer* lptr, PageBuffer* rptr);
};


int HashValueQsortComp(const void* e1, const void* e2);

#endif // __SQLSH_H
