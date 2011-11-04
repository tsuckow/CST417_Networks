/* Import external functions from Serial.c file                               */
void init_serial    (void);

extern "C" int sendchar (int ch);

extern "C" int getkey (void);
