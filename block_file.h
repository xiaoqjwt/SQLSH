#ifndef __BLOCK_FILE_H
#define __BLOCK_FILE_H

class BlockFile
{
public:
    FILE* fp_;
    char* file_name_;
    bool new_flag_;

    int block_length_;
    int act_block_;
    int num_blocks_;

    BlockFile(char* name, int b_length);
    ~BlockFile();

    void put_bytes(char* bytes, int num)
    {
        fwrite(bytes, num, 1, fp_);
    }

    void get_bytes(char* bytes, int num)
    {
        fread(bytes, num, 1, fp_);
    }

    void seek_block(int bnum)
    {
        fseek(fp_, (bnum - act_block_)*block_length_, SEEK_CUR);
    }

    bool file_new()
    {
        return new_flag_;
    }

    int get_blocklength()
    {
        return block_length_;
    }

    int get_num_of_blocks()
    {
        return num_blocks_;
    }

    void fwrite_number(int num);
    int fread_number();

    void read_header(char* header);
    void set_header(char* header);

    bool read_block(Block block, int index);
    bool write_block(Block block, int index);

    int append_block(Block block);

    bool delete_last_blocks(int num);
};

#endif // __BLOCK_FILE_H
