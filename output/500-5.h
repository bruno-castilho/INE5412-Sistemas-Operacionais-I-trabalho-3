#ifndef FS_H
#define FS_H

#include "disk.h"

class INE5412_FS
{
public:
    static const unsigned int FS_MAGIC = 0xf0f03410;
    static const unsigned short int INODES_PER_BLOCK = 128;
    static const unsigned short int POINTERS_PER_INODE = 5;
    static const unsigned short int POINTERS_PER_BLOCK = 1024;

    class fs_superblock {
        public:
            unsigned int magic;
            int nblocks;
            int ninodeblocks;
            int ninodes;
    }; 

    class fs_inode {
        public:
            int isvalid;
            int size;
            int direct[POINTERS_PER_INODE];
            int indirect;
    };

    union fs_block {
        public:
            fs_superblock super;
            fs_inode inode[INODES_PER_BLOCK];
            int pointers[POINTERS_PER_BLOCK];
            char data[Disk::DISK_BLOCK_SIZE];
    };

public:

    INE5412_FS(Disk *d) {
        disk = d;
        it_is_mounted = 0;
        bit_map= new int[disk->size()];
    } 

    ~INE5412_FS() {
        delete bit_map;
    } 

    void fs_debug();
    int  fs_format();
    int  fs_mount();

    int  fs_create();
    int  fs_delete(int inumber);
    int  fs_getsize(int inumber);

    int  fs_read(int inumber, char *data, int length, int offset);
    int  fs_write(int inumber, const char *data, int length, int offset);

private:
    Disk *disk;
    fs_superblock super;
    int it_is_mounted; 
    int* bit_map;

    void inode_load( int inumber, class fs_inode *inode ); // Lê um inode do disco
    void inode_save( int inumber, class fs_inode *inode ); // Ecreve um inode no disco
    int inode_read_block( int index, union fs_block *block, class fs_inode *inode); // Lê um bloco de dados de um inode, conforme a posição(index)
    int inode_write_block( int index, union fs_block *block, class fs_inode *inode); // Escreve um bloco de dados em um inode, conforme a posição(index)
    int search_free_block(); // Busca um bloco livre 
    
};

#endif