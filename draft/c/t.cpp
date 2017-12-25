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
#include <arpa/inet.h>
#include <sys/ioctl.h>
#include <mysql/mysql.h>

#include "iot_access_db.h"

using namespace std;

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

int main(int argc, char **argv)
{
	CIotdb cidb;
	cidb.init(DB_HOST, DB_NAME, DB_USR, DB_PWD, DB_PORT);
	//cidb.mqtt3_db_message_store(1, 1, "topic", 2, 0, NULL, 0);

	vector<MqttMessage> mm_list = cidb.mqtt3_db_message_get_retain(1, 2);

	vector<MqttMessage>::iterator it = mm_list.begin();
	vector<MqttMessage>::iterator it_end = mm_list.end();
	for (; it != it_end; ++it) {
		cout << (*it).topic << (*it).mid << endl;
	}

	char topic[] = "";
	char filter[] = "";
	cout << (mqtt_topic_match(argv[1], argv[2])?"true":"false") << endl;

	return 0;
}
