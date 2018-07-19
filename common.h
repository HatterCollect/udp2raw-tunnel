/*
 * common.h
 *
 *  Created on: Jul 29, 2017
 *      Author: wangyu
 */

#ifndef UDP2RAW_COMMON_H_
#define UDP2RAW_COMMON_H_
#define __STDC_FORMAT_MACROS 1
#include <inttypes.h>

#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<getopt.h>

#include<unistd.h>
#include<errno.h>
#include <sys/epoll.h>
#include <sys/wait.h>
#include <sys/socket.h>    //for socket ofcourse
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h> //for exit(0);
#include <errno.h> //For errno - the error number
#include <netdb.h> // for gethostbyname()
#include <netinet/tcp.h>   //Provides declarations for tcp header
#include <netinet/udp.h>
#include <netinet/ip.h>    //Provides declarations for ip header
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <byteswap.h>
#include <arpa/inet.h>
#include <linux/if_ether.h>
#include <linux/filter.h>
#include <sys/time.h>
#include <time.h>
#include <sys/timerfd.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <assert.h>
#include <linux/if_packet.h>
#include <byteswap.h>
#include <pthread.h>

#include<unordered_map>
#include <fstream>
#include <string>
#include <vector>
#include <map>
#include <set>
using  namespace std;


typedef unsigned long long u64_t;   //this works on most platform,avoid using the PRId64
typedef long long i64_t;

typedef unsigned int u32_t;
typedef int i32_t;

typedef u32_t id_t;

typedef u64_t iv_t;

typedef u64_t padding_t;

typedef u64_t anti_replay_seq_t;

const int max_addr_len=100;

extern int force_socket_buf;

/*
struct ip_port_t
{
	u32_t ip;
	int port;
	void from_u64(u64_t u64);
	u64_t to_u64();
	char * to_s();
};*/

typedef u64_t fd64_t;

u32_t djb2(unsigned char *str,int len);
u32_t sdbm(unsigned char *str,int len);

struct address_t  //TODO scope id
{
	struct hash_function
	{
	    u32_t operator()(const address_t &key) const
		{
	    	return sdbm((unsigned char*)&key.inner,sizeof(key.inner));
		}
	};

	union storage_t //sockaddr_storage is too huge, we dont use it.
	{
		sockaddr_in ipv4;
		sockaddr_in6 ipv6;
	};
	storage_t inner;

	address_t()
	{
		clear();
	}
	void clear()
	{
		memset(&inner,0,sizeof(inner));
	}
	int from_str(char * str);

	int from_sockaddr(sockaddr *,socklen_t);

	char* get_str();
	void to_str(char *);

	inline u32_t get_type()
	{
		return ((sockaddr*)&inner)->sa_family;
	}

	inline u32_t get_len()
	{
		u32_t type=get_type();
		switch(type)
		{
			case AF_INET:
				return sizeof(sockaddr_in);
			case AF_INET6:
				return sizeof(sockaddr_in6);
			default:
				assert(0==1);
		}
		return -1;
	}

	inline u32_t get_port()
	{
		u32_t type=get_type();
		switch(type)
		{
			case AF_INET:
				return ntohs(inner.ipv4.sin_port);
			case AF_INET6:
				return ntohs(inner.ipv6.sin6_port);
			default:
				assert(0==1);
		}
		return -1;
	}

	inline void set_port(int port)
	{
		u32_t type=get_type();
		switch(type)
		{
			case AF_INET:
				inner.ipv4.sin_port=htons(port);
				break;
			case AF_INET6:
				inner.ipv6.sin6_port=htons(port);
				break;
			default:
				assert(0==1);
		}
		return ;
	}

    bool operator == (const address_t &b) const
    {
    	//return this->data==b.data;
        return memcmp(&this->inner,&b.inner,sizeof(this->inner))==0;
    }

    int new_connected_udp_fd();

    char* get_ip();
};

namespace std {
template <>
 struct hash<address_t>
 {
   std::size_t operator()(const address_t& key) const
   {

	 //return address_t::hash_function(k);
	   return sdbm((unsigned char*)&key.inner,sizeof(key.inner));
   }
 };
}

struct not_copy_able_t
{
	not_copy_able_t()
	{

	}
	not_copy_able_t(const not_copy_able_t &other)
	{
		assert(0==1);
	}
	const not_copy_able_t & operator=(const not_copy_able_t &other)
	{
		assert(0==1);
		return other;
	}
};

const int max_data_len=1800;
const int buf_len=max_data_len+400;

const int max_address_len=512;

u64_t get_current_time();
u64_t pack_u64(u32_t a,u32_t b);

u32_t get_u64_h(u64_t a);

u32_t get_u64_l(u64_t a);

char * my_ntoa(u32_t ip);

void init_random_number_fd();
u64_t get_true_random_number_64();
u32_t get_true_random_number();
u32_t get_true_random_number_nz();
u64_t ntoh64(u64_t a);
u64_t hton64(u64_t a);
bool larger_than_u16(uint16_t a,uint16_t b);
bool larger_than_u32(u32_t a,u32_t b);
void setnonblocking(int sock);
int set_buf_size(int fd,int socket_buf_size);

void myexit(int a);

unsigned short csum(const unsigned short *ptr,int nbytes);

int numbers_to_char(id_t id1,id_t id2,id_t id3,char * &data,int &len);
int char_to_numbers(const char * data,int len,id_t &id1,id_t &id2,id_t &id3);

const int show_none=0;
const int show_command=0x1;
const int show_log=0x2;
const int show_all=show_command|show_log;

int run_command(string command,char * &output,int flag=show_all);
//int run_command_no_log(string command,char * &output);
int read_file(const char * file,string &output);

vector<string> string_to_vec(const char * s,const char * sp);
vector< vector <string> > string_to_vec2(const char * s);

string trim(const string& str, char c);

string trim_conf_line(const string& str);

vector<string> parse_conf_line(const string& s);

int hex_to_u32_with_endian(const string & a,u32_t &output);
int hex_to_u32(const string & a,u32_t &output);
//extern string iptables_pattern;

int create_fifo(char * file);

void print_binary_chars(const char * a,int len);


#endif /* COMMON_H_ */
