

#include "engine/Game.impl.h"

#include "game/GameScene.h"

typedef void (*dirCB)(const char *p, void *ud);
void eachFileIn(const char *sDir, dirCB cb, void *ud) {
  char sPath[2048];

#ifdef CLANGD_ANALYSIS
  (void)sDir;
  (void)cb;
  (void)ud;
  (void)sPath;
#elif _WIN32
  WIN32_FIND_DATA fdFile;
  HANDLE hFind = NULL;

  sprintf(sPath, "%s\\*.*", sDir);

  if ((hFind = FindFirstFile(sPath, &fdFile)) == INVALID_HANDLE_VALUE)
    return;

  do {
    if (strcmp(fdFile.cFileName, ".") == 0 || strcmp(fdFile.cFileName, "..") == 0 ||
        (fdFile.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY))
      continue;

    // Build up our file path using the passed in
    //   [sDir] and the file/foldername we just found:
    sprintf(sPath, "%s/%s", sDir, fdFile.cFileName);
    cb(sPath, ud);
  } while (FindNextFile(hFind, &fdFile)); // Find the next file.

  FindClose(hFind); // Always, Always, clean things up!
#else
#ifndef DT_REG
#define DT_REG 8
#endif

  DIR *dir;
  struct dirent *ent;
  if ((dir = opendir(sDir)) != NULL) {
    /* print all the files and directories within directory */
    while ((ent = readdir(dir)) != NULL) {
      if (ent->d_type != DT_REG)
        continue;
      sprintf(sPath, "%s/%s", sDir, ent->d_name);
      cb(sPath, ud);
    }
    closedir(dir);
  } else {
  }
#endif
}

bool str_ends_with(const char *str, const char *suffix) {
  if (!str || !suffix)
    return 0;
  size_t lenstr = strlen(str);
  size_t lensuffix = strlen(suffix);
  if (lensuffix > lenstr)
    return 0;
  return strncmp(str + lenstr - lensuffix, suffix, lensuffix) == 0;
}

void export_svg__with_inkscape(const char *fp, void *ud) {
  char call[2048];

  if (!ud || !str_ends_with(fp, ".svg"))
    return;

  sprintf(call, "%s --export-type=png -d 192 %s", (const char *)ud, fp);
  printf("%s\n", call);
  system(call);
}

int main(int argc, char *argv[]) {
  gc_start(&gc, &argc);

  for (int i = 0; i < argc - 1; ++i)
    if (strcmp(argv[i], "--export-svg") == 0) {
      eachFileIn("assets", export_svg__with_inkscape, argv[i + 1]);
      return 0;
    }

  int result = g_main(GameScene_create());

  gc_stop(&gc);

  return result;
}