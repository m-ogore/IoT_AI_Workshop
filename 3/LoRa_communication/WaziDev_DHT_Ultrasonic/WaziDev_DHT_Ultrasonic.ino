#include <WaziDev.h>
#include <xlpp.h>
#include <DHT.h>

/*************** Add Your Code Here ****************/
#define ECHO_PIN 4  
#define TRIG_PIN 3

#define DHT_PIN 2

DHT dht(DHT_PIN, DHT22);
/**************************************************/

// LoRaWANKey is used as both NwkSKey (Network Session Key) and
// Appkey (AppKey) for secure LoRaWAN transmission.
// Copy'n'paste the key to your Wazigate:
unsigned char loRaWANKey[16] = {0x23, 0x15, 0x8D, 0x3B, 0xBC, 0x31, 0xE6, 0xAF,
                               0x67, 0x0D, 0x19, 0x5B, 0x5A, 0xED, 0x55, 0x25};
// Copy'n'paste the DevAddr (Device Address): 26011D89
unsigned char devAddr[4] = {0x26, 0x01, 0x1D, 0x89};
// You can change the Key and DevAddr as you want.


WaziDev wazidev;

/*************** Add Your Code Here ****************/
float distance_centimetre() {
  // Send sound pulse
  digitalWrite(TRIG_PIN, HIGH); // pulse started
  delayMicroseconds(12);
  digitalWrite(TRIG_PIN, LOW); // pulse stopped

  // listen for echo 
  float tUs = pulseIn(ECHO_PIN, HIGH); // microseconds
  float distance = tUs / 58; // cm 
  return distance;
}
/**************************************************/

void setup()
{
  Serial.begin(115200);
  wazidev.setupLoRaWAN(devAddr, loRaWANKey);

  pinMode(TRIG_PIN, OUTPUT);
  digitalWrite(TRIG_PIN, LOW);
  pinMode(ECHO_PIN, INPUT);
 
  dht.begin(); // start DHT sensor
  delay(2000);
}

XLPP xlpp(120);

uint8_t uplink()
{
  uint8_t e;

  /*************** Add Your Code Here ****************/
  // 1.
  // Read sensor values.
  float humidity = dht.readHumidity(); // %
  float temperature = dht.readTemperature(); // °C
  float distance = distance_centimetre();
  // 2.
  // Create xlpp payload for uplink.
  xlpp.reset();
  
  // Add sensor payload
  xlpp.addRelativeHumidity(0, humidity);
  xlpp.addTemperature(1, temperature);
  xlpp.addDistance(2, distance);

  Serial.print("Temp: ");
  Serial.print(temperature);
  Serial.print("C, Humid: ");
  Serial.print(humidity);
  Serial.print("%, Dist: ");
  Serial.print(distance);
  Serial.println("cm.");
  /***************************************************/
  
  // 3.
  // Send payload uplink with LoRaWAN.
  serialPrintf("LoRaWAN send ... ");
  e = wazidev.sendLoRaWAN(xlpp.buf, xlpp.len);
  if (e != 0)
  {
    serialPrintf("Err %d\n", e);
    return e;
  }
  serialPrintf("OK\n");
  return 0;
}

uint8_t downlink(uint16_t timeout)
{
  uint8_t e;

  // 1.
  // Receive LoRaWAN downlink message.
  serialPrintf("LoRa receive ... ");
  uint8_t offs = 0;
  long startSend = millis();
  e = wazidev.receiveLoRaWAN(xlpp.buf, &xlpp.offset, &xlpp.len, timeout);
  long endSend = millis();
  if (e)
  {
    if (e == ERR_LORA_TIMEOUT)
      serialPrintf("nothing received\n");
    else 
      serialPrintf("Err %d\n", e);
    return e;
  }
  serialPrintf("OK\n");
  
  serialPrintf("Time On Air: %d ms\n", endSend-startSend);
  serialPrintf("LoRa SNR: %d\n", wazidev.loRaSNR);
  serialPrintf("LoRa RSSI: %d\n", wazidev.loRaRSSI);
  serialPrintf("LoRaWAN frame size: %d\n", xlpp.offset+xlpp.len);
  serialPrintf("LoRaWAN payload len: %d\n", xlpp.len);
  serialPrintf("Payload: ");
  if (xlpp.len == 0)
  {
    serialPrintf("(no payload received)\n");
    return 1;
  }
  printBase64(xlpp.getBuffer(), xlpp.len);
  serialPrintf("\n");

  // 2.
  // Read xlpp payload from downlink message.
  // You must use the following pattern to properly parse xlpp payload!
  int end = xlpp.len + xlpp.offset;
  while (xlpp.offset < end)
  {
    // [1] Always read the channel first ...
    uint8_t chan = xlpp.getChannel();
    serialPrintf("Chan %2d: ", chan);

    // [2] ... then the type ...
    uint8_t type = xlpp.getType();

  }
}

void loop(void)
{
  // error indicator
  uint8_t e;

  // 1. LoRaWAN Uplink
  e = uplink();
  // if no error...
  if (!e) {
    // 2. LoRaWAN Downlink
    // waiting for 6 seconds only!
    downlink(6000);
  }

  serialPrintf("Waiting 1min ...\n");
  delay(60000);
}
