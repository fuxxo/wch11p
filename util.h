#define TEST_ASSERT(condition, cmd) do {			\
    if (condition) {}						\
    else {							\
      printk(KERN_ERR "Test(%s:%d): %s\n",			\
	      __FUNCTION__, __LINE__, #condition);		\
      cmd;							\
    }								\
  } while (0)

#define RETURN_ASSERT(condition, cmd) do {			\
    if (condition) {}                                           \
    else {                                                      \
      printk(KERN_ERR "Return(%s:%d): %s\n",			\
	      __FUNCTION__, __LINE__, #condition);		\
      return (cmd);						\
    }                                                           \
  } while (0)

#define GOTO_ASSERT(condition, cmd, label) do {			\
    if (condition) {}                                           \
    else {                                                      \
      printk(KERN_ERR "Goto(%s:%d): %s\n",			\
	      __FUNCTION__, __LINE__, #condition);		\
      cmd;							\
      goto label;                                               \
    }                                                           \
  } while (0)

