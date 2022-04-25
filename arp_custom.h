typedef struct DataArp{

    unsigned char destinoEthernet[6];
    unsigned char origenEthernet[6];
    unsigned short tipoEthernet;
    unsigned short tipoHardware;

    unsigned short tipoProtocolo;

    unsigned char longitudHardware;
    unsigned char longitudProtocolo;
    unsigned short tipoMensaje;
    unsigned char origenMAC[6];
    unsigned char origenIP[4];
    unsigned char destinoMAC[6];
    unsigned char destinoIP[4];

} DataArp;