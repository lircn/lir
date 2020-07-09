#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <string>
#include <list>
#include <map>
#include <deque>
#include <fstream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <net/if.h>
#include <netdb.h>
#include <vector>
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <unordered_map>

typedef unsigned int u32;
typedef unsigned short u16;

using namespace std;

#define U32_MAX ((uint32_t)~0U)

#define BUF_LEN         (1024)
#define IFNAME_LIMIT    (10)

#define OK              (0)
#define ERR             (-1)

#define QLOGERR         cout

static int setup_vip(uint32_t vip)
{
    int fd = 0;
    struct ifreq req;
    struct sockaddr_in *sin;
    char ifname[BUF_LEN] = {0};
    int i = 0;
    int ret = ERR;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        cout << "set vip socket error" << endl;
        return ret;
    }

    for (; i < IFNAME_LIMIT; i++) {
        memset(&req, 0, sizeof(req));
        sprintf(ifname, "lo:%d", i);
        strcpy(req.ifr_name, ifname);

        sin = (struct sockaddr_in *)&req.ifr_addr;
        sin->sin_family = AF_INET;

        /* flag */
#if 0
        if (ioctl(fd, SIOCGIFFLAGS, &ifr) < 0) {
            cout << "get flag error" << endl;
            continue;
        }
        ifr.ifr_ifru.ifru_flags |= IFF_UP;
        if (ioctl(fd, SIOCSIFFLAGS, &ifr) < 0)
        {
            cout << "set flag error" << endl;
            continue;
        }
#endif

        printf("%s\n", ifname);
        if (ioctl(fd, SIOCGIFADDR, &req) == 0) {
            continue;
        }
        printf("else: %s\n", ifname);

        /* IP */
        sin->sin_addr.s_addr = vip;
        if (ioctl(fd, SIOCSIFADDR, &req) < 0) {
            printf("set vip %d ip error", vip);
            break;
        }

        /* mask */
        sin->sin_addr.s_addr = INADDR_BROADCAST;
        if (ioctl(fd, SIOCSIFNETMASK, &req) < 0) {
            printf("set vip %d mask error", vip);
            break;
        }

        ret = OK;
        break;
    }

    close(fd);
    return ret;
}

static int shutdown_vip(uint32_t vip)
{
    int fd = 0;
    struct ifreq req;
    struct sockaddr_in *sin;
    char ifname[BUF_LEN] = {0};
    int i = 0;
    int ret = ERR;

    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        QLOGERR << "shutdown vip socket error" << endl;
        return ret;
    }

    for (; i < IFNAME_LIMIT; i++) {
        memset(&req, 0, sizeof(req));
        sprintf(ifname, "lo:%d", i);
        strcpy(req.ifr_name, ifname);

        if (ioctl(fd, SIOCGIFADDR, &req) == 0) {
            sin = (struct sockaddr_in *)&req.ifr_addr;
            if (vip == sin->sin_addr.s_addr) {
                if (ioctl(fd, SIOCGIFFLAGS, &req) < 0) {
                    QLOGERR << "shutdown vip get flag error" << endl;
                    break;
                }

                short flag;
                flag = ~IFF_UP;
                req.ifr_ifru.ifru_flags &= flag;

                if (ioctl(fd, SIOCSIFFLAGS, &req) < 0) {
                    QLOGERR << "shutdown vip set flag error" << endl;
                    break;
                }

                ret = OK;
                break;
            }
        }
        else {
            continue;
        }
    }

    close(fd);
    return ret;
}

void split_string(const std::string& s, std::list<std::string>& v, const std::string& c)
{
    std::string::size_type pos1, pos2;
    pos2 = s.find(c);
    pos1 = 0;
    while(std::string::npos != pos2)
    {
        v.push_back(s.substr(pos1, pos2-pos1));

        pos1 = pos2 + c.size();
        pos2 = s.find(c, pos1);
    }
    if(pos1 != s.length())
        v.push_back(s.substr(pos1));
}

static bool check_attack(void)
{
    ifstream in("/proc/sys/net/ipv4/tcp_mem");
    string line;
    if (in) {
        while (getline(in, line)) {
            cout << line << endl;
        }
    }
    return true;
}

typedef struct _t {
    void *a;
    void *b;
} T;

#define DB_HOST "127.0.0.1"
#define DB_PORT (3306)
#define DB_NAME "iot"
#define DB_USR "root"
#define DB_PWD "123"

typedef map<string, string> HttpArgs;

vector<string> process_keyvalue(const string &input, char sep)
{
    string::size_type pos = string::npos;
    vector<string> ret;

    pos = input.find(sep);
    if (pos == string::npos) {
        return ret;
    }

    ret.push_back(input.substr(0, pos));
    ret.push_back(input.substr(pos + 1));
    return ret;
}

int process_args(const string &input)
{
    string::size_type pos = string::npos;
    string tmp_str;
    vector <string> kv;

    tmp_str = input;

    while (tmp_str.length() > 0) {
        pos = tmp_str.find("&");
        kv = process_keyvalue(tmp_str.substr(0, pos), '=');
        if (kv.size() == 0) {
            return -1;
        }
        cout << kv.at(0) << endl;
        cout << kv.at(1) << endl;
        if (pos == string::npos) {
            break;
        }
        tmp_str = tmp_str.substr(pos + 1);
    }

    return 0;
}

inline void dbg(const char *format, ...)
{
    va_list ap;

    va_start(ap, format);
    printf("%s %d: ", __func__, __LINE__);
    printf(format, ap);
    printf("\n");
    va_end(ap);
}

enum A {
    A_A,
    A_B,
    A_MAX
};


int32_t decode_string(const char* data, uint32_t data_len,
        char* buf, uint32_t buf_size, uint32_t& result_len)
{
    uint32_t j = 0;
    for(uint32_t i = 0; i < data_len && j < buf_size
            ; i++, j++)
    {
        if (data[i] == '%')
        {
            if (i+2 >= data_len)
            {
                return -1;  //  cant be decoded
            }
            char sPercentage[3] = {0};
            sPercentage[0] = data[i+1];
            sPercentage[1] = data[i+2];
            buf[j] = (char)strtol(sPercentage, (char **)0, 16);
            i += 2;
        }
    }
    result_len = j;
    return 0;
}

int32_t parse_uri(const char* data, uint32_t data_len
        , std::string& path, std::string& file, std::string& arg)
{
    //  first, find the first "?", left is path
    uint32_t uri_end = U32_MAX;
    for(uint32_t i = 0; i < data_len; i++)
    {
        if (data[i] == '?')
        {
            uri_end = i;
            break;
        }
    }
    if (uri_end == U32_MAX)
    {
        uri_end = data_len;
    }

    //  third, find the last '/' in the left
    uint32_t file_start = U32_MAX;
    for(uint32_t i = uri_end; i != 0; i--)
    {
        if (data[i-1] == '/')
        {
            file_start = i-1;
            break;
        }
    }
    if (file_start == U32_MAX)
    {
        return -1;
    }
    else
    {
        path = string(data, file_start);
        file = string(data + file_start, uri_end - file_start);
    }

    //  fifth, convert the "%6D" into char arg, in the right
    if (uri_end == data_len)
    {
        arg = "";
    }
    else
    {
        static const uint32_t C_MAX_ARG_SIZE = 1<<10;
        char sArg[C_MAX_ARG_SIZE];
        uint32_t arg_len = 0;
        int32_t ret = decode_string(data + uri_end + 1, data_len - uri_end - 1
                , sArg, sizeof(sArg), arg_len);
        if (ret)
        {
            return ret;
        }
        sArg[arg_len] = 0;
        arg = sArg;
    }
    return 0;
}

#define DEBUG(format, ...) printf("[%s](%d)" format, __func__, __LINE__, __VA_ARGS__)

const char *print_ip(u32 ip, u16 port)
{
    static char str[50];
    snprintf(str, 50, "%d.%d.%d.%d:%u", ip & 0xff, (ip >> 8) & 0xff,
            (ip >> 16) & 0xff, (ip >> 24) & 0xff, port);
    return str;
}

 struct ListNode {
     int val;
     struct ListNode *next;
 };

struct ListNode* addList(struct ListNode *h, int v) {
    if (h) {
        struct ListNode *n = (struct ListNode *)malloc(sizeof(struct ListNode));
        n->val = v;
        n->next = NULL;
        h->next = n;
    } else {
        h = (struct ListNode *)malloc(sizeof(struct ListNode));
        h->val = v;
        h->next = NULL;
    }

    return h;
}
struct ListNode* addTwoNumbers(struct ListNode* l1, struct ListNode* l2){
    struct ListNode *h = NULL;
    int cnt = 0;
    int v = 0;
    while (l1 || l2) {
        v = cnt;
        cnt = 0;
        if (l1 && l2) {
            v += l1->val + l2->val;
            l1 = l1->next;
            l2 = l2->next;
        } else if (l1) {
            v += l1->val;
            l1 = l1->next;
        } else if (l2) {
            v += l2->val;
            l2 = l2->next;
        }
        if (v > 9) {
            v -= 10;
            cnt = 1;
        }
        h = addList(h, v);
    }
    return h;
}



inline int find(char *s, int start, int end, char c) {
    for (; start <= end; start++) {
       if (s[start] == c) break; 
    }
    if (start>end) return -1;
    else return start;
}

int lengthOfLongestSubstring(char * s){
    int start = 0;
    int end = 0;
    int max = 0;
    while (s[end]) {
        int pos = find(s, start, end, s[end+1]);
        int len = end-start+1;
        if (pos >= 0) {
            start = pos+1;
        }
        if (max < len) max = len;
        printf("max %d, pos %d, %d - %d\n", max, pos, start,end);
        end++;
    }
    return max;
}


static void knet_set_env_helper(const char *str, int len)
{
    static char buf[1024] = "";

    if (len == 0) {
        return;
    }

    strncpy(buf, str, len);
    int i = 0;
    while (1) {
        if (buf[i] == '=') {
            buf[i] = 0;
            break;
        }
        i++;
    }

    printf("key: %s, val:%s\n", buf, buf+i+1);
}

void knet_set_env(const char *str)
{
    const char symbol = '|';
    const char *p1, *p2;
    p1 = p2 = str;

    while (1) {
        if (*p2 == 0) {
            knet_set_env_helper(p1, p2 - p1);
            break;
        } else if (*p2 == symbol) {
            knet_set_env_helper(p1, p2 - p1);
            p2++;
            p1 = p2;
        } else {
            p2++;
        }
    }
}

typedef struct _S {
    int i;
    char *c;
}S;

void func(unordered_map<int, S> &m)
{
    S s;
    s.i = 1;
    s.c = (char *)malloc(10);
    sprintf(s.c, "123");
    m[1] = s;
}

int main(int argc, char **argv)
{
    unordered_map<int, S> m;
    func(m);

    S *p = &(m[1]);
    printf("1 %d, %s\n", p->i, p->c);

    m.erase(1);
    printf("2 %d, %s\n", p->i, p->c);

    cout << m.size() << endl;

    return 0;
}
