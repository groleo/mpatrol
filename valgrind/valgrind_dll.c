/* gcc -g -Wall -DPSAPI_VERSION=1 -c -o valgrind_dll.o valgrind_dll.c */
/* gcc -shared -Wl,-soname,valgrind_dll.dll -Wl,--out-implib,libvalgrind_dll.dll.a -o valgrind_dll.dll valgrind_dll.o -lpsapi -limagehlp */

#include <stdio.h>

#include <windows.h>
#include <winnt.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <imagehlp.h>

#ifdef __GNUC__
# define __UNUSED__ __attribute__ ((unused))
#else
# define __UNUSED__
#endif

LPVOID WINAPI VG_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
BOOL WINAPI VG_HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
void *VG_malloc(size_t size);
void VG_free(void *memblock);



typedef struct
{
  char *func_name_old;
  PROC  func_proc_old;
  PROC  func_proc_new;
} Vg_Hook_Overload;

/*
 * WARNING
 *
 * Mofidy the value of VG_HOOK_OVERLOAD_COUNT and
 * VG_HOOK_OVERLOAD_COUNT when adding other overloaded
 * functions in overloads_instance
 */
#define VG_HOOK_OVERLOAD_COUNT 2
#define VG_HOOK_OVERLOAD_COUNT_CRT 4

Vg_Hook_Overload overloads_instance[VG_HOOK_OVERLOAD_COUNT_CRT] =
  {
    {
      "HeapAlloc",
      NULL,
      (PROC)VG_HeapAlloc
    },
    {
      "HeapFree",
      NULL,
      (PROC)VG_HeapFree
    },
    {
      "malloc",
      NULL,
      (PROC)VG_malloc
    },
    {
      "free",
      NULL,
      (PROC)VG_free
    }
  };

typedef struct
{
  char            *filename;
  HMODULE          mod;
  Vg_Hook_Overload overloads[VG_HOOK_OVERLOAD_COUNT_CRT];
  char *crt_name;
} Vg_Hook;

typedef LPVOID (WINAPI *vgd_heap_alloc_t) (HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes);
typedef BOOL   (WINAPI *vgd_heap_free_t)  (HANDLE hHeap, DWORD dwFlags, LPVOID lpMem);
typedef void *(*vgd_malloc_t) (size_t size);
typedef void (*vgd_free_t)(void *memblock);

Vg_Hook vgh_instance;

LPVOID WINAPI VG_HeapAlloc(HANDLE hHeap, DWORD dwFlags, SIZE_T dwBytes)
{
  vgd_heap_alloc_t ha;
  LPVOID data;

  printf("HeapAlloc !!!\n");

  ha = (vgd_heap_alloc_t)vgh_instance.overloads[0].func_proc_old;
  data = ha(hHeap, dwFlags, dwBytes);

  return data;
}

BOOL WINAPI VG_HeapFree(HANDLE hHeap, DWORD dwFlags, LPVOID lpMem)
{
  vgd_heap_free_t hf;
  BOOL res;

  printf("HeapFree !!!\n");

  hf = (vgd_heap_free_t)vgh_instance.overloads[1].func_proc_old;
  res = hf(hHeap, dwFlags, lpMem);

  return res;  
}

void *VG_malloc(size_t size)
{
  vgd_malloc_t ma;
  void *data;

  printf("malloc !!!\n");

  ma = (vgd_malloc_t)vgh_instance.overloads[2].func_proc_old;
  data = ma(size);

  return data;
}

void VG_free(void *memblock)
{
  vgd_free_t f;

  printf("free !!!\n");

  f = (vgd_free_t)vgh_instance.overloads[3].func_proc_old;
  f(memblock);
}

static char *
_vgh_crt_name_get()
{
  HANDLE hf;
  HANDLE hmap;
  BYTE *base;
  IMAGE_DOS_HEADER *dos_headers;
  IMAGE_NT_HEADERS *nt_headers;
  IMAGE_IMPORT_DESCRIPTOR *import_desc;
  char *res = NULL;

  hf = CreateFile(vgh_instance.filename, GENERIC_READ, FILE_SHARE_READ,
                  NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
  if (hf == INVALID_HANDLE_VALUE)
    return NULL;

  hmap = CreateFileMapping(hf, NULL, PAGE_READONLY | SEC_IMAGE, 0, 0, NULL);
  if (!hmap)
    goto close_file;

  base = (BYTE *)MapViewOfFile(hmap, FILE_MAP_READ, 0, 0, 0);
  if (!base)
    goto unmap;

  dos_headers = (IMAGE_DOS_HEADER *)base;
  nt_headers = (IMAGE_NT_HEADERS *)((BYTE *)dos_headers + dos_headers->e_lfanew);
  import_desc = (IMAGE_IMPORT_DESCRIPTOR *)((BYTE *)dos_headers + nt_headers->OptionalHeader.DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

  while (import_desc->Characteristics)
    {
      if(IsBadReadPtr((BYTE *)dos_headers + import_desc->Name,1) == 0)
        {
          char *module_name;

          module_name = (char *)((BYTE *)dos_headers + import_desc->Name);
          printf("Imports from %s\r\n",(BYTE *)dos_headers + import_desc->Name);
          if (lstrcmpi("msvcrt.dll", module_name) == 0)
            {
              printf("msvcrt.dll !!\n");
              res = _strdup(module_name);
              break;
            }
          if (lstrcmpi("msvcr90.dll", module_name) == 0)
            {
              printf("msvcr90.dll !!\n");
              res = _strdup(module_name);
              break;
            }
          if (lstrcmpi("msvcr90d.dll", module_name) == 0)
            {
              printf("msvcr90d.dll !!\n");
              res = _strdup(module_name);
              break;
            }
          import_desc = (IMAGE_IMPORT_DESCRIPTOR *)((BYTE *)import_desc + sizeof(IMAGE_IMPORT_DESCRIPTOR));
        }
      else
        break;
    }

  UnmapViewOfFile(base);
  CloseHandle(hf);

  return res;

 unmap:
  UnmapViewOfFile(base);
 close_file:
  CloseHandle(hf);

  return NULL;
}

int
vgh_init()
{
  HANDLE handle;
  void *base;
  char *tmp;
  DWORD res;
  int length;

  handle = OpenFileMapping(PAGE_READWRITE, FALSE, "shared_size");
  if (!handle)
    return 0;

  base = MapViewOfFile(handle, FILE_MAP_READ, 0, 0, sizeof(int));
  if (!base)
    {
      CloseHandle(handle);
      return 0;
    }

  CopyMemory(&length, base, sizeof(int));
  UnmapViewOfFile(base);
  CloseHandle(handle);

  handle = OpenFileMapping(PAGE_READWRITE, FALSE, "shared_filename");
  if (!handle)
    return 0;

  base = MapViewOfFile(handle, FILE_MAP_READ, 0, 0, length);
  if (!base)
    {
      CloseHandle(handle);
      return 0;
    }

  tmp = malloc(length * sizeof(char));
  if (!tmp)
    {
      UnmapViewOfFile(base);
      CloseHandle(handle);
      return 0;
    }

  CopyMemory(tmp, base, length);
  UnmapViewOfFile(base);
  CloseHandle(handle);

  length = GetFullPathName(tmp, 1, (char *)&res, NULL);
  if (length == 0)
    {
      free(tmp);
      return 0;
    }

  vgh_instance.filename = malloc((length + 1) * sizeof(char));
  if (!vgh_instance.filename)
    {
      free(tmp);
      return 0;
    }
  length = GetFullPathName(tmp, length + 1, vgh_instance.filename, NULL);
  if (length == 0)
    {
      free(vgh_instance.filename);
      free(tmp);
      return 0;
    }

  printf(" ** filename : %s\n", vgh_instance.filename);

  memcpy(vgh_instance.overloads, overloads_instance, sizeof(vgh_instance.overloads));

  vgh_instance.crt_name = _vgh_crt_name_get();

  return 1;
}

void
vgh_shutdown(void)
{
  if (vgh_instance.filename)
    free(vgh_instance.filename);
}

void
_vgd_modules_hook_set(HMODULE module, const char *lib_name, PROC old_function_proc, PROC new_function_proc)
{
  PIMAGE_IMPORT_DESCRIPTOR iid;
  PIMAGE_THUNK_DATA thunk;
  ULONG size;

  iid = (PIMAGE_IMPORT_DESCRIPTOR)ImageDirectoryEntryToData(module, TRUE, IMAGE_DIRECTORY_ENTRY_IMPORT, &size);
  if (!iid)
    return;

  while (iid->Name)
    {
      PSTR module_name;

      module_name = (PSTR)((PBYTE) module + iid->Name);
      if (_stricmp(module_name, lib_name) == 0)
        break;
      iid++;
    }

  if (!iid->Name)
    return;

  thunk = (PIMAGE_THUNK_DATA)((PBYTE)module + iid->FirstThunk );
  while (thunk->u1.Function)
    {
      PROC *func;

      func = (PROC *)&thunk->u1.Function;
      if (*func == old_function_proc)
        {
          MEMORY_BASIC_INFORMATION mbi;
          DWORD dwOldProtect;

          VirtualQuery(func, &mbi, sizeof(MEMORY_BASIC_INFORMATION));

          if (!VirtualProtect(mbi.BaseAddress, mbi.RegionSize, PAGE_READWRITE, &mbi.Protect))
            return;

          *func = *new_function_proc;
          VirtualProtect(mbi.BaseAddress, mbi.RegionSize, mbi.Protect, &dwOldProtect);
          break;
        }
      thunk++;
    }
}

void
_vgh_modules_hook(const char *lib_name, int crt)
{
  HMODULE mods[1024];
  HMODULE lib_module;
  HMODULE hook_module = NULL;
  DWORD res;
  DWORD mods_nbr;
  unsigned int i;
  unsigned int start;
  unsigned int end;

  if (!crt)
    {
      start = 0;
      end = VG_HOOK_OVERLOAD_COUNT;
    }
  else
    {
      start = VG_HOOK_OVERLOAD_COUNT;
      end = VG_HOOK_OVERLOAD_COUNT_CRT;
    }

  lib_module = LoadLibrary(lib_name);

  for (i = start; i < end; i++)
    vgh_instance.overloads[i].func_proc_old = GetProcAddress(lib_module, vgh_instance.overloads[i].func_name_old);

  if (!EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &mods_nbr))
    return;

  for (i = 0; i < (mods_nbr / sizeof(HMODULE)); i++)
    {
      char name[256] = "";

      res = GetModuleFileNameEx(GetCurrentProcess(), mods[i], name, sizeof(name));
      if (!res)
        continue;

      if (lstrcmp(name, vgh_instance.filename) != 0)
        continue;

      hook_module = mods[i];
    }

  if (hook_module)
    {
      for (i = start; i < end; i++)
        _vgd_modules_hook_set(hook_module, lib_name,
                              vgh_instance.overloads[i].func_proc_old,
                              vgh_instance.overloads[i].func_proc_new);
    }

  FreeLibrary(lib_module);
}

void
_vgh_modules_unhook(const char *lib_name, int crt)
{
  HMODULE mods[1024];
  HMODULE hook_module = NULL;
  DWORD mods_nbr;
  DWORD res;
  unsigned int i;
  unsigned int start;
  unsigned int end;

  if (!crt)
    {
      start = 0;
      end = VG_HOOK_OVERLOAD_COUNT;
    }
  else
    {
      start = VG_HOOK_OVERLOAD_COUNT;
      end = VG_HOOK_OVERLOAD_COUNT_CRT;
    }

  if (!EnumProcessModules(GetCurrentProcess(), mods, sizeof(mods), &mods_nbr))
    return;

  for (i = 0; i < (mods_nbr / sizeof(HMODULE)); i++)
    {
      char name[256] = "";

      res = GetModuleFileNameEx(GetCurrentProcess(), mods[i], name, sizeof(name));
      if (!res)
        continue;

      if (lstrcmp(name, vgh_instance.filename) != 0)
        continue;

      hook_module = mods[i];
    }

  if (hook_module)
    {
      for (i = start; i < end; i++)
        _vgd_modules_hook_set(hook_module, lib_name,
                              vgh_instance.overloads[i].func_proc_new,
                              vgh_instance.overloads[i].func_proc_old);
    }
}

BOOL APIENTRY DllMain(HMODULE hModule __UNUSED__, DWORD ulReason, LPVOID lpReserved __UNUSED__)
{
  switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:
      printf (" # process attach\n");
      if (!vgh_init())
        return FALSE;

      break;
    case DLL_THREAD_ATTACH:
      printf (" # thread attach begin\n");
      _vgh_modules_hook("kernel32.dll", 0);
      if (vgh_instance.crt_name)
        _vgh_modules_hook(vgh_instance.crt_name, 1);
      printf (" # thread attach end\n");
      break;
    case DLL_THREAD_DETACH:
      printf (" # thread detach\n");
      break;
    case DLL_PROCESS_DETACH:
      printf (" # process detach\n");
      _vgh_modules_unhook("kernel32.dll", 0);
      if (vgh_instance.crt_name)
        _vgh_modules_unhook(vgh_instance.crt_name, 1);
      vgh_shutdown();
      break;
    }

    return TRUE;
}
