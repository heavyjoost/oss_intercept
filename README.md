# oss_intercept

Saves and restores volume settings per application in FreeBSD

## Usage
* Method 1: LD_PRELOAD=THISLIB EXECUTABLE_TO_RUN
* Method 2: patchelf --add-needed THISLIB EXECUTABLE_TO_RUN
