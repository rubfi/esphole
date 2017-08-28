#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <FS.h>

#include "DNSServer.h"

#define wifi_ssid "........."
#define wifi_password ".........."

const byte DNS_PORT = 53;
DNSServer dnsServer;
ESP8266WebServer webServer(80);
File f;

void setup_wifi() {
  delay(10);

  Serial.println();
  Serial.print("Connecting to: ");
  Serial.println(wifi_ssid);

  WiFi.begin(wifi_ssid, wifi_password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("WiFi connected | IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(9600);
 
  WiFi.mode(WIFI_AP_STA);

  setup_wifi();
 
  SPIFFS.begin();

  dnsServer.setErrorReplyCode(DNSReplyCode::ServerFailure);
  
  bool dns_running = dnsServer.start(DNS_PORT, "*", WiFi.localIP());
  if (dns_running)
  {
    Serial.println("DNS Server ready"); 
  }
  else
  {
    Serial.println("Error: DNS Server not running");     
  }

}

int find_text(String needle, String haystack) {
  int foundpos = -1;
  for (int i = 0; i <= haystack.length() - needle.length(); i++) {
    if (haystack.substring(i,needle.length()+i) == needle) {
      foundpos = i;
    }
  }
  return foundpos;
}

void loop() {
  
  int dnsOK = dnsServer.processNextRequest();
  if (dnsOK == 0)
  {
    String dom = dnsServer.getQueryDomainName();
    f.setTimeout(5000);
      
    if ((dom != ""))
    {
      
      Serial.println ();
      Serial.print ("Domain: ");
      Serial.print (dom);
       
      char str[12];
      sprintf(str, "/hosts_%d", dom.length());

      f = SPIFFS.open(str, "r");
      if (!f) {
          Serial.printf("\nError: file open failed\n");
      }
     
      f.seek(0, SeekSet);
       
      uint32_t oMillis = millis();
      
      char dom_str[dom.length()+2];
      sprintf(dom_str, ",%s,", dom.c_str());
      
      bool found = f.findUntil(dom_str,"@@@");
      
      uint32_t findMs = millis() - oMillis;
      
      if (found)
      {
        Serial.printf(" Blocked | Find took %lu ms\n", findMs );
        dnsServer.replyWithIP(IPAddress(0, 0, 0, 0));
      }
      else
      {
        IPAddress ip;
        uint32_t oldMillis = millis();
        WiFi.hostByName(dom.c_str(), ip);
   
        dnsServer.replyWithIP(ip);
        
        uint32_t resolvMs = millis() - oMillis;
        
        Serial.print(" | IP:");
        Serial.print(ip);
        Serial.printf("\nResolv took %lu ms", resolvMs );
        Serial.printf(" | Find took %lu ms\n", findMs );
      }
      dom = "";
    }
  }
}
