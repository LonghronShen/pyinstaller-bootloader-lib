#include <cstddef>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <stdexcept>
#include <string>
#include <vector>

#include <mio/mmap.hpp>

extern "C" {
#include <pyi_archive.h>
#include <pyi_launch.h>
#include <pyi_main.h>
#include <pyi_path.h>
#include <pyi_pythonlib.h>
#include <pyi_utils.h>
}

namespace pyinstaller {
namespace bootloader {

void inline fix_for_msvcrt_stderr() {
#ifdef _MSC_VER
  /* Visual C runtime incorrectly buffers stderr */
  setbuf(stderr, (char *)NULL);
#endif /* _MSC_VER */
}

void inline set_process_name() {
#if defined(__linux__)
  char *processname = NULL;

  /* Set process name on linux. The environment variable is set by
     parent launcher process. */
  processname = pyi_getenv("_PYI_PROCNAME");
  if (processname) {
    VS("LOADER: restoring linux process name from _PYI_PROCNAME: %s\n",
       processname);
    if (prctl(PR_SET_NAME, processname, 0, 0)) {
      FATALERROR("LOADER: failed to set linux process name!\n");
      return -1;
    }
    free(processname);
  }
  pyi_unsetenv("_PYI_PROCNAME");
#endif /* defined(__linux__) */
}

inline FILE *make_fp_from_buffer(void *buffer, std::size_t buffer_length) {
  return nullptr;
}

int run_from_buffer(void *buffer, std::size_t buffer_length, std::int32_t argc,
                    char *argv[], const char *extractionpath = nullptr) {
  /*  archive_status contain status information of the main process. */
  ARCHIVE_STATUS *archive_status = NULL;
  SPLASH_STATUS *splash_status = NULL;

  char executable[PATH_MAX];
  char homepath[PATH_MAX];
  char archivefile[PATH_MAX];

  int rc = 0;
  int in_child = 1;

  fix_for_msvcrt_stderr();

  VS("PyInstaller Bootloader 3.x\n");

  archive_status = pyi_arch_status_new();
  if (archive_status == NULL) {
    return -1;
  }

  if ((!pyi_path_executable(executable, argv[0])) ||
      (!pyi_path_archivefile(archivefile, executable)) ||
      (!pyi_path_homepath(homepath, executable))) {
    return -1;
  }

  archive_status->fp = make_fp_from_buffer(buffer, buffer_length);

  if ((!pyi_arch_setup(archive_status, executable)) &&
      (!pyi_arch_setup(archive_status, archivefile))) {
    FATALERROR("Cannot open self %s or archive %s\n", executable, archivefile);
    return -1;
  }

  set_process_name();

  /* These are used only in pyi_pylib_set_sys_argv, which converts to wchar_t */
  archive_status->argc = argc;
  archive_status->argv = argv;

  if (!extractionpath && !pyi_launch_need_to_extract_binaries(archive_status)) {
    VS("LOADER: No need to extract files to run; setting extractionpath to "
       "homepath\n");
    extractionpath = homepath;
  }

#if defined(_WIN32) || defined(__CYGWIN__)
  if (extractionpath) {
    /* Add extraction folder to DLL search path */
    wchar_t dllpath_w[PATH_MAX];
#if defined(__CYGWIN__)
    /* Cygwin */
    if (cygwin_conv_path(CCP_POSIX_TO_WIN_W | CCP_RELATIVE, extractionpath,
                         dllpath_w, PATH_MAX) != 0) {
      FATAL_PERROR("cygwin_conv_path", "Failed to convert DLL search path!\n");
      return -1;
    }
#else
    /* Windows */
    if (pyi_win32_utils_from_utf8(dllpath_w, extractionpath, PATH_MAX) ==
        NULL) {
      FATALERROR("Failed to convert DLL search path!\n");
      return -1;
    }
#endif /* defined(__CYGWIN__) */
    VS("LOADER: SetDllDirectory(%S)\n", dllpath_w);
    SetDllDirectoryW(dllpath_w);
  }
#endif /* defined(_WIN32) || defined(__CYGWIN__) */

  if (extractionpath) {
    VS("LOADER: Already in the child - running user's code.\n");

    /*  If binaries were extracted to temppath,
     *  we pass it through status variable
     */
    if (strcmp(homepath, extractionpath) != 0) {
      if (snprintf(archive_status->temppath, PATH_MAX, "%s", extractionpath) >=
          PATH_MAX) {
        VS("LOADER: temppath exceeds PATH_MAX\n");
        return -1;
      }
      /*
       * Temp path exits - set appropriate flag and change
       * status->mainpath to point to temppath.
       */
      archive_status->has_temp_directory = true;
      strcpy(archive_status->mainpath, archive_status->temppath);
    }

    /* On macOS in windowed mode, process Apple events and convert
     * them to sys.argv - but only if we are in onedir mode! */
#if defined(__APPLE__) && defined(WINDOWED)
    if (!in_child) {
      /* Initialize argc_pyi and argv_pyi with argc and argv */
      if (pyi_utils_initialize_args(archive_status->argc,
                                    archive_status->argv) < 0) {
        return -1;
      }
      /* Process Apple events; this updates argc_pyi/argv_pyi
       * accordingly */
      /* NOTE: processing Apple events swallows up the initial
       * OAPP event, which seems to cause segmentation faults
       * in tkinter-based frozen bundles made with Homebrew
       * python 3.9 and Tcl/Tk 8.6.11. Until the exact cause
       * is determined and addressed, this functionality must
       * remain disabled.
       */
      /*pyi_process_apple_events(true);*/ /* short_timeout */
      /* Update pointer to arguments */
      pyi_utils_get_args(&archive_status->argc, &archive_status->argv);
      /* TODO: do we need to de-register Apple event handlers before
       * entering python? */
    }
#endif

    /* Main code to initialize Python and run user's code. */
    pyi_launch_initialize(archive_status);
    rc = pyi_launch_execute(archive_status);
    pyi_launch_finalize(archive_status);

    /* Clean up splash screen resources; required when in single-process
     * execution mode, i.e. when using --onedir on Windows or macOS. */
    pyi_splash_finalize(splash_status);
    pyi_splash_status_free(&splash_status);

#if defined(__APPLE__) && defined(WINDOWED)
    /* Clean up arguments that were used with Apple event processing .*/
    pyi_utils_free_args();
#endif
  } else {

    /* status->temppath is created if necessary. */
    if (pyi_launch_extract_binaries(archive_status, splash_status)) {
      VS("LOADER: temppath is %s\n", archive_status->temppath);
      VS("LOADER: Error extracting binaries\n");
      return -1;
    }

    /* Run the 'child' process, then clean up. */

    VS("LOADER: Executing self as child\n");
    pyi_setenv("_MEIPASS2", archive_status->temppath[0] != 0
                                ? archive_status->temppath
                                : homepath);

    VS("LOADER: set _MEIPASS2 to %s\n", pyi_getenv("_MEIPASS2"));

#if defined(__linux__)
    char tmp_processname[16]; /* 16 bytes as per prctl() man page */

    /* Pass the process name to child via environment variable. */
    if (!prctl(PR_GET_NAME, tmp_processname, 0, 0)) {
      VS("LOADER: linux: storing process name into _PYI_PROCNAME: %s\n",
         tmp_processname);
      pyi_setenv("_PYI_PROCNAME", tmp_processname);
    }

#endif /* defined(__linux__) */

    if (pyi_utils_set_environment(archive_status) == -1) {
      return -1;
    }

    /* Transform parent to background process on OSX only. */
    pyi_parent_to_background();

    /* Run user's code in a subprocess and pass command line arguments to it. */
    rc = pyi_utils_create_child(executable, archive_status, argc, argv);

    VS("LOADER: Back to parent (RC: %d)\n", rc);

    VS("LOADER: Doing cleanup\n");

    /* Finalize splash screen before temp directory gets wiped, since the splash
     * screen might hold handles to shared libraries inside the temp dir. Those
     * wouldn't be removed, leaving the temp folder behind. */
    pyi_splash_finalize(splash_status);
    pyi_splash_status_free(&splash_status);

    if (archive_status->has_temp_directory == true) {
      pyi_remove_temp_path(archive_status->temppath);
    }
    pyi_arch_status_free(archive_status);
  }
  return rc;
}

inline char **convert_to_argv(const std::vector<std::string> &argv,
                              char ***pbuffer, int *argc) {
  if (pbuffer == nullptr) {
    return nullptr;
  }
  char **buffer = (char **)malloc(sizeof(char *) * argv.size());
  for (int i = 0; i < argv.size(); i++) {
    const auto &item = argv[i];
    const auto item_size = sizeof(char) * item.size() + 1;
    char *item_buffer = (char *)malloc(item_size);
    memset(item_buffer, 0, item_size);
    strcpy(item_buffer, item.c_str());
    buffer[i] = item_buffer;
  }
  *pbuffer = buffer;
  *argc = argv.size();
  return buffer;
}

inline void free_buffer(char ***pbuffer, int argc) {
  if (pbuffer == nullptr) {
    return;
  }
  char **buffer = *pbuffer;
  for (int i = 0; i < argc; i++) {
    free(buffer[i]);
  }
  free(buffer);
}

std::int32_t run_from_buffer(const std::vector<char> &buffer,
                             const std::vector<std::string> &argv,
                             const std::string &extract_path = "") {
  char **args_array;
  int argc;
  if (convert_to_argv(argv, &args_array, &argc)) {
    auto ret =
        run_from_buffer((void *)buffer.data(), buffer.size(), argc, args_array);
    free_buffer(&args_array, argc);
    return (std::int32_t)ret;
  }
  return -1;
}

} // namespace bootloader
} // namespace pyinstaller
