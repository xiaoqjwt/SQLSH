#ifndef __B_TREE_H
#define __B_TREE_H

class BlockFile;
class BNode;

struct HashValue;

class BTree
{
public:
    int root_;

    BlockFile* file_;
    BNode* root_ptr_;

    BTree();
    ~BTree();

    void init(char* fname, int b_length);
    void init_restore(char* fname);
    int bulkload(HashValue* hashtable, int n);

    int read_header(char* buf);
    int write_header(char* buf);
    void load_root();
    void delete_root();
};

#endif // __B_TREE_H
