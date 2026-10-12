#ifndef FUNCTIONMACROS_H
#define FUNCTIONMACROS_H

#ifdef TESTABLE
#define APIDEF(RET_TYPE, FUNC_NAME, ...)                                       \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__);                                        \
  RET_TYPE (*FUNC_NAME)(__VA_ARGS__) = FUNC_NAME##_OG;                         \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define APIDEC(RET_TYPE, FUNC_NAME, ...)                                       \
  extern RET_TYPE (*FUNC_NAME)(__VA_ARGS__);                                   \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define INTDEF(RET_TYPE, FUNC_NAME, ...)                                       \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__);                                        \
  RET_TYPE (*FUNC_NAME)(__VA_ARGS__) = FUNC_NAME##_OG;                         \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define INTDEC(RET_TYPE, FUNC_NAME, ...)                                       \
  extern RET_TYPE (*FUNC_NAME)(__VA_ARGS__);                                   \
  RET_TYPE FUNC_NAME##_OG(__VA_ARGS__)
#define RESET_FN(FUNC_NAME) FUNC_NAME = FUNC_NAME##_OG
#else
#define APIDEF(RET_TYPE, FUNC_NAME, ...) RET_TYPE FUNC_NAME(__VA_ARGS__)
#define APIDEC(RET_TYPE, FUNC_NAME, ...) RET_TYPE FUNC_NAME(__VA_ARGS__)
#define INTDEF(RET_TYPE, FUNC_NAME, ...) static RET_TYPE FUNC_NAME(__VA_ARGS__)
#define INTDEC(RET_TYPE, FUNC_NAME, ...)
#define RESET_FN(FUNC_NAME)
#endif

#endif
