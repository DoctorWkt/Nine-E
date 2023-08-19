void romputc(int ch);
int romgetputc(void);
int ch375init(void);
int readblock(unsigned char *buf, long lba);
int writeblock(unsigned char *buf, long lba);
void exit(int status);
void startbin(void);
