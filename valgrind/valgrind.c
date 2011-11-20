/* gcc -g -Wall -ansi -pedantic -o valgrind.exe valgrind.c */

#include <stdio.h>
#include <string.h>

#include <windows.h>

#define CREATE_THREAD_ACCESS (PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_SUSPEND_RESUME | PROCESS_VM_OPERATION | PROCESS_VM_WRITE | PROCESS_VM_READ)

typedef HMODULE (*_load_library) (const char *);
typedef BOOL    (*_free_library) (HMODULE);

struct Vg_Map
{
  HANDLE handle;
  void *base;
};

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

  struct Vg_Map map_size;
  struct Vg_Map map_file;

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
      printf("failed to load module %s\n", module);
      return NULL;
    }
  printf ("done\n");

  printf (" * retrieving symbol %s... ", symbol);
  proc = GetProcAddress(mod, symbol);
  if (!proc)
    {
      printf("failed to retrieve symbol %s\n", symbol);
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
vg_map_filename(Vg *vg, const char *filename)
{
  int length;

  if (!filename || !*filename)
    return 0;

  length = lstrlen(filename) + 1;

  vg->map_size.handle = CreateFileMapping(INVALID_HANDLE_VALUE,
                                          NULL, PAGE_READWRITE, 0, sizeof(int),
                                          "shared_size");
  if (!vg->map_size.handle)
    return 0;

  vg->map_size.base = MapViewOfFile(vg->map_size.handle, FILE_MAP_WRITE,
                                    0, 0, sizeof(int));
  if (!vg->map_size.base)
    goto close_size_mapping;

  CopyMemory(vg->map_size.base, &length, sizeof(int));

  vg->map_file.handle = CreateFileMapping(INVALID_HANDLE_VALUE,
                                          NULL, PAGE_READWRITE, 0, length,
                                          "shared_filename");
  if (!vg->map_file.handle)
    goto unmap_size_base;
  vg->map_file.base = MapViewOfFile(vg->map_file.handle, FILE_MAP_WRITE,
                                    0, 0, length);
  if (!vg->map_file.base)
    goto close_file_mapping;
  CopyMemory(vg->map_file.base, filename, length);

  return 1;

 close_file_mapping:
  CloseHandle(vg->map_file.handle);
 unmap_size_base:
  UnmapViewOfFile(vg->map_size.base);
 close_size_mapping:
  CloseHandle(vg->map_size.handle);

  return 0;
}

void
vg_unmap_filename(Vg *vg)
{
  UnmapViewOfFile(vg->map_file.base);
  CloseHandle(vg->map_file.handle);
  UnmapViewOfFile(vg->map_size.base);
  CloseHandle(vg->map_size.handle);
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

  if (!vg_map_filename(vg, argv[1]))
    {
      printf(" * impossible to map filename\n * exiting...\n");
      goto del_vg;
    }

  if (!vg_dll_inject(vg, argv[1]))
    {
      printf(" * injection failed\n * exiting...\n");
      goto unmap_vg;
    }

  Sleep(2000);
  printf(" * fin process\n");

  vg_dll_eject(vg);

  vg_unmap_filename(vg);
  vg_del(vg);
  printf(" * ressources freed\n");

  return 0;

 unmap_vg:
  vg_unmap_filename(vg);
 del_vg:
  vg_del(vg);

  return -1;
}
