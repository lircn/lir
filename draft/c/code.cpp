
#define DEBUG(format, ...) printk("[%s](%d)" format, __func__, __LINE__, ##__VA_ARGS__)
