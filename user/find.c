#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"


char*
findname(char *path)
{
  char *p;

  // Find first character after last slash.
  for(p=path+strlen(path); p >= path && *p != '/'; p--)
    ;
  p++;

return p;
}
void dfs(char *path,char* specific)
{
    int fd;
    char buf[512],*p;
    struct dirent de;
    struct stat st;
  if((fd = open(path, 0)) < 0){
    fprintf(2, "find: cannot open %skkkk\n", path);
    return;
  }
    if(fstat(fd, &st) < 0){
    fprintf(2, "find: cannot stat %s\n", path);
    close(fd);
    return;
  }
   
    if(st.type == T_FILE)
    {
        if(strcmp(findname(path),specific)==0)
        {

            write(1,path,strlen(path));
            write(1,"\n",1);
            
        }
        
        close(fd);
        return ;
    }
    if(st.type==T_DIR)
    {
    if(strlen(path) + 1 + DIRSIZ + 1 > sizeof buf){
      fprintf(2,"find: path too long\n");
      return ;
    }
        strcpy(buf, path);
        p = buf+strlen(buf);
        *p++ = '/';
        read(fd,&de,sizeof(de));
        read(fd,&de,sizeof(de));
        while(read(fd,&de,sizeof(de))==sizeof(de))
        {
            if(de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            dfs(buf,specific);
        }
        return ;
    }
    
}
int main(int argc,char* argv[])
{
    if(argc==1||argc==2){
        fprintf(2,"error\n");
        exit(1);
    }
   dfs(argv[1],argv[2]);
    
    exit(0);
}