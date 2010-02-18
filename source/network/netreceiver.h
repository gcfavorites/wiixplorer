#ifndef _NETRECEIVER_H_
#define _NETRECEIVER_H_

class NetReceiver
{
    public:
        NetReceiver();
        ~NetReceiver();
        bool CheckIncomming();
        void CloseConnection();
        const u8 * ReceiveData();
        const u8 * GetData();
        u32 GetFilesize();
        const char * GetFilename() { return (const char *) &FileName; };
        const char * GetIncommingIP() { return (const char *) &incommingIP; };
        void FreeData();
    protected:
        const u8 * UncompressData();

        int connection;
        int socket;
        u8 * filebuffer;
        u32 filesize;
        u32 uncfilesize;
        char incommingIP[20];
        char FileName[50];
        char wiiloadVersion[2];
};


void IncommingConnection(NetReceiver & Receiver);

#endif
