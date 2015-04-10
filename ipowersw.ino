/*
 * Ethernet IP power switch with HTTP web interface and REST API. Includes temperature, humidity and ambient light metering.
 * Powered by Arduino + ENC28J60 module, relay, DHT11 sensor, photoresistor.
 * Read more at https://github.com/vinklat/ipowersw
 * by Vaclav Vinklat, 2015, GPLv2
 */
 
#include <UIPEthernet.h>
#include <DHT.h>

/* 
 * Setup your application here:
 */
 
#define DEBUG
#define PIN_RELAY 4
#define PIN_DHT 5
#define PIN_LIGHT 2

#define USE_DHCP
#define IP_ADDR 192,168,0,2
#define MAC_ADDR 0xDE,0xAD,0xBE,0xEF,0xFE,0xED

#define REQ_BUF_SIZE 5
#define LOC_BUF_SIZE 10

// Uncomment whatever type you're using!
#define DHTTYPE DHT11   // DHT 11 
//#define DHTTYPE DHT22   // DHT 22  (AM2302)
//#define DHTTYPE DHT21   // DHT 21 (AM2301)

/*
 *
 */

template<class T> inline Print &operator <<(Print &obj, T arg) { 
  obj.print(arg); return obj;
}

/*
 *
 */

EthernetServer server(80);
int relay_state;
#ifdef PIN_DHT
DHT dht(PIN_DHT, DHTTYPE);
#endif

/*
 *
 */

void setup() {
  #ifdef DEBUG
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  #endif

  const byte mac[] = { MAC_ADDR };
  
  // start the Ethernet connection:
  #ifdef USE_DHCP
  while (!Ethernet.begin(mac)) {
    #ifdef DEBUG
    Serial.println("Failed to configure Ethernet using DHCP");
    #endif
    delay(2);    
  }
  #else
  IPAddress ip(IP_ADDR);
  Ethernet.begin(mac, ip);
  #endif
  
  server.begin();
  #ifdef DEBUG
  Serial << "server is at " << Ethernet.localIP() << '\n';
  #endif
 
  // default relay pin state HIGH (=relay output LOW)
  pinMode(PIN_RELAY, OUTPUT);
  digitalWrite(PIN_RELAY, HIGH);
  relay_state=0;

  #ifdef PIN_DHT
  dht.begin();
  #endif
}


/*
 *
 */

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
            if (pos==LOC_BUF_SIZE-1 || c==' ' || c=='\n' || c=='\r')
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


/*
 *
 */

#define HTTP "HTTP/1.0 "
#define REPLY_200 "200 OK\n"
#define REPLY_400 F("400 Bad Request\n")
#define REPLY_404 F("404 Not Found\n")
#define API_CONTYPE F("Content-Type: application/json\n")
#define HTML_CONTYPE F("Content-Type: text/html\n")
#define CLOSE F("Connection: close\n")
#define NO_CACHE F("Cache-Control: no-cache, no-store, must-revalidate\nPragma: no-cache\nExpires: 0\n")
#define REDIRECT F("Refresh: 3; url=/")

#define HTML_HEAD F("<!DOCTYPE HTML><html><body>")
#define HTML_FOOT F("</body></html>")
#define HTML_FORM_ON F("<form action=\"/1\" method=\"POST\"><input type=\"submit\" value=\"Turn ON\"></form>")
#define HTML_FORM_OFF F("<form action=\"/0\" method=\"POST\"><input type=\"submit\" value=\"Turn OFF\"></form>")

#define LOC_ROOT 0
#define LOC_0 1
#define LOC_1 2
#define LOC_API_ROOT 3
#define LOC_API_0 4
#define LOC_API_1 5

void render_response(EthernetClient *client, int *req, char *loc_buf) {
  const char* const locations[] = {"/", "/0", "/1", "/api/", "/api/0", "/api/1" };
  bool found=false;

  int loc;  
  for (loc=0; loc<6; loc++) {
    if (!strcmp(loc_buf, locations[loc])) {
      found=true;
      break;
    }
  }
  
  if (!*req) {
    *client << HTTP << REPLY_400 << HTML_CONTYPE << CLOSE << '\n';
    *client << HTML_HEAD << REPLY_400 << HTML_FOOT;
    return;
  }
   
  if (found) 
  {
    if (*req==REQ_GET) {

      #ifdef PIN_DHT
      float h = dht.readHumidity();
      float t = dht.readTemperature();
      #endif
      #ifdef PIN_LIGHT
      int light  = analogRead(PIN_LIGHT);
      #endif
      
      switch (loc) {
        case LOC_ROOT:
          *client << HTTP << REPLY_200 << HTML_CONTYPE << NO_CACHE << CLOSE << '\n';
          *client << HTML_HEAD << "relay: " << relay_state << "<br/>";
          #ifdef PIN_DHT
          *client << "humidity: " << h << "%<br/>temperature: " << t << "&deg;C<br/>";
          #endif
          #ifdef PIN_LIGHT
          *client << "light: " << light << "<br/>";
          #endif
          if (relay_state)
            *client << HTML_FORM_OFF;
          else
            *client << HTML_FORM_ON;
          *client << HTML_FOOT;
          break;
        case LOC_API_ROOT:
          *client << HTTP << REPLY_200 << API_CONTYPE << CLOSE << '\n';
          *client << "{\"relay\": " << relay_state;
          #ifdef PIN_DHT
          *client << ", \"humidity\": " << h << ", \"temperature\": " << t;
          #ifdef PIN_LIGHT
          *client << ", \"light\": " << light;
          #endif
          #endif
          *client << "}";
          break;
        default:
          found=false;
      }
    }
    
    if (*req==REQ_POST) {
      switch (loc) {
        case LOC_API_0:
          digitalWrite(PIN_RELAY, HIGH);
          relay_state=0;
          *client << HTTP << REPLY_200 << API_CONTYPE << CLOSE << '\n';
          *client << "{\"relay\": " << relay_state << "}";
          break;
        case LOC_API_1:
          digitalWrite(PIN_RELAY, LOW);
          relay_state=1;
          *client << HTTP << REPLY_200 << API_CONTYPE << CLOSE << '\n';
          *client << "{\"relay\": " << relay_state << "}";
          break;
        case LOC_0:
          digitalWrite(PIN_RELAY, HIGH);
          relay_state=0;
          *client << HTTP << REPLY_200 << HTML_CONTYPE << NO_CACHE << REDIRECT << '\n';
//          *client << HTML_HEAD << "relay: " << relay_state << HTML_FORM_ON << HTML_FOOT;
          break;
        case LOC_1:
          digitalWrite(PIN_RELAY, LOW);
          relay_state=1;
          *client << HTTP << REPLY_200 << HTML_CONTYPE << NO_CACHE << REDIRECT << '\n';
//          *client << HTML_HEAD << "relay: " << relay_state << HTML_FORM_OFF << HTML_FOOT;
          break;
        default:
          found=false;
      }
    }
  }
  
  if (!found) {
    *client << HTTP << REPLY_404 << HTML_CONTYPE << CLOSE << '\n';
    *client << HTML_HEAD << REPLY_404 << HTML_FOOT;
    return;
  }
}

/*
 *
 */

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
    render_response(&client, &req, loc_buf);
    
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    
    #ifdef DEBUG
    Serial.println("client disconnected");
    #endif
  }
}

