// ENC28J60 -  STM32F103
//   VCC    -    3.3V
//   GND    -    GND
//   SCK    -    Pin PA5
//   SO     -    Pin PA6
//   SI     -    Pin PA7
//   CS     -    Pin PA8

//   DHT11  -    Pin PC13


#include <SPI.h>
#include <EtherCard_STM.h>
#include <SimpleDHT.h>

#define LONG_DELAY 600000
#define DELAY 30000

int pinDHT11 = PC13;
SimpleDHT11 dht11;
// ethernet interface mac address, must be unique on the LAN
static byte mymac[] = { 0x74, 0x69, 0x69, 0x2D, 0x30, 0x31 };

byte Ethernet::buffer[700];
static uint32_t timer = LONG_DELAY;
static uint32_t timerh;
static uint32_t humc;
static uint32_t tempc;
static uint32_t count;

const char website[] PROGMEM = "willard.tk";

// called when the client request is complete
static void my_callback (byte status, uint16_t off, uint16_t len) {
  Serial.println(">>>");
  Ethernet::buffer[off + 300] = 0;
  Serial.print((const char*) Ethernet::buffer + off);
  Serial.println("...");
}

void setup () {
  Serial.begin(57600);
  Serial.println(F("\n[webClient]"));

  if (ether.begin(sizeof Ethernet::buffer, mymac) == 0)
    Serial.println(F("Failed to access Ethernet controller"));
  if (!ether.dhcpSetup())
    Serial.println(F("DHCP failed"));

  ether.printIp("IP:  ", ether.myip);
  ether.printIp("GW:  ", ether.gwip);
  ether.printIp("DNS: ", ether.dnsip);

  if (!ether.dnsLookup(website))
    Serial.println("DNS failed");

  ether.printIp("SRV: ", ether.hisip);
}

void loop () {
  ether.packetLoop(ether.packetReceive());
  if (millis() > timerh) {
    timerh = millis() + DELAY;
    byte temperature = 0;
    byte humidity = 0;
    if (dht11.read(pinDHT11, &temperature, &humidity, NULL)) {
      Serial.print("Read DHT11 failed.");
    }else{
       Serial.print((int)temperature); Serial.print(" *C, "); 
      Serial.print((int)humidity); Serial.println(" %");
      count++;
      humc+=(int)humidity;
      tempc+=(int)temperature;
    }
  }

  if (millis() > timer&&count > 0) {
    timer = millis() + LONG_DELAY;
    Serial.println();
    String cs = "rec.php?hum=";
    cs += String((humc/(double) count),2);
    cs += "&temp=";
    cs += String((tempc/(double) count),2);
    Serial.println(cs);
    Serial.print("<<< REQ ");
   
    char val[32] PROGMEM;
    cs.toCharArray(val, 32);
    count=0;
    humc=0;
    tempc=0;
    ether.browseUrl("/", val, website, my_callback);
  }
}
