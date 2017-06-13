#include "header.h"

BlockFile::BlockFile(char* name, int b_length)
{
    file_name_ = new char[strlen(name) + 1];
    strcpy(file_name_, name);
    block_length_ = b_length;

    num_blocks_ = 0;

    if((fp_ = fopen(name, "rb+")) != 0)
    {
        new_flag_ = false;
        block_length_ = fread_number();
        num_blocks_ = fread_number();
    }
    else
    {
        if(block_length_ < BFHEAD_LENGTH)
        {
            error("BlockFile::BlockFile could not open file.\n", true);
        }

        fp_ = fopen(file_name_, "wb+");
        if(fp_ == NULL)
        {
            error("BlockFile::BlockFile could not create file.\n", true);
        }

        new_flag_ = true;
        fwrite_number(block_length_);
        fwrite_number(0);

        char* buffer = NULL;
        int len = -1;
        buffer = new char[(len = block_length_ - (int)ftell(fp_))];

        memset(buffer, 0, sizeof(buffer));
        put_bytes(buffer, len);

        delete[] buffer;
        buffer = NULL;
    }
    fseek(fp_, 0, SEEK_SET);
    act_block_ = 0;
}

BlockFile::~BlockFile()
{
    if(file_name_)
    {
        delete[] file_name_;
        file_name_ = NULL;
    }
    if(fp_)
        fclose(fp_);
}

void BlockFile::fwrite_number(int value)
{
    put_bytes((char*)&value, SIZEINT);
}

int BlockFile::fread_number()
{
    char ca[SIZEINT];
    get_bytes(ca, SIZEINT);

    return *((int*)ca);
}


void BlockFile::read_header(char* buffer)
{
    fseek(fp_, BFHEAD_LENGTH, SEEK_SET);

    get_bytes(buffer, block_length_ - BFHEAD_LENGTH);

    if(num_blocks_ < 1)
    {
        fseek(fp_, 0, SEEK_SET);
        act_block_ = 0;
    }
    else
    {
        act_block_ = 1;
    }
}

void BlockFile::set_header(char* header)
{
    fseek(fp_, BFHEAD_LENGTH, SEEK_SET);

    put_bytes(header, block_length_ - BFHEAD_LENGTH);

    if(num_blocks_ < 1)
    {
        fseek(fp_, 0, SEEK_SET);
        act_block_ = 0;
    }
    else
    {
        act_block_ = 1;
    }
}

bool BlockFile::read_block(Block block, int index)
{
    index++;

    if(index <= num_blocks_ && index > 0)
    {
        seek_block(index);
    }
    else
    {
        printf("BlockFile::read_block request the block %d which is illegal.", index-1);
        error("\n", true);
    }

    get_bytes(block, block_length_);
    if(index + 1 > num_blocks_)
    {
        fseek(fp_, 0, SEEK_SET);
        act_block_ = 0;
    }
    else
    {
        act_block_ = index + 1;
    }
    return true;
}

bool BlockFile::write_block(Block block, int index)
{
    index++;

    if(index <= num_blocks_ && index > 0)
    {
        seek_block(index);
    }
    else
    {
        printf("BlockFile::write_block request the block %d which is illegal.", index-1);
        error("\n", true);
    }

    put_bytes(block, block_length_);
    if(index + 1 > num_blocks_)
    {
        fseek(fp_, 0, SEEK_SET);
        act_block_ = 0;
    }
    else
    {
        act_block_ = index + 1;
    }
    return true;
}

int BlockFile::append_block(Block block)
{
    fseek(fp_, 0, SEEK_END);
    put_bytes(block, block_length_);
    num_blocks_++;

    fseek(fp_, SIZEINT, SEEK_SET);
    fwrite_number(num_blocks_);

    fseek(fp_, -block_length_, SEEK_END);
    return (act_block_ = num_blocks_ ) - 1;
}

bool BlockFile::delete_last_blocks(int num)
{
    if(num > num_blocks_)
    {
        return false;
    }

    num_blocks_ -= num;
    fseek(fp_, SIZEINT, SEEK_SET);
    fwrite_number(num_blocks_);

    fseek(fp_, 0, SEEK_SET);
    act_block_ = 0;
    return true;
}
