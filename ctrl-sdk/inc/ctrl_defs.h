#ifndef CTRL_DEFS_H
#define CTRL_DEFS_H

#define DEF_SECTION(x) __attribute__((section(x)))
#define DEF_USED __attribute__((used))
#define DEFIGN(n) __attribute__((aligned(n)))
#define DEF_WEAK __attribute__((weak))
#define DEF_inline __inline

#endif // CTRL_DEFS_H
