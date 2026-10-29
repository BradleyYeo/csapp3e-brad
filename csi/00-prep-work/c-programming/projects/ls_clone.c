#include <dirent.h>
#include <grp.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

typedef struct {
  char name[256];
  off_t size;
  struct stat st;
} FileInfo;

int sort_by_size(const void *a, const void *b) {
  FileInfo *f1 = (FileInfo *)a;
  FileInfo *f2 = (FileInfo *)b;
  return f2->size - f1->size;
}

void print_long_format(char *name, struct stat *st) {
  printf((S_ISDIR(st->st_mode)) ? "d" : "-");
  printf((st->st_mode & S_IRUSR) ? "r" : "-");
  printf((st->st_mode & S_IWUSR) ? "w" : "-");
  printf((st->st_mode & S_IXUSR) ? "x" : "-");
  printf((st->st_mode & S_IRGRP) ? "r" : "-");
  printf((st->st_mode & S_IWGRP) ? "w" : "-");
  printf((st->st_mode & S_IXGRP) ? "x" : "-");
  printf((st->st_mode & S_IROTH) ? "r" : "-");
  printf((st->st_mode & S_IWOTH) ? "w" : "-");
  printf((st->st_mode & S_IXOTH) ? "x" : "-");

  // Number of hard links
  printf(" %ld", st->st_nlink);

  // Owner name
  printf(" %s", getpwuid(st->st_uid)->pw_name);

  // Group name
  printf(" %s", getgrgid(st->st_gid)->gr_name);

  // File size
  printf(" %ld", st->st_size);

  // Timestamp of last modification
  char mod_time[20];
  strftime(mod_time, 20, "%b %d %H:%M", localtime(&(st->st_mtime)));
  printf(" %s", mod_time);

  // File/folder name
  printf(" %s\n", name);
}

int main(int argc, char *argv[]) {
  DIR *dir;
  struct dirent *entry;
  int long_format = 0;
  int sort_size = 0;
  int show_all = 0;
  char *path = ".";

  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-l") == 0) {
      long_format = 1;
    } else if (strcmp(argv[i], "-S") == 0) {
      sort_size = 1;
    } else if (strcmp(argv[i], "-a") == 0) {
      show_all = 1;
    } else {
      path = argv[i];
    }
  }

  if ((dir = opendir(path)) == NULL) {
    perror("Cannot open directory");
    return 1;
  }

  FileInfo files[1024];
  int count = 0;
  while ((entry = readdir(dir)) != NULL) {
    if (!show_all && entry->d_name[0] == '.') {
      continue;
    }

    strcpy(files[count].name, entry->d_name);
    char full_path[512];
    snprintf(full_path, sizeof(full_path), "%s/%s", path, entry->d_name);
    stat(full_path, &(files[count].st));
    files[count].size = files[count].st.st_size;
    count++;
  }
  closedir(dir);

  if (sort_size) {
    qsort(files, count, sizeof(FileInfo), sort_by_size);
  }

  for (int i = 0; i < count; i++) {
    if (long_format) {
      print_long_format(files[i].name, &(files[i].st));
    } else {
      printf("%s\n", files[i].name);
    }
  }

  return 0;
}
