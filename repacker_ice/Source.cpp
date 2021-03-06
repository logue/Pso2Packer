
//--------------------------------------------------
// Include
//--------------------------------------------------
#include "clICE.h"
#include "stdafx.h"

//--------------------------------------------------
// Convert character strings to hexadecimal numbers and convert them to numbers
//--------------------------------------------------
int atoi16(const char *s) {
  int r = 0;
  bool sign = false;
  if (*s == '+') {
    s++;
  } else if (*s == '-') {
    sign = true;
    s++;
  }
  while (*s) {
    if (*s >= '0' && *s <= '9')
      r = (r << 4) | (*s - '0');
    else if (*s >= 'A' && *s <= 'F')
      r = (r << 4) | (*s - 'A' + 10);
    else if (*s >= 'a' && *s <= 'f')
      r = (r << 4) | (*s - 'a' + 10);
    else
      break;
    s++;
  }
  return sign ? -r : r;
}

//--------------------------------------------------
// Load file
//--------------------------------------------------
char *LoadFile(const char *filename, int *fileSize = nullptr) {

  // Check
  if (!filename)
    return nullptr;

  // Open
  FILE *fp;
  if (fopen_s(&fp, filename, "rb"))
    return nullptr;

  // Get size
  fseek(fp, 0, SEEK_END);
  int size = ftell(fp);

  // Reserve
  char *data = new char[size + 1];
  if (!data) {
    fclose(fp);
    return nullptr;
  }

  // Reading file
  fseek(fp, 0, SEEK_SET);
  fread(data, size, 1, fp);
  data[size] = '\0';

  // Close
  fclose(fp);

  // Setting size
  if (fileSize)
    *fileSize = size;

  return data;
}

//--------------------------------------------------
// Get Directory
//--------------------------------------------------
char *GetDirectory(char *directory, int size, const char *path) {
  if (size > 0)
    directory[0] = '\0';
  for (int i = strlen(path) - 1; i >= 0; i--) {
    if (path[i] == '\\' || path[i] == '/') {
      if (i > size - 1)
        i = size - 1;
      memcpy(directory, path, i);
      directory[i] = '\0';
      break;
    }
  }
  return directory;
}

//--------------------------------------------------
// Get filename
//--------------------------------------------------
char *GetFilename(char *filename, int size, const char *path) {
  if (size > 0)
    filename[0] = '\0';
  for (int i = strlen(path) - 1; i >= 0; i--) {
    if (path[i] == '\\' || path[i] == '/') {
      int len = strlen(path) - i - 1;
      if (len > size - 1)
        len = size - 1;
      memcpy(filename, &path[i + 1], len);
      filename[len] = '\0';
      break;
    }
  }
  return filename;
}

//--------------------------------------------------
// Create directory
//--------------------------------------------------
bool DirectoryCreate(const char *directory) {
  // Check
  if (!(directory && *directory))
    return false;

  // Relative path (directory name only)
  if (!(strchr(directory, '\\') || strchr(directory, '/'))) {
    return _mkdir(directory) == 0;
  }

  // Check directory name
  if (directory[1] != ':' || directory[2] != '\\')
    return false; // Check drive infomation
  if (!directory[3])
    return false; // Check other infomation.
  if (strpbrk(directory + 3, "/,:;*<|>\""))
    return false; // Check invalid charctor.
  if (strstr(directory, "\\\\"))
    return false; // Check double directory separator.
  if (strstr(directory, " \\"))
    return false; // Check directory separator after space.

  // Create directory
  if (_mkdir(directory)) {
    char current[0x400];
    if (!GetDirectory(current, _countof(current), directory))
      return false;
    if (!DirectoryCreate(current))
      return false;
    if (_mkdir(directory))
      return false;
  }

  return true;
}

//--------------------------------------------------
// Group List class
//--------------------------------------------------
class clGroupList {
public:
  clGroupList() : _ext(nullptr) {}
  ~clGroupList() { Release(); }
  bool LoadFile(const char *filename) {
    if (!(filename))
      return false;
    char *buf = ::LoadFile(filename);
    if (!buf)
      return false;
    for (char *s = buf, *n, *p; s && *s; s = n) {
      n = strchr(s, '\n');
      if (n)
        *(n++) = '\0';
      p = strchr(s, '\r');
      if (p)
        *p = '\0';
      p = strchr(s, '\t');
      if (p)
        *p = '\0';
      p = strchr(s, ' ');
      if (p)
        *p = '\0';
      p = strchr(s, ';');
      if (p)
        *p = '\0';
      p = strstr(s, "//");
      if (p)
        *p = '\0';
      if (*s == '\0')
        continue;
      stExt *e = new stExt;
      if (!e) {
        delete[] buf;
        return false;
      }
      memset(e, 0, sizeof(*e));
      e->len = strlen(s);
      e->name = new char[e->len + 1];
      if (!e->name) {
        delete e;
        delete[] buf;
        return false;
      }
      strcpy_s(e->name, e->len + 1, s);
      e->next = _ext;
      _ext = e;
    }
    delete[] buf;
    return true;
  }
  void Release(void) {
    for (stExt *e = _ext, *next; e; e = next) {
      next = e->next;
      if (e->name)
        delete[] e->name;
      delete e;
    }
    _ext = NULL;
  }
  unsigned int GetGroupNumber(const char *filename,
                              unsigned int defaultNumber = 2) {
    if (filename) {
      int len = strlen(filename);
      for (stExt *e = _ext; e; e = e->next) {
        if (len >= e->len && strstr(filename + len - e->len, e->name))
          return 1;
      }
    }
    return defaultNumber;
  }

private:
  struct stExt {
    stExt *next;
    char *name;
    int len;
  } * _ext;
};

//--------------------------------------------------
// Find file class
//--------------------------------------------------
class clFindFile {
public:
  clFindFile() : _fileFirst(nullptr), _fileLast(nullptr), _count(0) {}
  ~clFindFile() { Release(); }
  bool Find(const char *path) {

    // Check
    if (!(path && *path))
      return false;

    // Check directory
    DWORD attr = GetFileAttributesA(path);
    if (!(attr & FILE_ATTRIBUTE_DIRECTORY)) {
      AddFile(path);
      return true;
    }

    // Get termination character
    char s = path[strlen(path) - 1];
    bool last = (s == '\\' || s == '/');

    // Generate pattern
    char pattern[0x400];
    sprintf_s(pattern, _countof(pattern), "%s%s*", path, last ? "" : "\\");

    // Start Search
    WIN32_FIND_DATAA fd;
    HANDLE fh = FindFirstFileA(pattern, &fd);
    if (fh == INVALID_HANDLE_VALUE)
      return true;

    do {

      // If it's not a filename, go to next
      if (strcmp(fd.cFileName, ".") == 0 || strcmp(fd.cFileName, "..") == 0)
        continue;

      // Generate filename
      char filename[0x400];
      sprintf_s(filename, _countof(filename), "%s%s%s", path, last ? "" : "\\",
                fd.cFileName);

      // Directory
      if ((fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
        Find(filename);
      }

      // File
      else {
        AddFile(filename);
      }

    } while (FindNextFileA(fh, &fd));

    // Finish search
    FindClose(fh);

    return true;
  }
  void Release(void) {
    for (stFile *f = _fileFirst, *n; f; f = n) {
      n = f->next;
      delete f;
    }
    _fileFirst = _fileLast = nullptr;
    _count = 0;
  }
  unsigned int GetFileCount(void) { return _count; }
  const char *GetFilename(unsigned int i) {
    stFile *f = _fileFirst;
    for (; f && i; f = f->next, i--)
      ;
    return f ? f->filename : nullptr;
  }

private:
  struct stFile {
    stFile *next;
    char filename[0x400];
  } * _fileFirst, *_fileLast;
  unsigned int _count;
  stFile *AddFile(const char *filename) {
    stFile *f = new stFile;
    if (f) {
      memset(f, 0, sizeof(*f));
      strcpy_s(f->filename, _countof(f->filename), filename);
      if (_fileLast)
        _fileLast->next = f;
      else
        _fileFirst = f;
      _fileLast = f;
      _count++;
    }
    return f;
  }
};

//--------------------------------------------------
// Main
//--------------------------------------------------
int main(int argc, char *argv[]) {
  const char *usage = "Usage:\n\
  -o output file, output folder\n\
  -r replacing files\n\
  -m maintain CRC when replacing files\n\
  -e extract(default)\n\
  -f filelist output\n\
  -l output filelist(write-once type)\n\
  -a archive\n\
  -g force select group when archiving(default: 2)\n\
  -s Grouping list file by extension when archiving\n\
  -c Compression when creating ICE file\n\
  -k Specify the key for extraction and archive\n\n";

  printf("repacker_ice\nVersion: %d.%d.%d.%d\nBuild: %s %s\n\n", VERSION_MAJOR,
         VERSION_MINOR, VERSION_BUILD, VERSION_REVISION, __DATE__, __TIME__);

  // Command line analysis
  unsigned int count = 0;
  char *filenameOut = nullptr;
  char *filenameReplace = nullptr;
  bool mode_crc = false;
  bool mode_filelist = false;
  bool mode_filelist_add = false;
  bool mode_archive = false;
  bool mode_compress = false;
  unsigned int group = 2;
  char *filenameGroup = nullptr;
  unsigned int key = CRYPT_KEY;

  for (int i = 1; i < argc; i++) {
    if (argv[i][0] == '-' || argv[i][0] == '/') {
      switch (argv[i][1]) {
      case 'o':
        if (i + 1 < argc) {
          filenameOut = argv[++i];
        }
        break;
      case 'r':
        if (i + 1 < argc) {
          filenameReplace = argv[++i];
        }
        break;
      case 'm':
        mode_crc = true;
        break;
      case 'f':
        mode_filelist = true;
        mode_filelist_add = false;
        break;
      case 'l':
        mode_filelist = true;
        mode_filelist_add = true;
        break;
      case 'a':
        mode_archive = true;
        break;
      case 'c':
        mode_compress = true;
        break;
      case 'g':
        group = atoi(argv[++i]);
        group = (group < 1) ? 1 : (group > 2) ? 2 : group;
        break;
      case 's':
        if (i + 1 < argc) {
          filenameGroup = argv[++i];
        };
        break;
      case 'k':
        if (i + 1 < argc) {
          key = atoi16(argv[++i]);
        }
        break;
      }
    } else if (*argv[i]) {
      argv[++count] = argv[i];
    }
  }

  // Check input file
  if (!count) {
    printf(usage);
    return -1;
  }

  // Get contents of directory
  clFindFile ff;
  for (unsigned int i = 1; i <= count; i++)
    ff.Find(argv[i]);
  count = ff.GetFileCount();

  // Output filelist
  if (mode_filelist) {
    for (unsigned int i = 0; i < count; i++) {
      const char *filenameIn = ff.GetFilename(i);

      // After the second file, invalidate the output filename option.
      if (i && !mode_filelist_add)
        filenameOut = nullptr;

      // Default output filename
      char path[0x400];
      if (!(filenameOut && filenameOut[0])) {
        strcpy_s(path, _countof(path), argv[0]);
        char *d1 = strrchr(path, '\\');
        char *d2 = strrchr(path, '/');
        char *e = strrchr(path, '.');
        if (e && d1 < e && d2 < e)
          *e = '\0';
        strcat_s(path, _countof(path), LIST_FILENAME);
        filenameOut = path;
      }

      printf("Analyzing... %s \n", filenameIn);
      clICE ice(key);
      if (!ice.LoadFile(filenameIn)) {
        printf("Error: Failed to load file.\n");
        continue;
      }
      FILE *fp;
      if (fopen_s(&fp, filenameOut, mode_filelist_add ? "a+b" : "wb")) {
        printf("Error: Cannot open group text file.\n");
        continue;
      }
      {
        const char *d1 = strrchr(filenameIn, '\\');
        const char *d2 = strrchr(filenameIn, '/');
        const char *d = nullptr;
        if (d1 && d1 >= d2)
          d = d1 + 1;
        else if (d2 && d2 >= d1)
          d = d2 + 1;
        fprintf(fp, "%s ICEv%d\r\n", d ? d : "???", ice.GetVersion());
      }
      for (unsigned int i = 0, count = ice.GetFileCount(); i < count; i++) {
        const char *filename;
        ice.GetFile(i, &filename, NULL, NULL);
        if (i + 1 < count)
          fputs("|--", fp);
        else
          fputs("`--", fp);
        fputs(filename, fp);
        fputs("\r\n", fp);
      }
      fclose(fp);
    }
  }

  // Archive
  else if (mode_archive) {

    // Default output file
    char path[0x400];
    if (!(filenameOut && filenameOut[0])) {
      strcpy_s(path, _countof(path), argv[0]);
      char *d1 = strrchr(path, '\\');
      char *d2 = strrchr(path, '/');
      if (d1 && d1 >= d2)
        *d1 = '\0';
      else if (d2 && d2 >= d1)
        *d2 = '\0';
      char filename[0x400];
      GetFilename(filename, _countof(filename), path);
      strcat_s(path, _countof(path), filename);
      strcat_s(path, _countof(path), ".ice");
      filenameOut = path;
    }

    //
    clGroupList gl;
    if (filenameGroup && filenameGroup[0] && !gl.LoadFile(filenameGroup)) {
      printf("Error: Failed to load grouping list file.\n");
      return -1;
    }

    clICE ice(key);
    printf("Checking File...\n");
    for (unsigned int i = 0; i < count; i++) {
      const char *filenameIn = ff.GetFilename(i);
      char filename[0x400];
      GetFilename(filename, _countof(filename), filenameIn);
      printf("[%d] %s\n", i, filename);
      ice.Add(filename, filenameIn, gl.GetGroupNumber(filename, group) - 1);
    }
    if (!ice.SaveFile(filenameOut, mode_compress)) {
      printf("Error: Failed to save file.\n");
      return -1;
    }

  }

  // Replace
  else if (filenameReplace) {

    // Default output file
    char path[0x400];
    if (!(filenameOut && filenameOut[0])) {
      strcpy_s(path, _countof(path), argv[0]);
      char *d1 = strrchr(path, '\\');
      char *d2 = strrchr(path, '/');
      if (d1 && d1 >= d2)
        *d1 = '\0';
      else if (d2 && d2 >= d1)
        *d2 = '\0';
      char filename[0x400];
      GetFilename(filename, _countof(filename), path);
      strcat_s(path, _countof(path), filename);
      strcat_s(path, _countof(path), ".ice");
      filenameOut = path;
    }

    printf("replacing...\n");
    clICE ice(key);
    if (!ice.LoadFile(ff.GetFilename(0))) {
      printf("Error: Failed to load file.\n");
      return -1;
    }
    char filename[0x400];
    ice.Replace(GetFilename(filename, _countof(filename), filenameReplace),
                filenameReplace);
    if (!ice.SaveFile(filenameOut, mode_compress, mode_crc)) {
      printf("Error: Failed to save file.\n");
      return -1;
    }

  }

  // Extraction
  else {

    for (unsigned int i = 0; i < count; i++) {
      const char *filenameIn = ff.GetFilename(i);

      // After the second file, invalidate the output directory option.
      if (i)
        filenameOut = nullptr;

      // Default output directory
      char path[0x400];
      if (!(filenameOut && filenameOut[0])) {
        strcpy_s(path, _countof(path), argv[0]);
        char *d1 = strrchr(path, '\\');
        char *d2 = strrchr(path, '/');
        if (d1 && d1 >= d2)
          *d1 = '\0';
        else if (d2 && d2 >= d1)
          *d2 = '\0';
        char filename[0x400];
        strcpy_s(filename, _countof(filename), filenameIn);
        d1 = strrchr(filename, '\\');
        d2 = strrchr(filename, '/');
        char *d = filename;
        if (d1 && d1 >= d2) {
          d = d1;
          *(d++) = '\0';
        } else if (d2 && d2 >= d1) {
          d = d2;
          *(d++) = '\0';
        }
        char *e = strrchr(d, '.');
        if (e)
          *e = '\0';
        strcat_s(path, _countof(path), "\\");
        strcat_s(path, _countof(path), d);
        strcat_s(path, _countof(path),
                 "_ext"); // raven3 - Add "_ext" to folder name
        filenameOut = path;
      }

      // Extraction
      printf("Extracting... %s \n", filenameIn);
      clICE ice(key);
      if (!ice.LoadFile(filenameIn)) {
        printf("Error: Failed to load file.\n");
        continue;
      }
      DirectoryCreate(filenameOut);
      for (unsigned int i = 0, count = ice.GetFileCount(); i < count; i++) {
        const char *filename;
        unsigned char *data;
        unsigned int size;
        if (ice.GetFile(i, &filename, &data, &size)) {
          char path[0x400];
          strcpy_s(path, _countof(path), filenameOut);
          strcat_s(path, _countof(path), "\\");
          strcat_s(path, _countof(path), filename);
          FILE *fp;
          if (!fopen_s(&fp, path, "wb")) {
            fwrite(data, size, 1, fp);
            fclose(fp);
          }
        }
      }
    }
  }

  return 0;
}
