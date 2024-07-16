#ifndef CONFIG_MODULE_H
#define CONFIG_MODULE_H
/*************************************************************************/
/*********
$ curl --location 'http://127.0.0.1:8181/v1/data/ebpf/allow' --header 'Content-Type: application/json' --data '{"input": {"funcName": "mptm_decap"}}'

> {"result":false}
*********/
/*************************************************************************/
#ifdef URL
#undef URL
#endif
#define URL "/v1/data/ebpf/allow"
/**********/
#ifdef SERVER_PORT
#undef SERVER_PORT
#endif
#define SERVER_PORT 8181
/**********/
#ifdef FILE_PATH
#undef FILE_PATH
#endif
#define FILE_PATH "/boot/kvstore"
/**********/
#ifdef BUFFER_SIZE
#undef BUFFER_SIZE
#endif
#define BUFFER_SIZE 512

/// $curl --location 'http://192.168.0.103:8181/v1/data/ebpf/allow' --header 'Content-Type: application/json' --data '{"input": {"funcName": "mptm_encap"}}'
/// {"result":true}

//#define SERVER_PORT 8181
////$IP="192.168.0.103"; printf '%02X' ${IP//./ }| sed 's/../& /g' | awk 'BEGIN {printf "0x"} {for(i=NF;i>=1;i--) printf "%s", $i} END {print ""}'
/// #define SERVER_ADDR 0x6700A8C0
/**********/
#ifdef SERVER_IP
#undef SERVER_IP
#endif
#define SERVER_IP "192.168.0.103"
/**********/
#ifdef SERVER_ADDR
#undef SERVER_ADDR
#endif
#define SERVER_ADDR 0x6700A8C0


//#define SERVER_ADDR "127.0.0.1"
// $IP="127.0.0.1"; printf '%02X' ${IP//./ }| sed 's/../& /g' | awk 'BEGIN {printf "0x"} {for(i=NF;i>=1;i--) printf "%s", $i} END {print ""}'

//#define SERVER_IP "127.0.0.1"
//#define SERVER_ADDR 0x100007f
/*************************************************************************/
#endif