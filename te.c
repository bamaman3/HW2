#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
/* function type that is called for each filename */
typedef int Myfunc(const char *, const struct stat *, int);
static Myfunc myfunc;
static int myftw(char *, Myfunc *);
static int dopath(Myfunc *, int, DIR *);
static long nreg, ndir, nblk, nchr, nfifo, nslink, nsock, ntot;
int s = 0, S = 0, f = 0, tf = 0, td = 0, size = 0, isdir = 1, ftype, i;
char *needle = "";

main(int argc, char *argv[])
{
    int ret;
    if (argc < 2){
        //err-quit
        printf("usage: ftw <starting-pathname>\n");
        exit(-1);
    }

    for(i = 2; i < argc; i++){
        if( strcmp(argv[i], "-s" ) == 0){
            if(argv[i+1] == NULL) {
                printf("The flag '-s' requires an integer \n");
                exit(-1);
            }            
            s = 1;
            size = atoi(argv[i+1]);

        }
        if( strcmp(argv[i], "-S" ) == 0) S = 1;
        if( strcmp(argv[i], "-f" ) == 0){
            if(argv[i+1] == NULL) {
                printf("The flag '-f' requires a string \n");
                exit(-1);
            }
             f = 1;
            needle = argv[i+1];
        }
        if( strcmp(argv[i], "-t") == 0){
            if(argv[i+1] == NULL){
                printf("The flag '-t' requires a character \n");
                exit(-1);
            }
            if(strcmp(argv[i+1], "f") == 0){
                tf++;
            }else if(strcmp(argv[i+1], "d") == 0){
                td++;
            }else printf("The parameter for the flag '-t' must be either 'f' or 'd'\n");
        }
    }
    ret = myftw(argv[1], myfunc); /* does it all */
    ntot = nreg + ndir + nblk + nchr + nfifo + nslink + nsock;
    if (ntot == 0)
        ntot = 1; /* avoid divide by 0; print 0 for all counts */
    exit(ret);
}
/*
* Descend through the hierarchy, starting at "pathname".
* The caller’s func() is called for every file.
*/
#define FTW_F 1 /* file other than directory */
#define FTW_D 2 /* directory */
#define FTW_DNR 3 /* directory that can’t be read */
#define FTW_NS 4 /* file that we can’t stat */
static char *fullpath; /* contains full pathname for every file */
static size_t pathlen;

static int /* we return whatever func() returns */
myftw(char *pathname, Myfunc *func)
{
    fullpath = malloc(pathlen); /* malloc PATH_MAX+1 bytes */ //currently malloc path (may need to add 1 byte later)
    /* (Figure 2.16) */
    if (pathlen <= strlen(pathname)) {
        pathlen = strlen(pathname) * 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            //err-sys
            printf("realloc failed\n");  
    }
    strcpy(fullpath, pathname);
    return(dopath(func, 0, NULL));
}
/*
* Descend through the hierarchy, starting at "fullpath".
* If "fullpath" is anything other than a directory, we lstat() it,
* call func(), and return. For a directory, we call ourself
* recursively for each name in the directory.
*/
static int /* we return whatever func() returns */
dopath(Myfunc* func, int tabs, DIR *prev)
{
    struct stat statbuf;
    struct stat sb;
    struct dirent *dirp;
    DIR *dp;
    int ret, n;
    if (lstat(fullpath, &statbuf) < 0) /* stat error */
        return(func(fullpath, &statbuf, FTW_NS));
    if (S_ISDIR(statbuf.st_mode) == 0){ /* not a directory */
        return(func(fullpath, &statbuf, FTW_F));
    }
    /*
    * It’s a directory. First call func() for the directory,
    * then process each filename in the directory.
    */
    if ((ret = func(fullpath, &statbuf, FTW_D)) != 0)
        return(ret);
    n=strlen(fullpath);
    if (n + NAME_MAX + 2 > pathlen) { /* expand path buffer */
        pathlen *= 2;
        if ((fullpath = realloc(fullpath, pathlen)) == NULL)
            //err-sys
            printf("realloc failed\n");
    }
    fullpath[n++] = '/';
    fullpath[n] = 0;
    if ((dp = opendir(fullpath)) == NULL) /* can’t read directory */
        return(func(fullpath, &statbuf, FTW_DNR));

    //if the path is in a new directory, add a tab to show nested directory
    if( (prev != NULL) && (prev != dp) ){
        tabs++;
    }

    while ((dirp = readdir(dp)) != NULL) {
        if (strcmp(dirp->d_name, ".") == 0 || strcmp(dirp->d_name, "..") == 0){
            continue; /* ignore dot and dot-dot */
        }else{
          


            char *filename = dirp->d_name;
            char *tpath = fullpath;
            strcpy(&tpath[n], filename);
            //check if size is correct for -s flag
            if (lstat(fullpath, &sb) < 0) printf("ERROR\n");
            int filesize = sb.st_size;

            if (S_ISDIR(sb.st_mode) == 0) isdir = 0;
            if(td == 1){
                ftype = 1;
            }else ftype = 0;

            //ensure that file size is acceptable & substring is present if -s or -f flags exist, ignore otherwise
            if( (filesize >= size) && (strstr(filename, needle) != NULL) ){
            //tabs to show nested files
            for(i = 0; i < tabs; i++){
                printf("\t");
            }         

                printf("%s", filename);
                //print the file size if -S flag is present
                if(S == 1) printf("(%d)", filesize);
                printf("\n");
            }

        }

        strcpy(&fullpath[n], dirp->d_name); /* append name after "/" */
        if ((ret = dopath(func, tabs, dp)) != 0) /* recursive */
            break; /* time to leave */
    }
    fullpath[n-1] = 0; /* erase everything from slash onward */
    if (closedir(dp) < 0)
        //err-ret
        printf("can't close directory %s\n", fullpath);
    return(ret);
}

/*
* for testing - replacing all err_XXXX(string) with printf(string) and exit(-1)
*/
static int
myfunc(const char *pathname, const struct stat *statptr, int type)
{
    switch (type) {
        case FTW_F:
            switch (statptr->st_mode & S_IFMT) {
                case S_IFREG: nreg++; break;
                case S_IFBLK: nblk++; break;
                case S_IFCHR: nchr++; break;
                case S_IFIFO: nfifo++; break;
                case S_IFLNK: nslink++; break;
                case S_IFSOCK: nsock++; break;
                case S_IFDIR: /* directories should have type = FTW_D */
                    //err-dump
                    printf("ERROR:\t\tfor S_IFDIR for %s\n", pathname);
            }       
            break;
        case FTW_D:
        ndir++;
            break;
        case FTW_DNR:
            //err-ret
            printf("ERRPR:\t\tcan't read directory %s\n", pathname);
            break;
        case FTW_NS:
            //err-ret
            printf("ERROR:\t\tstat error for %s\n", pathname);
            break;
        default:
            //err-dump
            printf("ERROR:\t\tunknown type %d for pathname %s\n", type, pathname);
    }
    return(0);
}