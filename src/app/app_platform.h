// The program's platform switches: APP_IOS on the iPhone and iPad (Apple's TARGET_OS_IPHONE), APP_TOUCH where the
// game is played with the fingers (Android and iOS: the touch gestures, the on-screen keyboard and the display size of
// play.c).

#ifndef APP_APP_PLATFORM_H
#define APP_APP_PLATFORM_H

#if defined(__APPLE__)
#include <TargetConditionals.h>
#if TARGET_OS_IPHONE
#define APP_IOS 1
#endif
#endif

#if defined(__ANDROID__) || defined(APP_IOS)
#define APP_TOUCH 1
#endif

#endif
