#include "fs.h"
#include <cmath>
#include <string.h>

int INE5412_FS::search_free_block(){
	for(int i=0; i < disk->size(); i++){
		if(!bit_map[i]) return i;
	}

	return 0;
}

int INE5412_FS::inode_read_block( int index, union fs_block *block, class fs_inode *inode){
		
		// Direto
		if(index < POINTERS_PER_INODE){
			if(inode->direct[index]){
				disk->read(inode->direct[index], block->data);
			} else 
				return 0;
			

		}
		// Indireto
		else{
			union fs_block block1;
			if(inode->indirect){
				disk->read(inode->indirect, block1.data);
				if(block1.pointers[index-POINTERS_PER_INODE]){
					disk->read(block1.pointers[index-POINTERS_PER_INODE], block->data);
				} else
					return 0;
				
			} else 
				return 0;

		}

		return 1;
}

int INE5412_FS::inode_write_block( int index, union fs_block *block, class fs_inode *inode){
		
		if(index >= POINTERS_PER_INODE+POINTERS_PER_BLOCK) return 0;

		// Direto
		if(index < POINTERS_PER_INODE){
			if(inode->direct[index]){
				disk->write(inode->direct[index], block->data);
			} else {
				inode->direct[index] = search_free_block();
				if(inode->direct[index]){
					bit_map[inode->direct[index]] = 1;
					disk->write(inode->direct[index], block->data);
				}
				else 
					return 0;
				
			}
		}
		// Indireto
		else{
			cout << "entao tão" << endl;
			union fs_block block1;
			if(inode->indirect){
				disk->read(inode->indirect, block1.data);
			} else {
				inode->indirect = search_free_block();
				if(inode->indirect){
					disk->read(inode->indirect, block1.data);
				}else 
					return 0;

			}

			if(block1.pointers[index-POINTERS_PER_INODE]){
				disk->write(block1.pointers[index-POINTERS_PER_INODE], block->data);
			} else {
				block1.pointers[index-POINTERS_PER_INODE] = search_free_block();
				if(block1.pointers[index-POINTERS_PER_INODE]){
					disk->write(block1.pointers[index-POINTERS_PER_INODE], block->data);
				}
				else 
					return 0;
			}
			
		}

		return 1;
}

void INE5412_FS::inode_load( int inumber, class fs_inode *inode ) {
		int block_number = inumber/INODES_PER_BLOCK + 1;
		int index = inumber%INODES_PER_BLOCK;

		union fs_block block;	
		disk->read(block_number, block.data);


		inode->isvalid = block.inode[index].isvalid;
   		inode->size = block.inode[index].size;

		for (int i = 0; i < POINTERS_PER_INODE; i++) {
        	inode->direct[i] = block.inode[index].direct[i];
    	}

    	inode->indirect = block.inode[index].indirect;
}

void INE5412_FS::inode_save( int inumber, class fs_inode *inode ) {
		int block_number = inumber/INODES_PER_BLOCK + 1;
		int index = inumber%INODES_PER_BLOCK;

		union fs_block block;	
		disk->read(block_number, block.data);

		block.inode[index].isvalid = inode->isvalid;
   		block.inode[index].size = inode->size;

		for (int i = 0; i < POINTERS_PER_INODE; i++) {
        	 block.inode[index].direct[i] = inode->direct[i];
    	}

    	block.inode[index].indirect = inode->indirect;

		disk->write(block_number, block.data);
}

int INE5412_FS::fs_format()
{	
	// Se não estiver montado
	if(!it_is_mounted){
		int disk_size = disk->size();
		int ninodeblocks = ceil(disk_size*0.1);

		// Cria superblock
		union fs_block block;
		block.super.magic = FS_MAGIC;
		block.super.nblocks = disk_size;
		block.super.ninodeblocks = ninodeblocks;
		block.super.ninodes = ninodeblocks*INODES_PER_BLOCK;
		disk->write(0, block.data);


		// Cria tabela de inodes
		for (int i = 0; i < INODES_PER_BLOCK; i++){
			block.inode[i].isvalid = 0;
		}

		for (int i = 1; i <= ninodeblocks; i++){
			disk->write(i, block.data);
		}

		return 1;
	} else 
		return 0;
	
	
	
}

void INE5412_FS::fs_debug()
{
	union fs_block block;
	disk->read(0, block.data);

	unsigned int magic = block.super.magic;
	int nblocks = block.super.nblocks;
    int ninodeblocks = block.super.ninodeblocks;
    int ninodes = block.super.ninodes;

	cout << "superblock:\n";
	cout << "    " << (magic == FS_MAGIC ? "magic number is valid\n" : "magic number is invalid!\n");
 	cout << "    " << nblocks << " blocks\n";
	cout << "    " << ninodeblocks << " inode blocks\n";
	cout << "    " << ninodes << " inodes\n";


	if (magic == FS_MAGIC){
		// Percorre pelos blocos de inodes.
		for (int i = 1; i <= ninodeblocks; i++){
			disk->read(i, block.data);
			// Percorre pelos inodes do bloco.
			for (int j = 0; j < INODES_PER_BLOCK; j++){
				fs_inode inode = block.inode[j];
				if(inode.isvalid){
					cout << "inode " << (j + (i-1)*INODES_PER_BLOCK) << ":\n";
					cout << "    "  << "size: " << inode.size << " bytes\n"; 

					cout << "    "  << "direct blocks:";
					for (int b : inode.direct) 
						if(b) cout << " " << b;	

					cout << endl;
					if(inode.indirect){
						cout << "    "  << "indirect block: " << inode.indirect << "\n"; 
						union fs_block tempBlock;
						disk->read(inode.indirect, tempBlock.data);
						cout << "    "  << "indirect data tempBlocks:";
						for (int b : tempBlock.pointers) 
							if(b) cout << " " << b;	
						
						cout << endl;
					} 

					
				}
			}
		}
	}


	//for(int i = 0; i < super.nblocks; i++){
	//	cout << i << ":" << bit_map[i] << endl;
	//}
}

int INE5412_FS::fs_mount()
{	
	union fs_block block;
	disk->read(0, block.data);
	if(block.super.magic == FS_MAGIC){
		
		if(disk->size() == block.super.nblocks){
			super = block.super;
			// Inicia bitmap com todas os blocos disponiveis.
			for(int i = 0; i < super.nblocks; i++){
				bit_map[i] = 0;
			}

			bit_map[0] = 1;

			// Percorre pelos blocos de inodes.
			for (int i = 1; i <= super.ninodeblocks; i++){
				bit_map[i] = 1;
				disk->read(i, block.data);
				// Percorre pelos inodes do bloco.
				for (int j = 0; j < INODES_PER_BLOCK; j++){
					fs_inode inode = block.inode[j];
					if(inode.isvalid){
						for (int b : inode.direct) 
							if(b) bit_map[b] = 1;
						
						if(inode.indirect){
							bit_map[inode.indirect] = 1;
							union fs_block tempBlock;
							disk->read(inode.indirect, tempBlock.data);
							for (int b : tempBlock.pointers) 
								if(b) bit_map[b] = 1;
						} 
					}
				}
			}

			it_is_mounted = 1;
			return 1;
		}
	}
	return 0;
}

int INE5412_FS::fs_create()
{
	if(it_is_mounted){
		union fs_block block;
		// Percorre pelos blocos de inodes.
		for (int i = 1; i <= super.ninodeblocks; i++){
			disk->read(i, block.data);
			// Percorre pelos inodes do bloco.
			for (int j = 0; j < INODES_PER_BLOCK; j++){
				int inode = j + (i-1)*INODES_PER_BLOCK;
				if( inode != 0 && !block.inode[j].isvalid){
					block.inode[j].isvalid = 1;
					block.inode[j].size = 0;
					block.inode[j].indirect = 0;

					for (int k = 0; k < POINTERS_PER_INODE; k++) {
						block.inode[j].direct[k] = 0;
					}

					disk->write(i, block.data);
					return (inode);
				}
			}
		}
	}
	return 0;
}

int INE5412_FS::fs_delete(int inumber)
{
	if(it_is_mounted  && inumber > 0 && inumber <= super.ninodes){
		fs_inode inode;
		inode_load(inumber, &inode);

		if(inode.isvalid){
			inode.isvalid = 0;
			inode_save(inumber, &inode);

			for (int b : inode.direct) 
				if(b) bit_map[b] = 0;;	
			
			if(inode.indirect){ 
				union fs_block block;	
				bit_map[inode.indirect] = 0;
				disk->read(inode.indirect, block.data);
				for (int b : block.pointers) 
					if(b) bit_map[b] = 0;
			} 

			return 1;
		}
	}

	return 0;
}

int INE5412_FS::fs_getsize(int inumber)
{

	if(it_is_mounted  && inumber > 0 && inumber <= super.ninodes){
		class fs_inode inode;
		inode_load(inumber, &inode);

		if(inode.isvalid)
			return inode.size;	
	}

	return -1;
}

int INE5412_FS::fs_read(int inumber, char *data, int length, int offset)
{
	if(it_is_mounted  && inumber > 0 && inumber <= super.ninodes){
		class fs_inode inode;
		inode_load(inumber, &inode);

		if(inode.isvalid && offset < inode.size){

			int inode_nblocks = ceil(float(inode.size)/Disk::DISK_BLOCK_SIZE);

			int index_block = offset/Disk::DISK_BLOCK_SIZE;
			int read_blocks = ceil(float(length)/Disk::DISK_BLOCK_SIZE) + index_block;
			read_blocks = (read_blocks < inode_nblocks ? read_blocks : inode_nblocks);
			int current_length = 0;

			union fs_block block;
			while(index_block < read_blocks && current_length < length)
			{
				if(inode_read_block(index_block, &block, &inode)){
					int read_size = (index_block != read_blocks-1 ? Disk::DISK_BLOCK_SIZE : inode.size - (read_blocks-1)*Disk::DISK_BLOCK_SIZE);
					read_size = (read_size <= length-current_length ? read_size : length-current_length);

					memcpy(data + current_length, block.data, read_size);

					current_length += read_size;

					index_block++;

				} else 
					return 0;

			}


			return current_length;
		}
	}

	return 0;
}

int INE5412_FS::fs_write(int inumber, const char *data, int length, int offset)
{	
	if(it_is_mounted  && inumber > 0 && inumber <= super.ninodes){
		class fs_inode inode;
		inode_load(inumber, &inode);

		if(!inode.isvalid || inode.size < offset) return 0;

		
		int index_block = offset/Disk::DISK_BLOCK_SIZE;
		int current_length = 0;

		union fs_block block;
		while(current_length < length){
			int write_size = (Disk::DISK_BLOCK_SIZE < length-current_length ? Disk::DISK_BLOCK_SIZE : length-current_length);

			if(index_block == offset/Disk::DISK_BLOCK_SIZE){
				inode_read_block(index_block, &block, &inode);
				memcpy(block.data+inode.size%Disk::DISK_BLOCK_SIZE, data+current_length, write_size - inode.size%Disk::DISK_BLOCK_SIZE);
				if(!inode_write_block(index_block, &block, &inode)) break;
				
				current_length += write_size - inode.size%Disk::DISK_BLOCK_SIZE;

			}else{

				memcpy(block.data, data+current_length, write_size);
				if(!inode_write_block(index_block, &block, &inode)) break;
				current_length += write_size;
			}

			index_block += 1;
		}

		inode.size += current_length;
		inode_save(inumber, &inode);
		return current_length;
	}

	return 0;
}
