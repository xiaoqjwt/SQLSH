#ifndef __B_NODE_H
#define __B_NODE_H

class BlockFile;
class BTree;

class BNode
{
public:
    char level_;
    int num_entries_;
    int left_sibling_;
    int right_sibling_;
    float* key_;

    bool dirty_;
    int block_;
    int capacity_;
    BTree* btree_;

    BNode();
    virtual ~BNode();

    virtual void init(int level, BTree* btree);
    virtual void init_restore(BTree* btree, int block);
    virtual void read_from_buffer(char* buf);
    virtual void write_to_buffer(char* buf);
    virtual int get_entry_size();
    virtual int find_position_by_key(float key);
    virtual float get_key(int index);
    virtual BNode* get_left_sibling();
    virtual BNode* get_right_sibling();

    int get_block();
    int get_num_entries();
    int get_level();
    int get_header_size();
    float get_key_of_node();
    bool isFull();
    void set_left_sibling(int left_sibling);
    void set_right_sibling(int right_sibling);
};

class BIndexNode : public BNode
{
public:
    int* son_;

    BIndexNode();
    virtual ~BIndexNode();

    virtual void init(int level, BTree* btree);
    virtual void init_restore(BTree* btree, int block);
    virtual void read_from_buffer(char* buf);
    virtual void write_to_buffer(char* buf);
    virtual int get_entry_size();
    virtual int find_position_by_key(float key);
    virtual float get_key(int index);

    virtual BIndexNode* get_left_sibling();
    virtual BIndexNode* get_right_sibling();

    int get_son(int index);
    void add_new_child(float key, int son);
};

class BLeafNode : public BNode
{
public:
    int num_keys_;
    int* id_;
    int capacity_keys_;

    BLeafNode();
    virtual BLeafNode();

    virtual void init(int level, BTree* btree);
    virtual void init_restore(BTree* btree, int block);
    virtual void read_from_buffer(char* buf);
    virtual void write_to_buffer(char* buf);
    virtual int get_entry_size();
    virtual int find_position_by_key(float key);
    virtual float get_key(int index);
    virtual BLeafNode* get_left_sibling();
    virtual BLeafNode* get_right_sibling();

    int get_key_size(int block_length);
    int get_increment();
    int get_num_keys();
    int get_entry_id(int index);
    void add_new_child(int id, float key);
};


#endif // __B_NODE_H
