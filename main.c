#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <linux/if_arp.h>
#include <pthread.h>
#include <stdint.h>
#include "arp_custom.h"

int sock;
char * cardInterface;
struct sockaddr sa;

void * peticionARP(void * data);

char * convertIpBytes(char * ip);

int main(){

    int cantIp = 0;
    int optval = 0;
    void * valor_retorno;
    cardInterface = (char *) calloc(100, sizeof(char));

    printf("\nInserta el nombre de tu tarjeta de red\n");
    scanf("%s", cardInterface);

    printf("\nInserta el número de IPs a insertar\n");
    scanf("%i", &cantIp);

    pthread_t anlzr[cantIp];
    char * ips[cantIp];
    fflush(stdin);
    
    for(int i = 0; i < cantIp; i++){
        printf("\nColoca la dirección para la IP %i\n", i+1);
        ips[i] = calloc(17, sizeof(char));
        scanf("%s", ips[i]);
        
    }
    
    sock = socket(PF_INET, SOCK_PACKET, htons(ETH_P_ARP));
    setsockopt(sock, SOL_SOCKET, SO_BROADCAST, &optval, sizeof(optval));
    
    for(int i = 0; i < cantIp; i++){
        if(pthread_create(&anlzr[i], NULL, peticionARP, (void * )ips[i])){
            printf("\nProblemas creando el hilo del analizador\n");
            exit(EXIT_FAILURE);
        };
    }

    for(int i = 0; i < cantIp; i++){ 
        if(pthread_join(anlzr[i], &valor_retorno)){
            printf("\nProblemas creando el enlace\n");
            exit(EXIT_FAILURE);
        };
        free(ips[i]);
    }

    //ARPOP_REPLY

    free(cardInterface);
    

    return 0;
}

void * peticionARP(void * data){
    
    int counter = 10;

    DataArp dataArp, dataArpRes;

    char * ip = (char * ) data;

    struct ifreq ifr;
    struct ifreq ethreq;
    ioctl(sock, SIOCGIFFLAGS, &ethreq);
    
    bcopy(&ifr.ifr_hwaddr.sa_data, &dataArp.origenMAC, 6);
    bcopy(&ifr.ifr_hwaddr.sa_data, &dataArp.origenEthernet, 6);

    strcpy(ifr.ifr_name, cardInterface);
    ioctl(sock, SIOCGIFADDR, &ifr);
    bcopy(&ifr.ifr_addr.sa_data[2], &dataArp.origenIP, 4);

    memset(&dataArp.destinoEthernet, 0xFF, 6);
    dataArp.tipoEthernet = htons(ETH_P_ARP);
    dataArp.tipoHardware = htons(ARPHRD_ETHER);
    dataArp.tipoProtocolo = htons(ETH_P_IP);
    dataArp.longitudProtocolo = 4;
    dataArp.longitudHardware = 6;
    dataArp.tipoMensaje = htons(ARPOP_REQUEST);

    char * ipBytes = convertIpBytes(ip);

    bzero(&dataArp.destinoMAC, 6);
    for(int i = 0; i < 4; i++ ){
        dataArp.destinoIP[i] = ipBytes[i];
    }

    do{
        sendto(sock, &dataArp, sizeof(dataArp), 0, (struct sockaddr * ) &sa, sizeof(sa));
        do{
            recvfrom(sock, &dataArpRes, 42, 0, NULL, NULL);
        }
        while(htons(dataArpRes.tipoMensaje) == 1);
        counter++;
    }
    while((dataArpRes.origenIP[0] != dataArp.destinoIP[0] || dataArpRes.origenIP[1] != dataArp.destinoIP[1]
    || dataArpRes.origenIP[2] != dataArp.destinoIP[2] || dataArpRes.origenIP[3] != dataArp.destinoIP[3]) && counter < 10);

    
    if(counter < 10){
        printf("\nLa dirección MAC para la IP %i.%i.%i.%i\n", ip[0], ip[1], ip[2], ip[3]);
        for(int i = 0; i < 6; i++){
            printf("%2X", dataArpRes.origenMAC[i]);
            if(i != 5){
                printf(":");
            }
        }
    }
    else{
        printf("\nSe agotó el tiempo de espera, para la dirección %i.%i.%i.%i\n", ip[0], ip[1], ip[2], ip[3]);
    }

    
}

char * convertIpBytes(char * ip){

    unsigned char * ipBytes = (unsigned char * ) calloc(4, sizeof(unsigned char));
    int len = strlen(ip);
    int initial = 0;
    int j = 0;
    for(int i = 0; i < len; i++){
        
        if(ip[i] == '.' || i == len-1){
            
            unsigned char * subtext = (unsigned char *) calloc(3, sizeof(unsigned char));
            strncpy(subtext, &ip[initial], i == len-1 ? i+1:i-initial);
            ipBytes[j] = (unsigned char) atoi(subtext);
            initial = i+1;
            j++;
        }

    }

    return  ipBytes;

}