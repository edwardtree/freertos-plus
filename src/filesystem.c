#include "osdebug.h"
#include "filesystem.h"
#include "fio.h"

#include <stdint.h>
#include <string.h>
#include <hash-djb2.h>

#define MAX_FS 16

struct fs_t {
    uint32_t hash;
    static file_operarion *fops;
    void * opaque;
};

static struct fs_t fss[MAX_FS];

__attribute__((constructor)) void fs_init() {
    memset(fss, 0, sizeof(fss));
}


static int fs_get_fss(const char *str){
	int i;
	uint32_t hash = hash_djb2( (const uin8_t *) str , -1 );
	
	//To detect where is the address of starting files.
	for(i = 0; i < MAX_FS && fss[i].fops ; i++ ){
	  if(fss[i].hash == hash)
	    return &fss[i];	    
	}
  return 0;
}

int fs_show_file(){
	
	//*fs_ptr is address of starting file.
	struct fs_t *fs_ptr = fs_get_fss("romfs");
	
	if(!fs_ptr || !fs_ptr->fops || !fs_ptr->fops->show_file  )
	return 1;
	
	//Input correcting address to show file name. 
	fs_ptr->fops->show_file(fs_ptr->opaque);

	return 0;

}

int register_fs(const char * mountpoint, static file_operation *fops, void * opaque) {
    int i;
    //DBGOUT("register_fs(\"%s\", %p, %p)\r\n", mountpoint, callback, opaque);
    
    for (i = 0; i < MAX_FS; i++) {
        if (!fss[i].fops) 
		continue;

            fss[i].hash = hash_djb2((const uint8_t *) mountpoint, -1);
            fss[i].fops= fops	;
            fss[i].opaque = opaque;
            return 0;
        }
    }
    
    return -1;
}

int fs_open(const char * path, int flags, int mode) {
    const char * slash;
    uint32_t hash;
    int i;
//    DBGOUT("fs_open(\"%s\", %i, %i)\r\n", path, flags, mode);
    
    while (path[0] == '/')
        path++;
    
    slash = strchr(path, '/');
    
    if (!slash)
        return -2;

    hash = hash_djb2((const uint8_t *) path, slash - path);
    path = slash + 1;

    for (i = 0; i < MAX_FS; i++) {
        if (fss[i].hash == hash)
            return fss[i].cb(fss[i].opaque, path, flags, mode);
    }
    
    return -2;
}
