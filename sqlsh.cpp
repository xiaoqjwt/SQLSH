#include "header.h"

int HashValueQsortComp(const void* e1, const void* e2)
{
    int ret = 0;
    HashValue* value1 = (HashValue *)e1;
    HashValue* value2 = (HashValue *)e2;

    if(value1->proj_ < value2->proj_)
    {
        ret = -1;
    }
    else if(value1->proj_ > value2->proj_)
    {
        ret = 1;
    }
    else
    {
        if(value1->id_ < value2->id_)
            ret = -1;
        else if(value1->id_ > value2->id_)
            ret = 1;
    }
    return ret;
}

SQLsh::SQLsh()
{
    n_pts_ = dim_ = B_ = alter = maxk = -1;
    appro_ratio_ = alpha_ = beta_ = delta_ = -1.0f;
    w_ = p1 = p2 = -1.0f;
    m_ = l_ = -1;
    a_array_ = NULL;
    productResult = NULL;
    decision = NULL;
}

SQLsh::~SQLsh()
{
    if(a_array_ != NULL)
    {
        delete[] a_array_;
        a_array_ = NULL;
    }

    if(productResult != NULL)
    {
        for(int i=0; i<n_pts_; i++)
        {
            delete[] productResult[i];
            productResult[i] = NULL;
        }
        delete[] productResult;
        productResult = NULL;
    }

    if(decision != NULL)
    {
        delete[] decision;
        decision = NULL;
    }

    if(radius_index_ != NULL)
    {
        for(int i=0; i<alter_; i++)
        {
            delete radius_index_[i];
            radius_index_[i] = NULL;
        }
        delete radius_index_;
        radius_index_ = NULL;
    }
}

void SQLsh::init(int n, int d, int B, int alter, int maxk, float appro_ratio, char* output_folder, float** data)
{
    n_pts_ = n;
    dim_ = d;
    B_ = B;
    alter_ = alter;
    appro_ratio_ = appro_ratio;
    maxk_ = maxk;

    strcpy(index_path_, output_folder);
    strcat(index_path_, "L2_indices/");

    calc_params();
    gen_hash_func();
    gen_product_value(data);
    construct_decision();
}

void SQLsh::calc_params()
{
    delta_ = 1.0f / E;
    beta_ = 1.5 * maxk_ / n_pts_;

    w_ = sqrt((8.0 * appro_ratio_ * appro_ratio_ * log(appro_ratio_)) / (appro_ratio_ * appro_ratio_ - 1.0f));
    p1_ = calc_l2_prob(w_ / 2.0f);
    p2_ = calc_l2_prob(w_ / (2.0f * appro_ratio_));

    float para1 = sqrt(log(2.0f / beta_));
    float para2 = sqrt(log(1.0f / delta_));
    float para3 = 2.0f * (p1_ - p2_) * (p1_ - p2_);

    float eta = para1 / para2;
    alpha_ = (eta * p1_ + p2_) / (1 + eta);
    m_ = (int)ceil((para1 + para2) * (para1 + para2) / para3);
    l_ = (int)ceil((p1_ * para1 + p2_ * para2) * (para1 + para2) / para3);
}

float SQLsh::calc_l2_prob(float value)
{
    return new_gaussian_prob(value);
}

void SQLsh::gen_hash_func()
{
    int sum = m_ * dim_;
    a_array_ = new float[sum];

    for(int i=0; i<sum; i++)
    {
        a_array_[i] = gaussian(0.0f, 1.0f);
    }
}

void SQLsh::gen_product_value(float** data)
{
    productResult = new float*[n_pts_];

    for(int i=0; i<n_pts_; i++)
    {
        productResult[i] = new float[m_];
        for(int j=0; j<m_; j++)
        {
            productResult[i][j] = calc_hash_value(j, data[i]);
        }
    }

}

float SQLsh::calc_hash_value(int table_id, float* point)
{
    float ret = 0.0f;
    for(int i=0; i<dim_; i++)
    {
        ret += (a_array_[table_id * dim_ + i] * point[i]);
    }
    return ret;
}

void SQLsh::construct_decision()
{
    int** hashResult = new int[n_pts_][dim_];
    bool* isChecked = new bool[n_pts_];
    int* count_num = new int[n_pts_];
    //map<int, list<int>>* result_map = new map<int, list<int>>[m_];
    std::multimap<int, int>* result_map = new std::multimap<int, int>[m_];
    radius_index_ = new std::list<int>[alter_];     // 存储每个半径下的点的ID
    int threshold_maxk = 1.5 * maxk;

    for(int i=0; i<alter_-1; i++)
    {
       // 构建每一个hash表
       float interval = base_R * pow(appro_ratio_, i) * w_;
       for(int j=0; j<n_pts_; j++)
       {
           for(int k=0; k<m_; k++)
           {
               int key_value = (int)ceil(productResult[j][k] / interval);
               hashResult[j][k] = key_value;
               result_map[k].insert(make_pair(key_value, j));
           }
       }

       std::pair<std::multimap<int,int>::iterator, std::multimap<int, int>::iterator> ret;

       // check每一个点的近邻
       for(int j=0; j<n_pts_; j++)
       {
           memset(isChecked, 0, n_pts_*SIZEBOOL);
           memset(count_num, 0, n_pts_*SIZEINT);
           int collision_num = 0;

           for(int k=0; k<m_; k++)
           {
               int key_value = hashResult[j][k];
               ret = result_map[k].equal_range(key_value);
               for(std::multimap<int, int>::iterator tmp_it = ret.first; tmp_it!=ret.second; ++tmp_it)
               {
                   int tmp_id = tmp_it->second;
                   count_num[tmp_id]++;
                   if(count_num[tmp_id]>=l_ && !isChecked[tmp_id])
                   {
                       isChecked[tmp_id] = 1;
                       collision_num++;
                   }
               }
               /***
               map<int, list<int>>::iterator iter;
               iter = result_map[k].find(key_value);
               if(iter != result_map[k].end())
               {
                   list<int> tmp_list = iter->second;
                   list<int>::iterator list_iter;
                   for(list_iter=tmp_list.begin(); list_iter!=tmp_list.end(); list_iter++)
                   {
                        count_num[*list_iter]++;
                        if(count_num[*list_iter]>=l_ && !isChecked)
                        {
                            isChecked[*list_iter] = 1;
                            collision_num++;
                        }
                   }
               }
               else
               {
                   printf("%d hash_value error\n", key_value);
               }
               ***/
           }

           if(decision[j]==-1 && collision_num>threshold_maxk)
           {
               decision[j] = i;
               radius_index_[i].push_back(j);
           }
       }
    }

    for(int i=0; i<n_pts_; i++)
    {
        if(decision[i] == -1)
        {
            decision[i] = alter - 1;
            radius_index_[alter-1].push_back(i);
        }
    }

    // Release the space
    if(hashResult != NULL)
    {
        for(int i=0; i<n_pts_; i++)
        {
            delete[] hashResult[i];
            hashResult[i] = NULL;
        }
        delete[] hashResult;
    }

    if(isChecked != NULL)
    {
        delete[] isChecked;
        isChecked = NULL;
    }

    if(count_num != NULL)
    {
        delete[] count_num;
        count_num = NULL;
    }

    if(result_map != NULL)
    {
        for(int i=0; i<m_; i++)
        {
            delete result_map[i];
            result_map[i] = NULL;
        }
        delete result_map;
        result_map = NULL;
    }
}


void SQLsh::bulkload(float** data)
{
    int len = (int) strlen(index_path_);
    for(int i=0; i<len; i++)
    {
        if(index_path_[i] == '/')
        {
            char ch = index_path_[i+1];
            index_path_[i+1] = '\0';

            int ret = access(index_path_, F_OK);
            if(ret != 0)
            {
                ret = mkdir(index_path_, 0755);
                if(ret != 0)
                {
                    printf("Could not create directory %s\n", index_path_);
                }
            }
            index_path_[i+1] = ch;
        }
    }

    char fname[200];
    strcpy(fname, index_path_);
    strcat(fname, "para");
    if(write_para_file(fname))
        return 1;

    for(int i=0; i<alter_; i++)
    {
        int list_size = radius_index_[i].size();
        HashValue* hashtables = new HashValue[list_size];

        for(int j=0; j<m_; j++)
        {
            std::list<int>::iterator list_iter = radius_index_.begin();
            for(int k=0; k<list_size; k++)
            {
                int tmp_id = *(list_iter+k);
                hashtables[k].id_ = tmp_id;
                hashtables[k].proj_ = productResult[tmp_id][j];
            }

            qsort(hashtables, list_size, sizeof(HashValue), HashValueQsortComp);

            get_tree_filename(i, j, fname);

            BTree* bt = new BTree();
            bt->init(fname, B_);
            if(bt->bulkload(hashtables, list_size))
            {
                return 1;
            }
            delete bt;
            bt = NULL;
        }
        if(hashtables != NULL)
        {
            delete[] hashtables;
            hashtables = NULL;
        }


    }
    return 0;
}

int SQLsh::knn(float* query, int maxk, ResultItem* rslt, char* output_folder)
{
    for(int i=0; i<maxk; i++)
    {
        rslt[i].id_ = -1;
        rslt[i].dist_ = MAXREAL;
    }

    float* data = new float[dim_];
    for(int i=0; i<dim_; i++)
    {
        data[i] - 0.0f;
    }

    int* frequency = new int[n_pts_];
    bool* is_checked = new bool[n_pts_];
    for(int i=0; i<n_pts_; i++)
    {
        frequency[i] = 0;
        bool[i] = false;
    }

    float* q_val = new float[m_];
    for(int i=0; i<m_; i++)
    {
        q_val[i] = calc_hash_value(i, query);
    }

    PageBuffer* lptr = new PageBuffer[m_];
    PageBuffer* rptr = new PageBuffer[m_];
    for(int i=0; i<m_; i++)
    {
        lptr[i].leaf_node_ = NULL;
        lptr[i].index_pos_ = -1;
        lptr[i].leaf_pos_ = -1;
        lptr[i].size_ = -1;

        rptr[i].leaf_node_ = NULL;
        rptr[i].index_pos_ = -1;
        rptr[i].leaf_pos_ = -1;
        rptr[i].size_ = -1;
    }

    page_io_ = 0;
    dist_io_ = 0;
    int candidates = 100 + maxk;

    bool* flag = new bool[m_];
    for(int i=0; i<m_; i++)
    {
        flag[i] = true;
    }

    for(int i=0; i<alter_; i++)
    {
        init_buffer(i, lptr, rptr, q_val, query);
        float bucket_width = base_R * pow(appro_ratio_, i) * w_ / 2.0f;

        bool again = true;
        int flag_num = 0;
        int scanned_id = 0;

        int id = -1;
        int count = -1;
        int start = -1;
        int end = -1;

        float left_dist = -1.0f;
        float right_dist = -1.0f;
        float knn_dist = MAXREAL;

        ResultItem* item = new ResultItem();

        while(again)
        {
            flag_num = 0;
            for(int j=0; j<m_; j++)
            {
                flag[j] = true;
            }

            while(true)
            {
                for(int j=0; j<m_; j++)
                {
                    if(!flag[j])
                        continue;

                    left_dist = -1.0f;
                    if(lptr[j].size_ != -1)
                    {
                        left_dist = calc_proj_dist(&lptr[j], q_val[j]);
                    }

                    right_dist = -1.0f;
                    if(rptr[j].size_ != -1)
                    {
                        right_dist = calc_proj_dist(&rptr[j], q_val[j]);
                    }

                    if(left_dist >=0 && left_dist<bucket_width && ((right_dist >= 0 && left_dist <= right_dist) || right_dist < 0))
                    {
                        count = lptr[j].size_;
                        end = lptr[j].leaf_pos_;
                        start = end - count;
                        for(int k=end; k>start; k--)
                        {
                            id = lptr[j].leaf_node_->get_entry_id(k);
                            frequency[id]++;
                            scanned_id++;

                            if(frequency[id]>l_ && !is_checked[id])
                            {
                                is_checked[id] = true;
                                read_data(id, dim_, B_, data, output_folder);

                                item->dist_ = calc_l2_dist(data, query, dim_);
                                item->id_ = id;
                                knn_dist = update_result(rslt, item, maxk);

                                dist_io_++;
                                if(dist_io_ >= candidates)
                                {
                                    again = false;
                                    flag_num += m_;
                                    break;
                                }
                            }
                        }
                        update_left_buffer(&lptr[j], &rptr[j]);
                    }
                    else if(right_dist >= 0 && right_dist < bucket_width && ((left_dist >= 0 && left_dist > right_dist)|| left_dist<0))
                    {
                        count = rptr[j].size_;
                        start = rptr[j].index_pos_;
                        end = start + count;
                        for(int k=start; k<end; k++)
                        {
                            id = rptr[j].leaf_node_->get_entry_id(k);
                            frequency[id]++;
                            scanned_id++;
                            if(frequency[id]>l_ && !is_checked[id])
                            {
                                is_checked[id] = true;
                                read_data(id, dim_, B_, data, output_folder);

                                item->dist_ = calc_l2_dist(data, query, dim_);
                                item->id_ = id;
                                knn_dist = update_result(rslt, item, maxk);

                                dist_io_++;
                                if(dist_io_ >= candidates)
                                {
                                    again = false;
                                    flag_num += m_;
                                    break;
                                }
                            }
                        }
                        update_right_buffer(&lptr[j], &rptr[j]);
                    }
                    else
                    {
                        flag[i] = false;
                        flag_num++;
                    }
                    if(flag_num >= m_)
                        break;
                }
                if(flag_num >= m_)
                    break;
            }

            if(knn_dist < appro_ratio_ * base_R * pow(appro_ratio_, i) && dist_io_ >= maxk)
            {
                again = false;
                break;
            }

            if(bucket_width == base_R * pow(appro_ratio_, i+1))
            {
                again = false;
                break;
            }

            bucket_width = appro_ratio_ * bucket_width;
        }

        if(item != NULL)
        {
            delete item;
            item = NULL;
        }
    }

    if(data != NULL || frequency!= NULL || is_checked != NULL)
    {
        delete[] data; data = NULL;
        delete[] frequency; frequency = NULL;
        delete[] is_checked; is_checked = NULL;
    }

    if(q_val != NULL || flag != NULL)
    {
        delete[] q_val; q_val = NULL;
        delete[] flag; flag = NULL;
    }

    for(int i=0; i<m_; i++)
    {
        if(lptr[i].leaf_node_ && lptr[i].leaf_node_ != rptr[i].leaf_node_)
        {
            delete lptr[i].leaf_node_;
            lptr[i].leaf_node_ = NULL;
        }
        if(rptr[i].leaf_node_)
        {
            delete rptr[i].leaf_node_;
            rptr[i].leaf_node_ = NULL;
        }
    }
    delete[] lptr; lptr = NULL;
    delete[] rptr; rptr = NULL;

    return (page_io_ + dist_io_);
}

float SQLsh::update_result(ResultItem* rslt, ResultItem* item, int maxk)
{
    int i = -1;
    int pos = -1;
    bool alreadyIn = false;

    for(i=0; i<maxk; i++)
    {
        if(item->id_ == rslt[i].id_)
        {
            alreadyIn = true;
            break;
        }
        else if(compfloats(item->dist_, rslt[i].dist_) == -1)
        {
            break;
        }
    }
    pos = i;

    if(!alreadyIn && pos < maxk)
    {
        for(i = maxk-1; i>pos; i--)
        {
            rslt[i].setto(&(rslt[i-1]));
        }
        rslt[pos].setto(item);
    }
    return rslt[maxk-1].dist_;
}

float SQLsh::calc_proj_dist(const PageBuffer* ptr, float q_val)
{
    int pos = ptr->index_pos_;
    float key = ptr->leaf_node_->get_key(pos);
    float dist = fabs(key - q_val);
    return dist;
}

void SQLsh::init_buffer(int alter_num, PageBuffer* lptr, PageBuffer* rptr, float* q_val, float* query)
{
    int block = -1;
    int follow = -1;
    bool lescape = false;

    int pos = -1;
    int increment = -1;
    int num_entries = -1;

    BIndexNode* index_node = NULL;

    for(int i=0; i<m_; i++)
    {
        block = trees_[alter_num * m_ + i]->root_;
        index_node = new BIndexNode();
        index_node->init_restore(trees_[alter_num * m_ + i], block);
        page_io_++;

        lescape = false;
        while(index_node->get_level()>1)
        {
            follow = index_node->find_position_by_key(q_val[i]);
            if(follow == -1)
            {
                if(lescape)
                {
                    follow = 0;
                }
                else
                {
                    if(block != trees_[alter_num*m_ + i]->root_)
                    {
                        error("knn_bucket No branch found\n", true);
                    }
                    else
                    {
                        follow = 0;
                        lescape = true;
                    }
                }
            }
            block = index_node->get_son(follow);
            delete index_node;
            index_node = NULL;

            index_node = new BIndexNode();
            index_node->init_restore(trees_[alter_num*m_ + i], block);
            page_io_++;
        }

        follow = index_node->find_position_by_key(q_val[i]);
        if(follow < 0)
        {
            lescape = true;
            follow = 0;
        }

        if(lescape)    // only ini right buffer
        {
            block = index_node->get_son(0);
            rptr[i].leaf_node_ = new BLeafNode();
            rptr[i].leaf_node_->init_restore(trees_[alter_num*m_ + i], block);
            rptr[i].index_pos_ = 0;
            rptr[i].leaf_node_ = 0;

            increment = rptr[i].leaf_node_->get_increment();
            num_entries = rptr[i].leaf_node_->get_num_entries();
            if(increment > num_entries)
            {
                rptr[i].size_ = num_entries;
            }
            else
            {
                rptr[i].size_ = increment;
            }
            page_io_++;
        }
        else
        {
            block = index_node->get_son(follow);
            lptr[i].leaf_node_ = new BLeafNode();
            lptr[i].leaf_node_->init_restore(trees_[alter_num*m_ + i], block);

            pos = lptr[i].leaf_node_->find_position_by_key(q_val[i]);
            if(pos < 0)
                pos = 0;
            lptr[i].index_pos_ = pos;
            increment = lptr[i].leaf_node_->get_increment();
            if(pos == lptr[i].leaf_node_->get_num_keys()-1)
            {
                num_entries = lptr[i].leaf_node_->get_num_entries();
                lptr[i].leaf_pos_ = num_entries - 1;
                lptr[i].size_ = num_entries - pos * increment;
            }
            else
            {
                lptr[i].leaf_pos_ = pos * increment + increment - 1;
                lptr[i].size_ = increment;
            }
            page_io_++;

            if(pos < lptr[i].leaf_node_->get_num_keys()-1)
            {
                rptr[i].leaf_node_ = lptr[i].leaf_node_;
                rptr[i].index_pos_ = (pos + 1);
                rptr[i].leaf_pos_ = (pos + 1)*increment;

                if((pos+1) == rptr[i].leaf_node_->get_num_keys() -1)
                {
                    num_entries = rptr[i].leaf_node_->get_num_entries();
                    rptr[i].size_ = num_entries - (pos+1)*increment;
                }
                else
                {
                    rptr[i].size_ = increment;
                }
            }
            else
            {
                rptr[i].leaf_node_ = lptr[i].leaf_node_->get_right_sibling();
                if(rptr[i].leaf_node_)
                {
                    rptr[i].index_pos_ = 0;
                    rptr[i].leaf_pos_ = 0;

                    increment = rptr[i].leaf_node_->get_increment();
                    num_entries = rptr[i].leaf_node_->get_num_entries();
                    if(increment > num_entries)
                    {
                        rptr[i].size_ = num_entries;
                    }
                    else
                    {
                        rptr[i].size_ = increment;
                    }
                    page_io_++;
                }
            }
        }

        if(index_node != NULL)
        {
            delete index_node;
            index_node = NULL;
        }
    }
}

void SQLsh::update_left_buffer(PageBuffer* lptr, const PageBuffer* rptr)
{
    BLeafNode* leaf_node = NULL;
    BLeafNode* old_leaf_node = NULL;

    if(lptr->index_pos_ > 0)
    {
        lptr->index_pos_--;

        int pos = lptr->index_pos_;
        int increment = lptr->leaf_node_->get_increment();
        lptr->leaf_pos_ = pos * increment + increment - 1;
        lptr->size_ = increment;
    }
    else
    {
        old_leaf_node = lptr->leaf_node_;
        leaf_node = lptr->leaf_node_->get_left_sibling();

        if(leaf_node)
        {
            lptr->leaf_node_ = leaf_node;
            lptr->index_pos_ = lptr->leaf_node_->get_num_keys() - 1;

            int pos = lptr->index_pos_;
            int increment = lptr->leaf_node_->get_increment();
            int num_entries = lptr->leaf_node_->get_num_entries();
            lptr->leaf_node_ = num_entries - 1;
            lptr->size_ = num_entries - pos * increment;
            page_io_++;
        }
        else
        {
            lptr->leaf_node_ = NULL;
            lptr->index_pos_ = -1;
            lptr->leaf_pos_ = -1;
            lptr->size_ = -1;
        }

        if(rptr->leaf_node_ != old_leaf_node)
        {
            delete old_leaf_node;
            old_leaf_node = NULL;
        }
    }
}

void SQLsh::update_right_buffer(const PageBuffer* lptr, PageBuffer* rptr)
{
    BLeafNode* leaf_node = NULL;
    BLeafNode* old_leaf_node = NULL;

    if(rptr->index_pos_ < rptr->leaf_node_->get_num_keys()-1)
    {
        rptr->index_pos_++;

        int pos = rptr->index_pos_;
        int increment = rptr->leaf_node_->get_increment();
        rptr->leaf_node_ = pos * increment;
        if(pos == rptr->leaf_node_->get_num_keys()-1)
        {
            int num_entries = rptr->leaf_node_->get_num_entries();
            rptr->size_ = num_entries - pos * increment;
        }
        else
        {
            rptr->size_ = increment;
        }
    }
    else
    {
        old_leaf_node = rptr->leaf_node_;
        leaf_node = rptr->leaf_node_->get_right_sibling();

        if(leaf_node)
        {
            rptr->leaf_node_ = leaf_node;
            rptr->index_pos_ = 0;
            rptr->leaf_pos_ = 0;

            int increment = rptr->leaf_node_->get_increment();
            int num_entries = rptr->leaf_node_->get_num_entries();
            if(increment > num_entries)
            {
                rptr->size_ = num_entries;
            }
            else
            {
                rptr->size_ = increment;
            }
            page_io_++;
        }
        else
        {
            rptr->leaf_node_ = NULL;
            rptr->index_pos_ = -1;
            rptr->leaf_pos_ = -1;
            rptr->size_ = -1;
        }

        if(lptr->leaf_node_ != old_leaf_node)
        {
            delete old_leaf_node;
            old_leaf_node = NULL;
        }
    }
}

int SQLsh::restore(char* output_folder)
{
    strcpy(index_path_, output_folder);
    strcat(index_path_, "L2_indices/");

    char fname[200];
    strcpy(fname, index_path_);
    strcat(fname, "para");

    if(read_para_file(fname))
    {
        return 1;
    }

    trees_ = new BTree*[alter_ * m_];
    for(int i=0; i<alter_; i++)
    {
        for(int j=0; j<m_; j++)
        {
            get_tree_filename(i, j, fname);
            trees_[i*m_ + j] = new BTree();
            trees_[i*m_ + j]->init_restore(fname);
        }
    }

    return 0;
}

void SQLsh::get_tree_filename(int alter_id, int function_id, char* fname)
{
    char c[20];

    strcpy(fname, index_path_);
    sprintf(c, "%d_%d", alter_id, function_id);
    strcat(fname, c);
    strcat(fname, ".sqlsh");
}

int SQLsh::write_para_file(char* fname)
{
    FILE* fp = NULL;
    fp = fopen(fname, "r");
    if(fp)
    {
        error("Hash tables exist.\n", true);
    }

    fp = fopen(fname, "w");
    if(!fp)
    {
        printf("Could not create %s\n", fname);
        printf("Perhaps no such folder %s\n", index_path_);
        return 1;
    }

    fprintf(fp, "n = %d\n", n_pts_);
    fprintf(fp, "d = %d\n", dim_);
    fprintf(fp, "B = %d\n", B_);
    fprintf(fp, "appro_ratio = %f\n", appro_ratio_);
    fprintf(fp, "alter = %d\n", alter_);
    fprintf(fp, "maxk = %d\n", maxk_);
    fprintf(fp, "w = %f\n", w_);
    fprintf(fp, "p1 = %f\n", p1_);
    fprintf(fp, "p2 = %f\n", p2_);
    fprintf(fp, "alpha = %f\n", alpha_);
    fprintf(fp, "beta = %f\n", beta_);
    fprintf(fp, "delta = %f\n", delta_);
    fprintf(fp, "m = %d\n", m_);
    fprintf(fp, "l = %d\n", l_);

    int a_count = 0;
    for(int i=0; i<m_; i++)
    {
        fprintf(fp, "%f", a_array_[a_count++]);
        for(int j=0; j<dim_; j++)
        {
            fprintf(fp, " %f", a_array_[a_count++];
        }
        fprintf(fp, "\n");
    }

    fprintf(fp, "\n");
    fprintf(fp, "The decision Radius is :\n");

    for(int i=0; i<n_pts_; i++)
    {
        if(i%200==0)
        {
            fprintf(fp, "\n");
            fprintf(fp, "%d", decision[i]);
        }
        fprintf(fp, " %d", decision[i]);
    }

    if(fp)
        close(fp);

    return 0;
}

int SQLsh::read_para_file(char* fname)
{
    FILE* fp = NULL;
    fp = fopen(fname, "r");
    if(!fp)
    {
        printf("QALSH::read_para_file could not open %s.\n", fname);
        return 1;
    }

    fscanf(fp, "n = %d\n", &n_pts_);
    fscanf(fp, "d = %d\n", &dim_);
    fscanf(fp, "B = %d\n", &B_);
    fscanf(fp, "appro_ratio = %f\n", &appro_ratio_);
    fscanf(fp, "alter = %d\n", &alter_);
    fscanf(fp, "maxk = %d\n", &maxk_);
    fscanf(fp, "w = %f\n", &w_);
    fscanf(fp, "p1 = %f\n", &p1_);
    fscanf(fp, "p2 = %f\n", &p2_);
    fscanf(fp, "alpha = %f\n", &alpha_);
    fscanf(fp, "beta = %f\n", &beta_);
    fscanf(fp, "delta = %f\n", &delta_);
    fscanf(fp, "m = %d\n", &m_);
    fscanf(fp, "l = %d\n", &l_);

    a_array_ = new float[m_ * dim_];
    int a_count = 0;
    for(int i=0; i<m_; i++)
    {
        fscanf(fp, "%f", &a_array_[a_count++]);
        for(int j=0; j<dim_; j++)
        {
            fscanf(fp, " %f", &a_array_[a_count++];
        }
        fscanf(fp, "\n");
    }

    fscanf(fp, "\n");
    fscanf(fp, "The decision Radius is :\n");

    for(int i=0; i<n_pts_; i++)
    {
        if(i%200==0)
        {
            fscanf(fp, "\n");
            fscanf(fp, "%d", &decision[i]);
        }
        fscanf(fp, " %d", &decision[i]);
    }

    if(fp)
        close(fp);

    return 0;
}


