#include <UIPEthernet.h>
// The connection_data struct needs to be defined in an external file.
//#include <UIPServer.h>
//#include <UIPClient.h>

#define DEBUG
#define REQ_BUF_SIZE 5
#define LOC_BUF_SIZE 10
#define PIN_RELAY 4
#define PIN_DHT 5

#define IP_DHCP

byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

#ifndef IP_DHCP
IPAddress ip(10, 28, 17, 123);
#endif

template<class T> inline Print &operator <<(Print &obj, T arg) { 
  obj.print(arg); return obj;
}

// Initialize the Ethernet server library
// with the IP address and port you want to use
// (port 80 is default for HTTP):
EthernetServer server(80);

void setup() {
  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  #endif
  
  // start the Ethernet connection:
  #ifdef IP_DHCP
  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to configure Ethernet using DHCP");
    // no point in carrying on, so do nothing forevermore:
    for (;;)
      ;
  #else
  Ethernet.begin(mac, ip);
  #endif
  }
  
  server.begin();
  #ifdef DEBUG
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
  #endif
 
  // default relay pin state HIGH (=relay output LOW)
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, HIGH);
}


#define REQ_ERR 0
#define REQ_GET 1
#define REQ_POST 2

void get_req(EthernetClient *client, int *req, char *loc_buf) {
    #define STATE_FIND_REQ 0
    #define STATE_GET_LOC 1
    #define STATE_SKIP_LINE 2
    #define STATE_SKIP_ALL 3
    #define STATE_GET_BODY 4
    
    int pos=0;
    int state=STATE_FIND_REQ;
    char req_buf[REQ_BUF_SIZE];
    *req=REQ_ERR;
    
    while (client->connected()) {
      if (client->available()) {
        char c = client->read();
        /*
        #ifdef DEBUG
        Serial << "ch='" << c << "'(" << (byte)c << ") state=" << state << '\n';
        #endif
        */
        switch (state) {

          case STATE_FIND_REQ: 
            if (pos==REQ_BUF_SIZE-1 || c==' ') 
              c='\0';
            *(req_buf+pos) = c;
            if (c=='\0') {
              if (!strcmp(req_buf,"GET")) {
                *req=REQ_GET;
                state=STATE_GET_LOC;
                pos=0;
              }
              else 
                if (!strcmp(req_buf,"POST")) {
                  *req=REQ_POST;
                  state=STATE_GET_LOC;
                  pos=0;
                }
                else {
                  state = STATE_SKIP_LINE;
                }
              }
              else
                pos++;
          break;

          case STATE_SKIP_LINE:
            if (c=='\n') {
              state=STATE_FIND_REQ;
              pos=0;
            }
          break;

          case STATE_GET_LOC:
            if (pos==LOC_BUF_SIZE-1 || c==' ')
              c='\0';
            *(loc_buf+pos) = c;
            if (c=='\0') {
              state = STATE_SKIP_ALL;
            }
            else
            {
              pos++;
            }
          break;
           
          case STATE_SKIP_ALL:
            if (c=='\n') {
              if (!pos)
                state=STATE_GET_BODY;
              pos=0;
            }
            else
              if (c != 13)
                pos++;
          break;
          
          case STATE_GET_BODY:
            //not implemented yet
          break;
          
        }
      }
      else
        break;        
    }  
}



void loop() {
 char loc_buf[LOC_BUF_SIZE];
 int req;
  // listen for incoming clients
 EthernetClient client = server.available();
  if (client) {
    #ifdef DEBUG
    Serial << "new client...\n";
    #endif

    get_req(&client, &req, loc_buf);

    if (req)
    {
      switch (req) {
        case REQ_GET:
          client << "request: " << req << ' ' << loc_buf << '\n';
        break;
        case REQ_POST:
          if (!strcmp(loc_buf,"/1"))
            digitalWrite(PIN_RELAY, LOW);
          else
            if (!strcmp(loc_buf,"/0"))
              digitalWrite(PIN_RELAY, HIGH);
              //todo else 404
        break;             
      }
    }     
    //todo else 404
    
    // give the web browser time to receive the data
    delay(2);
    // close the connection:
    client.stop();
    
    #ifdef DEBUG
    Serial.println("client disconnected");
    #endif
  }
}

