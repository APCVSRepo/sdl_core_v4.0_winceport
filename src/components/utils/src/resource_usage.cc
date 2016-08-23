#include "utils/resource_usage.h"
#if defined(__QNXNTO__)
#include <dirent.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/neutrino.h>
#include <sys/procfs.h>
#include <sys/stat.h>
#include <sys/trace.h>
#include <sys/types.h>
#include <unistd.h>
#endif
#if defined(OS_WIN32) || defined(OS_WINCE)
#include <process.h>
#include "windows.h"  
#include "tlhelp32.h"
#include "psapi.h"
#else
#include <sys/resource.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <sstream>
#endif
#include "utils/file_system.h"

namespace utils {

CREATE_LOGGERPTR_GLOBAL(logger_, "Utils")

#if defined(OS_WIN32)
const char* Resources::proc = "C:\\Windows";
#elif defined(OS_WINCE)
const char* Resources::proc = "\\Windows";
#else
const char* Resources::proc = "/proc/";
#endif

ResourseUsage* Resources::getCurrentResourseUsage() {
  PidStats pid_stats;
  if (false == GetProcInfo(pid_stats)) {
    LOG4CXX_ERROR(logger_, "Failed to get cpu proc info");
    return NULL;
  }
  MemInfo mem_info;
  if (false == GetMemInfo(mem_info)) {
    LOG4CXX_ERROR(logger_, "Failed to get memory info");
    return NULL;
  }
  ResourseUsage* usage = new ResourseUsage();
  usage->utime = pid_stats.utime;
  usage->stime = pid_stats.stime;
  usage->memory = static_cast<long long int>(mem_info);
  return usage;
}

bool Resources::ReadStatFile(std::string& output) {
#if !(defined(OS_WIN32) || defined(OS_WINCE))
  std::string filename = GetStatPath();
  if (false == file_system::FileExists(filename)) {
    return false;
  }
  if (false == file_system::ReadFile(filename,output)) {
    return false;
  }
#endif
  return true;
}

bool Resources::GetProcInfo(Resources::PidStats& output) {
#if defined(OS_WIN32)||defined(OS_WINCE)
	DWORD processId=GetProcessId(GetCurrentProcess());
	PROCESSENTRY32 pe32;
	pe32.dwSize = sizeof(pe32); 
	pe32.th32ProcessID=processId;
	HANDLE hProcessSnap = ::CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS,processId);  
	if (hProcessSnap == INVALID_HANDLE_VALUE)  {  
		printf("CreateToolhelp32Snapshot Failed.\n");  
		return false;  
	}  
	bool isGet=false;
	
	BOOL bMore = ::Process32First(hProcessSnap,&pe32);  
	while (bMore)  {  
		if(pe32.th32ProcessID==processId) {
			output.pid=processId;
			output.ppid=pe32.th32ParentProcessID;
			output.state=pe32.dwFlags;
			output.num_threads=pe32.cntThreads;
			output.priority=pe32.pcPriClassBase;
			//ProcessIdToSessionId(processId,&output.session);
			isGet=true;
			break;
		}
		bMore = ::Process32Next(hProcessSnap,&pe32);  
	}  
	//不要忘记清除掉snapshot对象  
	::CloseHandle(hProcessSnap); 
	return isGet;
#elif defined(OS_LINUX)
  std::string proc_buf;
  if (false == ReadStatFile(proc_buf)) {
    return false;
  }
  uint32_t num_succes = sscanf(proc_buf.c_str(),
         "%d" //pid
         " %*s"//com
         " %c" //state
         " %d" //ppid
         " %d" //pgrp
         " %d" //session
         " %d" //tty_nr
         " %d" //tpgid
         " %u" //flags
         " %lu" //minflt
         " %lu" //cminflt
         " %lu" //majflt
         " %lu" //cmajflt
         " %lu" //utime
         " %lu" //stime
         " %ld" //cutime
         " %ld" //cstime
         " %ld" //priority
         " %ld" //nice
         " %ld" //num_threads
         " %ld" //itrealvalue
         " %llu" //starttime
         " %lu" //vsize
         " %ld" //rss
         " %lu" //rsslim
         " %lu" //startcode
         " %lu" //endcode
         " %lu" //startstack
         " %lu" //kstkesp
         " %lu" //kstkip
         " %lu" //signal
         " %lu" //blocked
         " %lu" //sigignore
         " %lu" //sigcatch
         " %lu" //wchan
         " %lu" //nswap
         " %lu" //cnswap
         " %d" //exit_signal
         " %d" //processor
         " %u" //rt_priority
         " %u" //policy
         " %llu" //delayacct_blkio_ticks
         " %lu" //guest_time
         " %ld" //cguest_time
         ,&(output.pid), &(output.state), &(output.ppid), &(output.pgrp), &(output.session),
         &(output.tty_nr), &(output.tpgid), &(output.flags), &(output.minflt), &(output.cminflt),
         &(output.majflt), &(output.cmajflt), &(output.utime), &(output.stime), &(output.cutime),
         &(output.cstime), &(output.priority), &( output.nice), &(output.num_threads), &(output.itrealvalue),
         &(output.starttime), &(output.vsize), &(output.rss), &(output.rsslim), &(output.startcode),
         &(output.endcode), &(output.startstack), &(output.kstkesp), &(output.kstkeip), &(output.signal),
         &(output.blocked), &(output.sigignore), &(output.sigcatch), &(output.wchan), &(output.nswap),
         &(output.cnswap), &(output.exit_signal), &(output.processor), &(output.rt_priority), &(output.policy),
         &(output.delayacct_blkio_ticks), &(output.guest_time), &(output.cguest_time)
  );
  if(num_succes != 43) { // 43 is number of iteams in Resources::PidStats
    LOG4CXX_ERROR(logger_, "Couldn't parse all iteams in /proc/PID/stat file");
    return false;
  }
  return true;
#elif defined(__QNXNTO__)
  int fd = open(GetProcPath().c_str(), O_RDONLY);
  if (0 >= fd) {
    LOG4CXX_ERROR(logger_, "Failed open process proc file : " << GetProcPath() <<
                  "; error no : " << strerror( errno ) );

    close(fd);
    return false;
  }
  devctl(fd, DCMD_PROC_INFO, &output, sizeof(output), 0);
  close(fd);
  return true;
#else
  return true;
#endif
}

bool Resources::GetMemInfo(Resources::MemInfo &output) {
  bool result = false;
#if defined(OS_WIN32) || defined(OS_WINCE)
  Resources::PidStats pid_stat;
  if (false == GetProcInfo(pid_stat)) {
	  LOG4CXX_ERROR(logger_, "Failed to get proc info");
	  result = false;
  } else {
	  output = pid_stat.vsize;
	  result = true;
  }
#elif defined(OS_LINUX)
  Resources::PidStats pid_stat;
  if (false == GetProcInfo(pid_stat)) {
    LOG4CXX_ERROR(logger_, "Failed to get proc info");
    result = false;
  } else {
    output = pid_stat.vsize;
    result = true;
  }

#elif defined(__QNXNTO__)
  std::string as_path = GetStatPath();
  struct stat st;
  struct _dir* proc_dir = 0;
  if (0 == (proc_dir = opendir(proc))) {
    LOG4CXX_ERROR(logger_, "Unable to access to " << proc);
    result = false;
    return result;
  }  
  if (0 == readdir(proc_dir)) {
    LOG4CXX_ERROR(logger_, "Unable to read : " << proc_dir);
    closedir(proc_dir);
    result = false;
    return result;
  }
  closedir(proc_dir);
  if (-1 == stat(as_path.c_str(), &st) || 0 == st.st_size) {
     LOG4CXX_ERROR(logger_, "Unable to stat : " << as_path.c_str());
     result = false;
     return result;
  }
  output = st.st_size;
  result = true;
#endif
  return result;
}

std::string Resources::GetStatPath() {
  std::string filename;
#if defined(OS_LINUX)
  filename = GetProcPath() + "/stat";
#elif defined(__QNXNTO__)
  filename = GetProcPath() + "/as";
#endif

#if defined(OS_WIN32) || defined(OS_WINCE)
  filename = GetProcPath() + "/memstat";
#endif

  return filename;
}

std::string Resources::GetProcPath() {
  char buffer[1024];
  pid_t my_pid = getpid();
  snprintf(buffer, sizeof(buffer), "%s%d/", proc , my_pid);
  std::string filename(buffer);
  return filename;
}

} // namespace utils
