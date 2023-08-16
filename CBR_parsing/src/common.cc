//
// Created by Vito Wu on 6/11/2021.
//

#include "common.hh"

/* convert timeval to miliseconds */
#define TIMEVAL2F(stamp) \
    ((stamp).tv_sec * 1000.0 + (stamp).tv_usec / 1000.0)

/* get timestamp to the precision of miliseconds since the program starts */
double get_timestamp() {
    static double __init_stamp = -1;
    static struct timeval __cur_time;

    if (-1 == __init_stamp) {
        gettimeofday(&__cur_time, NULL);
        __init_stamp = TIMEVAL2F(__cur_time);
    }

    gettimeofday(&__cur_time, NULL);
    return ((TIMEVAL2F(__cur_time) - __init_stamp) / 1000.0);
}

#undef TIMEVAL2F

/* function: safe_malloc
 * usage: abort if malloc failed
 * arguments: size, number of bytes to allocate
 * return: a void* pointer
 */
void *
safe_malloc(size_t size) {
    void *ptr = NULL;
    if (NULL == (ptr = malloc(size))) {
        EXIT_WITH_MSG("[!] insufficient memory\n");
    }
    return ptr;
}

/* function: get_cpu_freq()
 * usage: read CPU frequency of the platform. Only Linux is supported.
 * arguments: void
 * return: CPU frequency and -1 on error
 */
double
get_cpu_freq() {
    static char *cpu_freq_file = "/sys/devices/system/cpu/cpu0/cpufreq"
                                 "/cpuinfo_max_freq";  /* read the max freq */
    FILE *fp = NULL;
    char buf[1024];  /* should be less than 1024 bytes */

    if (NULL == (fp = fopen(cpu_freq_file, "r"))) {
        PRINTF_ERR_STAMP("[!] error opening file: %s\n", cpu_freq_file);
        return -1;
    }

    if (NULL == fgets(buf, 1024, fp)) {
        PRINTF_ERR_STAMP("[!] error reading from file: %s\n", cpu_freq_file);
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return atof(buf);
}

/* function: get_proc_status
 * usage: read the following info from /proc/$PID/status
 *      VmPeak          peak virtual memory size
 *      VmSize          total program size
 *      VmLck           locked memory size
 *      VmHWM           peak resident set size ("high water mark")
 *      VmRSS           size of memory portions
 *      VmData          size of data, stack, and text segments
 *      VmStk           size of data, stack, and text segments
 *      VmExe           size of text segment
 *      VmLib           size of shared library code
 *      VmPTE           size of page table entries
 *      VmSwap          size of swap usage (the number of referred swapents)
 * arguments:
 *      1) proc_info: a buffer to hold the info.
 *      2) size: size of the buffer
 * return: normally 0, -1 on error, -2 on insufficient space
 */
int get_proc_status(char *proc_info, size_t size) {
    char buf[1024];
    snprintf(buf, 1024, "/proc/%d/status", getpid());

    FILE *fp = NULL;
    if (NULL == (fp = fopen(buf, "r"))) {
        PRINTF_ERR_STAMP("[!] error open proc status: %s\n", buf);
        return -1;
    }

    proc_info[0] = '\0';  /* init */
    size_t space_left = size - 1;
    size_t line_len = 0;

    while (NULL != fgets(buf, 1024, fp)) {
        if (0 != strncmp(buf, "Vm", 2)) continue;  /* skip irrelevant lines */

        line_len = strlen(buf);
        if (space_left < line_len) {
            fclose(fp);
            return -2;
        }

        strcat(proc_info, buf);
        space_left -= line_len;
    }

    fclose(fp);
    return 0;
}