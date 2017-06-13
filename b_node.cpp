#include "header.h"

BNode::BNode()
{
    level_ = -1;
    num_entries_ = -1;
    left_sibling_ = right_sibling_ = -1;
    key_ = NULL;

    block_ = capacity_ = -1;
    dirty_ = false;
    btree_ = NULL;
}

BNode::~BNode()
{
    key_ = NULL;
    btree_ = NULL;
}


void BNode::init(int level, BTree* btree)
{
    btree_ = btree;
    level_ = (char)level;

    dirty_ = true;
    left_sibling_ = -1;
    right_sibling_ = -1;
    key_ = NULL;

    num_entries_ = 0;
    block_ = -1;
    capacity_ = -1;
}

void BNode::init_restore(BTree* btree, int block)
{
    btree_ = btree;
    block_ = block;

    dirty_ = false;
    left_sibling_ = -1;
    right_sibling_ = -1;
    key_ = NULL;

    num_entries_ = 0;
    level_ = -1;
    capacity_ = -1;
}

int BNode::get_entry_size()
{
    return 0;
}

void BNode::read_from_buffer(char* buf)
{

}

void BNode::write_to_buffer(char* buf)
{

}

int BNode::find_position_by_key(float key)
{
    return -1;
}

float BNode::get_key(int index)
{
    return -1.0f;
}

BNode* BNode::get_left_sibling()
{
    BNode* node = NULL;
    if(left_sibling_ != -1)
    {
        node = new BNode();
        node->init_restore(btree_, left_sibling_);
    }
    return node;
}

BNode* BNode::get_right_sibling()
{
    BNode* node = NULL;
    if(right_sibling_ != -1)
    {
        node = new BNode();
        node->init_restore(btree_, right_sibling_);
    }
    return node;
}

int BNode::get_block()
{
    return block_;
}

int BNode::get_num_entries()
{
    return num_entries_;
}

int BNode::get_level()
{
    return level_;
}

int BNode::get_header_size()
{
    // level  num_entries left_sibling right_sibling
    int header_size = SIZECHAR + SIZEINT*3;
    return header_size;
}

float BNode::get_key_of_node()
{
    return key_[0];
}

bool BNode::isFull()
{
    if(num_entries_ >= capacity_)
        return true;
    else
        return false;
}

void BNode::set_left_sibling(int left_sibling)
{
    left_sibling_ = left_sibling;
}

void BNode::set_right_sibling(int right_sibling)
{
    right_sibling_ = right_sibling;
}


// BIndexNode
BIndexNode::BIndexNode()
{
    level_ = -1;
    num_entries_ = -1;
    left_sibling_ = right_sibling_ = -1;
    block_ = capacity_ = -1;
    dirty_ = false;
    btree_ = NULL;

    key_ = NULL;
    son_ = NULL;
}


BIndexNode::~BIndexNode()
{
    char* buf = NULL;
    if(dirty_)
    {
        int block_length = btree_->file_->get_blocklength();
        buf = new char[block_length];
        write_to_buffer(buf);
        btree_->file_->write_block(buf, block_);

        delete[] buf;
        buf = NULL;
    }

    if(key_)
    {
        delete[] key_;
        key_ = NULL;
    }

    if(son_)
    {
        delete[] son_;
        son_ = NULL;
    }
}

void BIndexNode::init(int level, BTree* btree)
{
    btree_ = btree;
    level_ = (char)level;

    num_entries_ = 0;
    left_sibling_ = -1;
    right_sibling_ = -1;
    dirty_ = true;

    int b_length = btree->file_->get_blocklength();
    capacity_ = (b_length - get_header_size()) / get_entry_size();
    if(capacity_ < 50)
    {
        printf("capacity = %d\n", capacity_);
        error("BIndexNode::init() capacity too small.\n", true);
    }

    key_ = new float[capacity_];
    for(int i=0; i<capacity_; i++)
    {
        key_[i] = MINREAL;
    }
    son_ = new int[capacity_];
    for(int i=0; i<capacity_; i++)
    {
        son_[i] = -1;
    }

    char* blk = new char[b_length];
    block_ = btree->file_->append_block(blk);

    delete[] blk;
    blk = NULL;
}

void BIndexNode::init_restore(BTree* btree, int block)
{
    btree_ = btree;
    block_ = block;
    dirty_ = false;

    int b_length = btree_->file_->get_blocklength();
    capacity_ = (b_length - get_header_size()) / get_entry_size();
    if(capacity_ < 50)
    {
        printf("capacity = %d\n", capacity_);
        error("BIndexNode::init() capacity too small.\n", true);
    }
    key_ = new float[capacity_];
    for(int i=0; i<capacity_; i++)
    {
        key_[i] = MINREAL;
    }
    son_ = new int[capacity_];
    for(int i=0; i<capacity_; i++)
    {
        son_[i] = -1;
    }

    char* blk = new char[b_length];
    btree->file_->read_block(blk, block);
    read_from_buffer(blk);

    delete[] blk;
    blk = NULL;
}

int BIndexNode::get_entry_size()
{
    // key_ and son_
    int entry_size = SIZEFLOAT + SIZEINT;
    return entry_size;
}

void BIndexNode::read_from_buffer(char* buf)
{
    int i=0;
    memcpy(&level_, &buf[i], SIZECHAR);
    i += SIZECHAR;

    memcpy(&num_entries_, &buf[i], SIZEINT);
    i += SIZEINT;

    memcpy(&left_sibling_, &buf[i], SIZEINT);
    i += SIZEINT;

    memcpy(&right_sibling_, &buf[i], SIZEINT);
    i += SIZEINT;

    for(int j=0; j<num_entries_; j++)
    {
        memcpy(&key_[j], &buf[i], SIZEFLOAT);
        i += SIZEFLOAT;

        memcpy(&son_[j], &buf[i], SIZEINT);
        i += SIZEINT;
    }
}

void BIndexNode::write_to_buffer(char* buf)
{
    int i=0;
    memcpy(&buf[i], &level_; SIZECHAR);
    i += SIZECHAR;

    memcpy(&buf[i], &num_entries_, SIZEINT);
    i += SIZEINT;

    memcpy(&buf[i], &left_sibling_, SIZEINT);
    i += SIZEINT;

    memcpy(&buf[i], &right_sibling_, SIZEINT);
    i += SIZEINT;

    for(int j=0; j<num_entries_; j++)
    {
        memcpy(&buf[i], &key_[j], SIZEFLOAT);
        i += SIZEFLOAT;

        memcpy(&buf[i], &key_[j], SIZEINT);
        i += SIZEINT;
    }
}

int BIndexNode::find_position_by_key(float key)
{
    int pos = -1;

    for(int i=num_entries_; i>=0; i--)
    {
        if(key_[i] <= key)
        {
            pos = i;
            break;
        }
    }
    return pos;
}

float BIndexNode::get_key(int index)
{
    if(index < 0 || index >= num_entries_)
        error("BIndexNode:: get_key out of range.\n", true);

    return key_[index];
}

BIndexNode* BIndexNode::get_left_sibling()
{
    BIndexNode* node = NULL;
    if(left_sibling_ != -1)
    {
        node = new BIndexNode();
        node->init_restore(btree_, left_sibling_);
    }
    return node;
}


BIndexNode* BIndexNode::get_right_sibling()
{
    BIndexNode* node = NULL;
    if(right_sibling_ != -1)
    {
        node = new BIndexNode();
        node->init_restore(btree_, right_sibling_);
    }
    return node;
}


int BIndexNode::get_son(int index)
{
    if(index < 0 || index >= num_entries_)
    {
        error("BIndexNode::get_son out of range\n", true);
    }
    return son_[index];
}

void BIndexNode::add_new_child(float key, int son)
{
    if(num_entries_ >= capacity_)
    {
        error("BIndexNode::add_child overflow\n", true);
    }

    key_[num_entries_] = key;
    son_[num_entries_] = son;

    num_entries_++;
    dirty_ = true;
}


// BLeafNode
BLeafNode::BLeafNode()
{
    level_ = -1;
    num_entries_ = -1;
    left_sibling_ = right_sibling_ = -1;

    block_ = capacity_ = -1;
    dirty_ = false;
    btree_ = NULL;

    num_keys_ = -1;
    capacity_keys_ = -1;
    key_ = NULL;
    id_ = NULL;
}

BLeafNode::~BNode()
{
    char* buf = NULL;
    if(dirty_)
    {
        int block_length = btree_->file_->get_blocklength();
        buf = new char[block_length];
        write_to_buffer(buf);
        btree_->file_->write_block(buf, block_);

        delete[] buf;
        buf = NULL;
    }

    if(key_)
    {
        delete[] key_;
        key_ = NULL;
    }

    if(id_)
    {
        delete[] id_;
        id_ = NULL;
    }
}

void BLeafNode::init(int level, BTree* btree)
{
    btree_ = btree;
    level_ = (char)level;

    num_entries_ = 0;
    num_keys_ = 0;
    left_sibling_ = -1;
    right_sibling_ = -1;
    dirty_ = true;

    int b_length = btree_->file_->get_blocklength();
    int key_size = get_key_size(b_length);

    key_ = new float[capacity_keys_];
    for(int i=0; i<capacity_keys_; i++)
    {
        key_[i] = MINREAL;
    }

    int header_size = get_header_size();
    int entry_size = get_entry_size();
    capacity_ = (b_length - header_size - key_size) / entry_size;
    if(capacity_ <100)
    {
        printf("capacity = %d\n", capacity_);
        error("BLeafNode::init capacity too small.\n", true);
    }
    id_ = new int[capacity_];
    for(int i=0; i<capacity_; i++)
    {
        id_[i] = -1;
    }

    char* blk = new char[b_length];
    block_ = btree_->file_->append_block(blk);
    delete[] blk;
    blk = NULL;
}

void BLeafNode::init_restore(BTree* btree, int block)
{
    btree_ = btree;
    block_ = block;
    dirty_ = false;

    int b_length = btree_->file_->get_blocklength();
    int key_size = get_key_size(b_length);

    key_ = new float[capacity_keys_];
    for(int i=0; i<capacity_keys_; i++)
    {
        key_[i] = MINREAL;
    }

    int header_size = get_header_size();
    int entry_size = get_entry_size();
    capacity_ = (b_length - header_size - key_size) / entry_size;
    if(capacity_ < 100)
    {
        printf("capacity = %d\n", capacity_);
        error("BLeafNode::init_store capacity too small.\n", true);
    }
    id_ = new int[capacity_];
    for(int i=0; i<capacity_; i++)
    {
        id_[i] = -1;
    }

    char* blk = new char[b_length];
    btree_->file_->read_block(blk, block);
    read_from_buffer(blk);

    delete[] blk;
    blk = NULL;
}

void BLeafNode::read_from_buffer(char* buf)
{
    int i=0;

    memcpy(&level_, &buf[i], SIZECHAR);
    i += SIZECHAR;

    memcpy(&num_entries_, &buf[i], SIZEINT);
    i += SIZEINT;

    memcpy(&left_sibling_, &buf[i], SIZEINT);
    i += SIZEINT;

    memcpy(&right_sibling_, &buf[i], SIZEINT);
    i += SIZEINT;

    memcpy(&num_keys_, &buf[i], SIZEINT);
    i += SIZEINT;

    for(int j=0; j<capacity_keys_; j++)
    {
        memcpy(&key_[j], &buf[i], SIZEFLOAT);
        i += SIZEFLOAT;
    }

    for(int j=0; j<num_entries_; j++)
    {
        memcpy(&id_[j], &buf[i], SIZEINT);
        i += SIZEINT;
    }
}

void BLeafNode::write_to_buffer(char* buf)
{
    int i=0;

    memcpy(&buf[i], &level_, SIZECHAR);
    i += SIZECHAR;

    memcpy(&buf[i], &num_entries_, SIZEINT);
    i += SIZEINT;

    memcpy(&buf[i], &left_sibling_, SIZEINT);
    i += SIZEINT;

    memcpy(&buf[i], &right_sibling_, SIZEINT);
    i += SIZEINT;

    memcpy(&buf[i], &num_keys_, SIZEINT);
    i += SIZEINT;

    for(int j=0; j<capacity_keys_; j++)
    {
        memcpy(&buf[i], &key_[j], SIZEFLOAT);
        i += SIZEFLOAT;
    }

    for(int j=0; j<num_entries_; j++)
    {
        memcpy(&buf[i], &id_[j], SIZEINT);
        i += SIZEINT;
    }
}

int BLeafNode::find_position_by_key(float key)
{
    int pos = -1;
    for(int i=num_keys_-1; i>=0; i--)
    {
        if(key_[i] <= key)
        {
            pos = i;
            break;
        }
    }
    return pos;
}

float BLeafNode::get_key(int index)
{
    if(index < 0 || index >= num_keys_)
    {
        error("BLeafNode::get_key out of range.", true);
    }
    return key_[index];
}

BLeafNode* BLeafNode::get_left_sibling()
{
    BLeafNode* node = NULL;
    if(left_sibling_ != -1)
    {
        node = new BLeafNode();
        node->init_restore(btree_, left_sibling_);
    }
    return node;
}

BLeafNode* BLeafNode::get_right_sibling()
{
    BLeafNode* node = NULL;
    if(right_sibling_ != -1)
    {
        node = new BLeafNode();
        node->init_restore(btree_, right_sibling_);
    }
    return node;
}

int BLeafNode::get_key_size(int block_length)
{
    capacity_keys_ = (int)ceil((float)block_length / INDEX_SIZE_LEAF_NODE);

    // arrays of keys and number_keys_
    int key_size = capacity_keys_ * SIZEFLOAT + SIZEINT;
    return key_size;
}

int BLeafNode::get_increment()
{
    int entry_size = get_entry_size();
    int increment = INDEX_SIZE_LEAF_NODE / entry_size;

    return increment;
}

int BLeafNode::get_num_keys()
{
    return num_keys_;
}

int BLeafNode::get_entry_id(int index)
{
    if(index < 0 || index >= num_entries_)
    {
        error("BLeafNode:: get_entry_id out of range.", true);
    }
    return id_[index];
}

void BLeafNode::add_new_child(int id, float key)
{
    if(num_entries_ >= capacity_)
    {
        error("BLeafNode:: add_new_child entry ouverflow", true);
    }

    id_[num_entries_] = id;

    if((num_entries_ * SIZEINT) % INDEX_SIZE_LEAF_NODE == 0)
    {
        if(num_keys_ >= capacity_keys_)
        {
            error("BLeafNode:: add_new_child key overflow", true);
        }

        key_[num_keys_] = key;
        num_keys_++;
    }

    num_entries_++;
    dirty_ = true;
}

int BLeafNode::get_entry_size()
{
    return SIZEINT;
}
