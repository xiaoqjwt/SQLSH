#include "header.h"

BTree::BTree()
{
    root_ = -1;
    file_ = NULL;
    root_ptr_ = NULL;
}

BTree::~BTree()
{
    if(root_ptr_ != NULL)
    {
        delete root_ptr_;
        root_ptr_ = NULL;
    }

    char* header = new char[file_->get_blocklength()];
    write_header(header);
    file_->set_header(header);

    delete[] header;
    header = NULL;

    if(file_ != NULL)
    {
        delete file_;
        file_ = NULL;
    }
}

void BTree::init(char* fname, int b_length)
{
    FILE* fp = fopen(fname, "r");
    if(fp)
    {
        error("The file %s exists\n", true);
    }

    file_ = new BlockFile(fname, b_length);
    root_ptr_ = new BIndexNode();
    root_ptr_->init(0, this);
    root_ = root_ptr_->get_block();
    delete_root();
}

int BTree::bulkload(HashValue* hashtable, int n)
{
    BIndexNode* index_child = NULL;
    BIndexNode* index_prev_nd = NULL;
    BIndexNode* index_act_nd = NULL;

    BLeafNode* leaf_child = NULL;
    BLeafNode* leaf_prev_nd = NULL;
    BLeafNode* leaf_act_nd = NULL;

    int id = -1;
    int block = -1;
    float key = MINREAL;

    bool first_node = false;
    int start_block = -1;
    int end_block = -1;

    int current_level = -1;
    int last_start_block = -1;
    int last_end_block = -1;

    start_block = 0;
    end_block = 0;
    first_node = true;

    for(int i=0; i<n; i++)
    {
        id = hashtable[i].id_;
        key = hashtable[i].proj_;

        if(!leaf_act_nd)
        {
            leaf_act_nd = new BLeafNode();
            leaf_act_nd->init(0, this);

            if(first_node)
            {
                first_node = false;
                start_block = leaf_act_nd->get_block();
            }
            else
            {
                leaf_act_nd->set_left_sibling(leaf_prev_nd->get_block());
                leaf_prev_nd->set_right_sibling(leaf_act_nd->get_block());

                delete leaf_prev_nd;
                leaf_prev_nd = NULL;
            }
            end_block = leaf_act_nd->get_block();
        }
        leaf_act_nd->add_new_child(id, key);

        if(leaf_act_nd->isFull())
        {
            leaf_prev_nd = leaf_act_nd;
            leaf_act_nd = NULL;
        }
    }

    if(leaf_prev_nd != NULL)
    {
        delete leaf_prev_nd;
        leaf_prev_nd = NULL;
    }

    if(leaf_act_nd != NULL)
    {
        delete leaf_act_nd;
        leaf_act_nd = NULL;
    }

    current_level = 1;
    last_start_block = start_block;
    last_end_block = end_block;

    while(last_end_block > last_start_block)
    {
        first_node = true;
        for(int i=last_start_block; i<=last_end_block; i++)
        {
            block = i;
            if(current_level == 1)
            {
                leaf_child = new BLeafNode();
                leaf_child->init_restore(this, block);
                key = leaf_child->get_key_of_node();

                delete leaf_child;
                leaf_child = NULL;
            }
            else
            {
                index_child = new BIndexNode();
                index_child->init_restore(this, block);
                key = index_child->get_key_of_node();

                delete index_child;
                index_child = NULL;
            }

            if(!index_act_nd)
            {
                index_act_nd = new BIndexNode();
                index_act_nd->init(current_level, this);

                if(first_node)
                {
                    first_node = false;
                    start_block = index_act_nd->get_block();
                }
                else
                {
                    index_act_nd->set_left_sibling(index_prev_nd->get_block());
                    index_prev_nd->set_right_sibling(index_act_nd->get_block());

                    delete index_prev_nd;
                    index_prev_nd = NULL;
                }
                end_block = index_act_nd->get_block();
            }
            index_act_nd->add_new_child(key, block);

            if(index_act_nd->isFull())
            {
                index_prev_nd = index_act_nd;
                index_act_nd = NULL;
            }
        }

        if(index_prev_nd != NULL)
        {
            delete index_prev_nd;
            index_prev_nd = NULL;
        }
        if(index_act_nd != NULL)
        {
            delete index_act_nd;
            index_act_nd = NULL;
        }

        last_start_block = start_block;
        last_end_block = end_block;
    }
    root_ = last_start_block;

    if(index_prev_nd != NULL)
    {
        delete index_prev_nd;
        index_prev_nd = NULL;
    }
    if(index_act_nd != NULL)
    {
        delete index_act_nd;
        index_act_nd = NULL;
    }
    if (index_child != NULL) {
		delete index_child; index_child = NULL;
	}
	if (leaf_prev_nd != NULL) {
		delete leaf_prev_nd; leaf_prev_nd = NULL;
	}
	if (leaf_act_nd != NULL) {
		delete leaf_act_nd; leaf_act_nd = NULL;
	}
	if (leaf_child != NULL) {
		delete leaf_child; leaf_child = NULL;
	}

	return 0;
}

void BTree::load_root()
{
    if(root_ptr_ == NULL)
    {
        root_ptr_ = new BIndexNode();
        root_ptr_->init_restore(this, root_);
    }
}

void BTree::delete_root()
{
    if(root_ptr_ != NULL)
    {
        delete root_ptr_;
        root_ptr_ = NULL;
    }
}

void BTree::init_restore(char* fname)
{
    FILE* fp = fopen(fname, "r");
    if(!fp)
    {
        printf("tree file %s does not exist\n", fname);
        delete[] fname;
        fname = NULL;
        error("", true);
    }
    fclose(fp);

    file_ = new BlockFile(fname, 0);
    root_ptr_ = NULL;
    char* header = new char[file_->get_blocklength()];
    file_->read_header(header);
    read_header(header);

    if(header != NULL)
    {
        delete[] header;
        header = NULL;
    }
}

int BTree::read_header(char* buf)
{
    memcpy(&root_, buf, SIZEINT);
    return SIZEINT;
}
