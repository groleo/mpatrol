/* gcc -g -Wall -ansi -pedantic -o valgrind.exe valgrind.c */

#include <stdio.h>
#include <string.h>

#include <windows.h>

#define CREATE_THREAD_ACCESS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_SUSPEND_RESUME | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ)

typedef HMODULE (*_load_library) (const char *);
typedef BOOL    (*_free_library) (HMODULE);

typedef struct _Vg Vg;

struct _Vg
{
  _load_library        ll;
  _free_library        fl;

  char                *lock_file;
  char                *dll_fullname;
  int                  dll_length;

  struct {
    HANDLE process1;
    HANDLE thread;
    HANDLE process2;
  } child;

  DWORD exit_code; /* actually the base address of the mapped DLL */
};

FARPROC
_vg_symbol_get (const char *module, const char *symbol)
{
  HMODULE  mod;
  FARPROC  proc;

  printf (" * loading library %s... ", module);
  mod = LoadLibrary(module);
  if (!mod)
    {
      printf("failed\n", module);
      return NULL;
    }
  printf ("done\n");

  printf (" * retrieving symbol %s... ", symbol);
  proc = GetProcAddress(mod, symbol);
  if (!proc)
    {
      printf("failed\n", symbol);
      goto free_library;
    }

  printf ("done\n");

  FreeLibrary(mod);

  return proc;

 free_library:
  FreeLibrary(mod);

  return NULL;
}

Vg *
vg_new()
{
  char    buf[MAX_PATH];
  Vg     *vg;
  HMODULE kernel32;
  DWORD   length;

  /* Check if CreateRemoteThread() is available. */
  /* MSDN suggests to check the availability of a */
  /* function instead of checking the Windows version. */

  kernel32 = LoadLibrary("kernel32.dll");
  if (!kernel32)
    {
      printf("no kernel32.dll found\n");
      return 0;
    }

  if (!GetProcAddress(kernel32, "CreateRemoteThread"))
    {
      printf("no CreateRemoteThread found\n");
      goto free_kernel32;
    }

  vg = (Vg *)calloc (1, sizeof (Vg));
  if (!vg)
    goto free_kernel32;

  vg->ll = (_load_library)_vg_symbol_get("kernel32.dll", "LoadLibraryA");
  if (!vg->ll)
    goto free_vg;

  vg->fl = (_free_library)_vg_symbol_get("kernel32.dll", "FreeLibrary");
  if (!vg->fl)
    goto free_vg;

  length = GetFullPathName("valgrind_dll.dll", MAX_PATH, buf, NULL);
  if (!length)
    {
      printf ("can't get full path name\n");
      goto free_vg;
    }

  vg->dll_fullname = malloc(sizeof(char) * (strlen(buf) + 1));
  if (!vg->dll_fullname)
    goto free_vg;
  memcpy(vg->dll_fullname, buf, strlen(buf) + 1);

  vg->dll_length = length + 1;

  FreeLibrary(kernel32);

  return vg;

 free_vg:
  free(vg);
 free_kernel32:
  FreeLibrary(kernel32);

  return 0;
}

void
vg_del(Vg *vg)
{
  if (!vg)
    return;

  if (vg->child.process2)
    CloseHandle(vg->child.process2);
  if (vg->child.thread)
    CloseHandle(vg->child.thread);
  if (vg->child.process1)
    CloseHandle(vg->child.process1);
  free(vg->dll_fullname);
  if (vg->lock_file)
    {
      DeleteFile(vg->lock_file);
      free(vg->lock_file);
    }
  free(vg);
}

int
vg_lock(Vg *vg, const char *filename)
{
  char user_profile[4096];
  char *fullfilename;
  char *tmp;
  HANDLE hf;
  DWORD res;
  size_t length;
  DWORD written;

  if (!filename || !*filename)
    return 0;

  length = GetFullPathName(filename, 1, (char *)&res, NULL);
  if (length == 0)
    return 0;
  fullfilename = malloc((length + 1) * sizeof(char));
  if (!fullfilename)
    return 0;
  length = GetFullPathName(filename, length + 1, fullfilename, NULL);
  if (length == 0)
    goto free_filename;

  tmp = fullfilename;
  while (!tmp)
    {
      if (*tmp == '/') *tmp = '\\';
      tmp++;
    }

  res = GetEnvironmentVariable("USERPROFILE", user_profile, sizeof(user_profile));
  if ((res == 0) || (res == 4096))
    goto free_filename;

  vg->lock_file = malloc((res + sizeof("\\.valgrind")) * sizeof(char));
  if (!vg->lock_file)
    goto free_filename;

  memcpy(vg->lock_file, user_profile, res);
  memcpy(vg->lock_file + res, "\\.valgrind", sizeof("\\.valgrind"));

  printf("lock file : %s\n", vg->lock_file);

  /* /\* Check if the lock file exists *\/ */
  /* hf = CreateFile(fullpath, GENERIC_READ, 0, NULL, OPEN_EXISTING, */
  /*                 FILE_ATTRIBUTE_READONLY | FILE_ATTRIBUTE_HIDDEN | FILE_FLAG_NO_BUFFERING, */
  /*                 NULL); */

  hf = CreateFile(vg->lock_file, GENERIC_WRITE, 0, NULL, CREATE_NEW,
                  FILE_ATTRIBUTE_NORMAL,
                  NULL);
  if ((hf == INVALID_HANDLE_VALUE) && (GetLastError() != ERROR_FILE_NOT_FOUND))
    {
      if (GetLastError() == ERROR_FILE_EXISTS)
        {
          printf("%s already exists. valgrind is already running or has been interrupted.\n", vg->lock_file);
          printf("Exiting...\n");
          goto free_filename;
        }
      if ((hf == INVALID_HANDLE_VALUE) && (GetLastError() != ERROR_FILE_NOT_FOUND))
        {
          printf("A non awaited error has raised (%ld).\n", GetLastError());
          printf("Exiting...\n");
          goto free_filename;
        }
    }

  if (!LockFile(hf, 0, 0, length, 0))
    {
      printf("can not lock file\n");
      printf("Exiting...\n");
      goto close_file;
    }
  printf("filename : %s (%d)\n", fullfilename, length);
  if (!WriteFile(hf, fullfilename, length, &written, NULL))
    {
      printf("can not write to file (%ld)\n", GetLastError());
      printf("Exiting...\n");
      goto unlock_file;
    }
  UnlockFile(hf, 0, 0, length, 0);
  CloseHandle(hf);
  free(fullfilename);

  return 1;

 unlock_file:
  UnlockFile(hf, 0, 0, length, 0);
 close_file:
  CloseHandle(hf);
 free_filename:
  free(fullfilename);

  return 0;
}

int
vg_dll_inject(Vg *vg, const char *prog)
{
  STARTUPINFO         si;
  PROCESS_INFORMATION pi;
  HANDLE              process;
  HANDLE              remote_thread;
  LPVOID              remote_string;
  DWORD               exit_code; /* actually the base address of the mapped DLL */

  ZeroMemory(&pi, sizeof(PROCESS_INFORMATION));
  ZeroMemory(&si, sizeof(STARTUPINFO));
  si.cb = sizeof(STARTUPINFO);

  printf (" * creating child process %s... ", prog); 
  if (!CreateProcess(NULL, (char *)prog, NULL, NULL, TRUE,
                     CREATE_SUSPENDED, NULL, NULL, &si, &pi))
    {
      printf ("failed\n * can't spawn child process %s\n", prog);
      return 0;
    }
  printf ("done\n");

  printf (" * waiting for the child process to initialize... ");
  if (!WaitForInputIdle(pi.hProcess, INFINITE))
    {
      printf("failed\n * no process for %s\n", prog);
      goto close_handles;
    }
  printf ("done\n");

  printf (" * opening child process... ");
  process = OpenProcess(CREATE_THREAD_ACCESS, FALSE, pi.dwProcessId);
  if (!process)
    {
      printf("failed\n * no process for %s\n", prog);
      goto close_handles;
    }
  printf ("done\n");

  printf (" * allocating virtual memory... ");
  remote_string = VirtualAllocEx(process, NULL, vg->dll_length, MEM_COMMIT, PAGE_READWRITE);
  if (!remote_string)
    {
      printf("failed\n");
      goto close_process;
    }
  printf ("done\n");

  printf(" * writing process in virtual memory... ");
  if (!WriteProcessMemory(process, remote_string, vg->dll_fullname, vg->dll_length, NULL))
    {
      printf("failed\n");
      goto virtual_free;
    }
  printf ("done\n");

  printf (" * execute thread... ");
  remote_thread = CreateRemoteThread(process, NULL, 0, (LPTHREAD_START_ROUTINE)vg->ll, remote_string, 0, NULL);
  if (!remote_thread)
    {
      printf("failed\n");
      goto virtual_free;
    }
  printf ("done\n");

  WaitForSingleObject(remote_thread, INFINITE);

  printf (" * getting exit code... ");
  if (!GetExitCodeThread(remote_thread, &exit_code))
    {
      printf("failed\n");
      goto close_thread;
    }
  printf ("done\n");

  CloseHandle(remote_thread);
  VirtualFreeEx(process, remote_string, 0, MEM_RELEASE);

  printf(" * resume child process\n");
  ResumeThread(pi.hThread);

  vg->child.process1 = pi.hProcess;
  vg->child.thread = pi.hThread;
  vg->child.process2 = process;
  vg->exit_code = exit_code;

  return 1;

 close_thread:
  CloseHandle(remote_thread);
 virtual_free:
  VirtualFreeEx(process, remote_string, 0, MEM_RELEASE);
 close_process:
  CloseHandle(process);
 close_handles:
  CloseHandle(pi.hThread);
  CloseHandle(pi.hProcess);

  return 0;
}

void
vg_dll_eject(Vg *vg)
{
  HANDLE thread;

  thread = CreateRemoteThread(vg->child.process2, NULL, 0,
                              (LPTHREAD_START_ROUTINE)vg->fl,
                              (void*)vg->exit_code, 0, NULL );
  WaitForSingleObject(thread, INFINITE );
  CloseHandle(thread );
}

int main(int argc, char *argv[])
{
  Vg *vg;

  if (argc < 2)
    {
      printf ("Usage: %s file\n\n", argv[0]);
      return -1;
    }

  vg = vg_new();
  if (!vg)
    return -1;

  if (!vg_lock(vg, argv[1]))
    {
      printf(" * impossible to create lock file\n * exiting...\n");
      goto del_vg;
    }

  if (!vg_dll_inject(vg, argv[1]))
    {
      printf(" * injection failed\n * exiting...\n");
      goto del_vg;
    }

  Sleep(2000);
  printf(" * fin process\n");

  vg_dll_eject(vg);

  vg_del(vg);
  printf(" * ressources freed\n");

  return 0;

 del_vg:
  vg_del(vg);

  return -1;
}
