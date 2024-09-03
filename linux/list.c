#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <pwd.h>
#include <grp.h>
#include <time.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

int listDir(char *arg) {
	DIR *pdir;
	struct dirent *dirt;
	struct stat statBuf;
	struct passwd *username;
	struct group *groupname;
	struct tm *t;
	int i = 0, count = 0;
	char *dirName[255], buf[255], permission[11], mtime[20];

	memset(dirName, 0, sizeof(dirName));
	memset(&dirt, 0, sizeof(dirt));
	memset(&statBuf, 0, sizeof(statBuf));

	if ((pdir = opendir(arg)) <= 0) {
		perror("opendir");
		return -1;
	}

	chdir(arg);
	getcwd(buf, 255);
	printf("\n%s: Directory\n", arg);
	
	while ((dirt = readdir(pdir)) != NULL) {
		lstat(dirt->d_name, &statBuf);
				
		switch (statBuf.st_mode & __S_IFMT) {
			case __S_IFDIR: permission[0] = 'd';	break;
			case __S_IFLNK: permission[0] = 'l';	break;
			case __S_IFCHR: permission[0] = 'c';	break;
			case __S_IFBLK: permission[0] = 'b';	break;
			case __S_IFSOCK: permission[0] = 's';	break;
			case __S_IFIFO: permission[0] = 'P';	break;
			default:	permission[0] = '-';	break;
		}

		permission[1] = statBuf.st_mode & S_IRUSR ? 'r' : '-';
		permission[2] = statBuf.st_mode & S_IWUSR ? 'w' : '-';
		permission[3] = statBuf.st_mode & S_IXUSR ? 'x' : 
						statBuf.st_mode & S_ISUID ? 'S' : '-';
		
		permission[4] = statBuf.st_mode & S_IRGRP ? 'r' : '-';
		permission[5] = statBuf.st_mode & S_IWGRP ? 'w' : '-';
		permission[6] = statBuf.st_mode & S_IXGRP ? 'r' : 
						statBuf.st_mode & S_ISGID ? 'S' : '-';

		permission[7] = statBuf.st_mode & S_IROTH ? 'r' : '-';
		permission[8] = statBuf.st_mode & S_IWOTH ? 'w' : '-';
		permission[9] = statBuf.st_mode & S_IXOTH ? 'r' : 
						statBuf.st_mode & S_ISOTH ? 'S' : '-';
		
		if (statBuf.st_mode & S_IXOTH) {
			permission[9] = statBuf.st_mode & S_ISVTX
		}

	}
}

int main(int argc, char** argv) {

	return 0;
}