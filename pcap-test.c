#include <pcap.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdint.h>
#include <netinet/in.h>
#include <string.h>
#define ETHER_ADDR_LEN 6

void usage() {
	printf("syntax: pcap-test <interface>\n");
	printf("sample: pcap-test wlan0\n");
}

typedef struct {
	char* dev_;
} Param;

Param param  = {
	.dev_ = NULL
};

struct libnet_ethernet_hdr{
	u_int8_t  ether_dhost[ETHER_ADDR_LEN];/* destination ethernet address */
	u_int8_t  ether_shost[ETHER_ADDR_LEN];/* source ethernet address */
	u_int16_t ether_type;                 /* protocol */
};

struct libnet_ipv4_hdr{
    #if (LIBNET_LIL_ENDIAN)
        u_int8_t ip_hl:4,      /* header length */
               ip_v:4;         /* version */
    #endif
    #if (LIBNET_BIG_ENDIAN)
        u_int8_t ip_v:4,       /* version */
               ip_hl:4;        /* header length */
    #endif
        u_int8_t ip_tos;       /* type of service */
    #ifndef IPTOS_LOWDELAY
    #define IPTOS_LOWDELAY      0x10
    #endif
    #ifndef IPTOS_THROUGHPUT
    #define IPTOS_THROUGHPUT    0x08
    #endif
    #ifndef IPTOS_RELIABILITY
    #define IPTOS_RELIABILITY   0x04
    #endif
    #ifndef IPTOS_LOWCOST
    #define IPTOS_LOWCOST       0x02
    #endif
        u_int16_t ip_len;         /* total length */
        u_int16_t ip_id;          /* identification */
        u_int16_t ip_off;
    #ifndef IP_RF
    #define IP_RF 0x8000        /* reserved fragment flag */
    #endif
    #ifndef IP_DF
    #define IP_DF 0x4000        /* dont fragment flag */
    #endif
    #ifndef IP_MF
    #define IP_MF 0x2000        /* more fragments flag */
    #endif 
    #ifndef IP_OFFMASK
    #define IP_OFFMASK 0x1fff   /* mask for fragmenting bits */
    #endif
        u_int8_t ip_ttl;          /* time to live */
        u_int8_t ip_p;            /* protocol */
        u_int16_t ip_sum;         /* checksum */
        struct in_addr ip_src, ip_dst; /* source and dest address */
};

struct libnet_tcp_hdr {
        u_int16_t th_sport;       /* source port */
        u_int16_t th_dport;       /* destination port */
        u_int32_t th_seq;          /* sequence number */
        u_int32_t th_ack;          /* acknowledgement number */
    #if (LIBNET_LIL_ENDIAN)
        u_int8_t th_x2:4,         /* (unused) */
               th_off:4;        /* data offset */
    #endif
    #if (LIBNET_BIG_ENDIAN)
        u_int8_t th_off:4,        /* data offset */
               th_x2:4;         /* (unused) */
    #endif
        u_int8_t  th_flags;       /* control flags */
    #ifndef TH_FIN
    #define TH_FIN    0x01      /* finished send data */
    #endif
    #ifndef TH_SYN
    #define TH_SYN    0x02      /* synchronize sequence numbers */
    #endif
    #ifndef TH_RST
    #define TH_RST    0x04      /* reset the connection */
    #endif
    #ifndef TH_PUSH
    #define TH_PUSH   0x08      /* push data to the app layer */
    #endif
    #ifndef TH_ACK
    #define TH_ACK    0x10      /* acknowledge */
    #endif
    #ifndef TH_URG
    #define TH_URG    0x20      /* urgent! */
    #endif
    #ifndef TH_ECE
    #define TH_ECE    0x40
    #endif
    #ifndef TH_CWR
    #define TH_CWR    0x80
    #endif
        u_int16_t th_win;         /* window */
        u_int16_t th_sum;         /* checksum */
        u_int16_t th_urp;         /* urgent pointer */
};

bool parse(Param* param, int argc, char* argv[]) {
	if (argc != 2) {
		usage();
		return false;
	}
	param->dev_ = argv[1];
	return true;
}


int main(int argc, char* argv[]) {
	if (!parse(&param, argc, argv))
		return -1;

	char errbuf[PCAP_ERRBUF_SIZE];
	pcap_t* pcap = pcap_open_live(param.dev_, BUFSIZ, 1, 1000, errbuf);
	if (pcap == NULL) {
		fprintf(stderr, "pcap_open_live(%s) return null - %s\n", param.dev_, errbuf);
		return -1;
	}

	while (true) {
		struct pcap_pkthdr* header;
		struct libnet_ethernet_hdr* ether;
		struct libnet_ipv4_hdr* ip;
		struct libnet_tcp_hdr* tcp;
		char* payload;
		int pay_len;
		const u_char*packet;

		int res = pcap_next_ex(pcap, &header, &packet);
		if (res == 0) continue;
		if (res == PCAP_ERROR || res == PCAP_ERROR_BREAK) {
			printf("pcap_next_ex return %d(%s)\n", res, pcap_geterr(pcap));
			break;
		}

		ether = (struct libnet_ethernet_hdr*)(packet);

		if(ntohs(ether->ether_type) == 0x0800){
    			ip = (struct libnet_ipv4_hdr*)(packet + 14);
    			tcp = (struct libnet_tcp_hdr*)(packet + 34);
    			payload = (u_char *)(struct libnet_ethernet_hdr*)(packet + 65);
			pay_len = strlen(payload);

    			if(ip->ip_p == 6){
				printf("=========Best==of==the==Best===11th=========\n");
				printf("--------------------PCAP--------------------\n");
			        printf("%u bytes captured\n", header->caplen);

			        printf("\n[Departure] Mac Address : ");
				for(int j=0; j<ETHER_ADDR_LEN; j++){
				        printf("%02x", ether->ether_shost[j]);
				}
				printf("\n[Arrival] Mac Address : ");
				for(int k=0; k<ETHER_ADDR_LEN; k++){
					printf("%02x", ether->ether_dhost[k]);
				}

    				printf("\n[Departure] IP Address : %s\n", inet_ntoa(ip->ip_src));
    				printf("[Arrival] IP Address : %s\n", inet_ntoa(ip->ip_dst));
				printf("[Departure] Port : %d\n", ntohs(tcp->th_sport));
    				printf("[Arrival] Port : %d\n", ntohs(tcp->th_dport));
				
				if(pay_len > 10){
					printf("[Hex] Payload : ");
	    				for(int l=1; l<=10; l++){
	        				printf("%x ", payload[l]);
	    				}
					printf("\n");
					printf("[PlaneText] Payload : ");
					for(int o=1; o<=10; o++){
						printf("%c ", payload[o]);
					}
				}
				else{
					printf("[Hex] Payload : Empty! \n");
					printf("[PlaneText] Payload : Empty! \n");
				}
				printf("\n--------------------------------------------\n\n");
				
    			}
		}
	}
	pcap_close(pcap);
}
