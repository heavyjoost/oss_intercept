/* 
 * Usage:
 * - Method 1: LD_PRELOAD=THISLIB EXECUTABLE_TO_RUN
 * - Method 2: patchelf --add-needed THISLIB EXECUTABLE_TO_RUN
 */
#include <dlfcn.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/soundcard.h>
#include <sys/stat.h>
#include <unistd.h>
/* Hackish way to exclude declaration of open() but it works: */
#define _KERNEL
#include <fcntl.h>
#undef _KERNEL

#if DEBUG
    #undef DEBUG
    #define DEBUG(...) fprintf(stderr, "DEBUG: "__VA_ARGS__); fprintf(stderr, "\n")
#else
    #undef DEBUG
    #define DEBUG(...)
#endif

#ifndef VOLPATH
    #define VOLPATH "/tmp/oss_intercept"
#endif


typedef int (*open_t)(const char*, int, mode_t);
typedef int (*close_t)(int);
typedef int (*ioctl_t)(int, unsigned long, void*);
struct volume {
    uint8_t l;
    uint8_t r;
};

static int dsp_fd=-1;
static open_t o_open = NULL;
static close_t o_close = NULL;
static ioctl_t o_ioctl = NULL;

__attribute__((constructor)) static void set_funcs(){
    o_open=(open_t)dlsym(RTLD_NEXT, "open");
    o_close=(close_t)dlsym(RTLD_NEXT, "close");
    o_ioctl=(ioctl_t)dlsym(RTLD_NEXT, "ioctl");
    
    /* Use mkdir + chmod to ignore the umask but not for the whole process */
    mkdir(VOLPATH, 0777);
    chmod(VOLPATH, 0777);
}

int open(const char *pathname, int flags, mode_t mode){
    int r=o_open(pathname, flags, mode);
    if (r>-1 && !strcmp(pathname, "/dev/dsp")){
        DEBUG("open /dev/dsp");
        dsp_fd=r;
        
        char volpath[256]=VOLPATH;
        strcat(volpath, getprogname());
        int tfd=o_open(volpath, O_RDONLY, 0);
        if (tfd>-1){
            struct volume volume;
            read(tfd, &volume, sizeof(volume));
            o_close(tfd);
            DEBUG("restore volume: %u:%u", volume.l, volume.r);
            o_ioctl(dsp_fd, MIXER_WRITE(SOUND_MIXER_PCM), &volume);
        }
    }
    return r;
}

int close(int fd){
    if (dsp_fd>-1 && fd==dsp_fd){
        DEBUG("closing /dev/dsp");
        struct volume volume;
        o_ioctl(fd, MIXER_READ(SOUND_MIXER_PCM), &volume);
        DEBUG("save volume: %u:%u", volume.l, volume.r);
        char volpath[256]=VOLPATH;
        strcat(volpath, getprogname());
        int tfd=o_open(volpath, O_WRONLY | O_CREAT | O_TRUNC, 0666);
        write(tfd, &volume, sizeof(volume));
        o_close(tfd);
        dsp_fd=-1;
    }
    return o_close(fd);
}

#warning TODO: remove ioctl stuff or do something useful (:
#if 0
int ioctl(int fd, unsigned long request, void *argp){
    int r=o_ioctl(fd, request, argp);
    struct volume *volume=argp;
    if (request == MIXER_WRITE(SOUND_MIXER_PCM)){
        if (dsp_fd==fd){
            DEBUG("/dev/dsp write(pcm)=%u:%u", volume->l, volume->r);
        } else {
            DEBUG("write(pcm)=%u:%u", volume->l, volume->r);
        }
    } else if (request == MIXER_READ(SOUND_MIXER_PCM)){
        if (dsp_fd==fd){
            DEBUG("/dev/dsp read(pcm)=%u:%u", volume->l, volume->r);
        } else {
            DEBUG("read(pcm)=%u:%u", volume->l, volume->r);
        }
    }
    return r;
}
#endif
