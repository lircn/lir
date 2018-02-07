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
#include <mysql/mysql.h>

#include <linux/version.h>

using namespace std;

#define U32_MAX ((uint32_t)~0U)

#define BUF_LEN			(1024)
#define IFNAME_LIMIT	(10)

#define OK				(0)
#define ERR				(-1)

#define QLOGERR			cout

void show_all_if()
{
	int fdSock = 0;
	struct ifconf ifconf;
	struct ifreq ifreq;
	char szBuf[20480] = {0};
	char * ip;


	ifconf.ifc_len = 2048;
	ifconf.ifc_buf = szBuf;

	if((fdSock = socket (AF_INET, SOCK_DGRAM, 0)) < 0  )
	{
		cout << "socket error" <<endl;
		return;
	}

	if(ioctl (fdSock, SIOCGIFCONF, &ifconf))
	{
		close(fdSock);
		cout << "ioctl error \n"<<endl;
		return;
	}

	struct ifreq *it = ifconf.ifc_req;
	const struct ifreq * const end = it + (ifconf.ifc_len / sizeof(struct ifreq));

	for(;it != end; ++it)
	{
		strcpy(ifreq.ifr_name, it->ifr_name);
		cout<< ifreq.ifr_name <<endl;

		if(!strstr(ifreq.ifr_name, "lo:"))
		{
			continue;
		}

		if(ioctl(fdSock, SIOCGIFADDR, &ifreq) == 0)
		{
			string strAddr;
			struct sockaddr_in *sin;
			sin = (struct sockaddr_in*) &ifreq.ifr_addr;

			strAddr = (const char *) inet_ntoa(sin->sin_addr);
			cout << "ip addr : "<<strAddr.c_str()<<endl;
		}
		else
		{
			cout << "get mac info error \n"<<endl;
		}


		if(ioctl(fdSock, SIOCGIFNETMASK, &ifreq) == 0)
		{
			string strMask;
			struct sockaddr_in *sin;
			sin = (struct sockaddr_in*) &ifreq.ifr_netmask;

			strMask = (const char *) inet_ntoa(sin->sin_addr);
			cout << "net mask : "<<strMask.c_str()<<endl;
		}
		else
		{
			cout << "get mask info error \n"<<endl;
		}

	}

	close(fdSock);
}

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


void func(vector<int> &out)
{
	vector<int> v;
	v.push_back(1);
	v.push_back(2);
	out = v;
}

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
				return -1;	//	cant be decoded
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
	//	first, find the first "?", left is path
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

	//	third, find the last '/' in the left
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

	//	fifth, convert the "%6D" into char arg, in the right
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

int main(int argc, char **argv)
{
	char url[] = "192.168/haha?a=1&b=2";
	DEBUG("%s\n", url);

	return 0;
}
