#include <WaziDev.h>
#include <xlpp.h>
// 2E 1B EC C7 0D BC B9 C1 8F 50 E3 CA D5 D3 B0 AF
unsigned char loRaWANKey[16] = {0x2E, 0x1B, 0xEC, 0xC7, 0x0D, 0xBC, 0xB9, 0xC1, 0x8F,
                                0x50, 0xE3, 0xCA, 0xD5, 0xD3, 0xB0, 0xAF};

                                // B9C18F50
unsigned char devAddr[4] = {0xB9, 0xC1, 0x8F, 0x50};

WaziDev wazidev;

/************ Add your code here **********/
int proxSwitch = 5;
int state = LOW;
int value;
/******************************************/


/************ Add your code here **********/
int readProxSwitch(void)
{
  value = digitalRead(proxSwitch);
  if (value != state) {
    state = value;
    Serial.print("sensor value = ");
    if (state == 0) {
      Serial.println("target detected");
      return (1);
    }

    else {
      Serial.println("No target detected");
      return (0);
    }
  }
}
/******************************************/

void setup()
{
  Serial.begin(9600);
  wazidev.setupLoRaWAN(devAddr, loRaWANKey);

  /************ Add your code here **********/
  pinMode(proxSwitch, INPUT);
  /******************************************/

}

XLPP xlpp(120);

void loop()
{
  uint8_t err;

  err = uplink();

  if (!err)
  {
    downlink(6000);
  }

  Serial.println("waiting 1 min...");
  delay(10000);
}

int8_t uplink()
{
  uint8_t err;

  /************ Add your code here **********/
  // 1.
  // Read sensor values.
  int proximity = readProxSwitch();

  // 2.
  // Create xlpp payload for uplink.
  xlpp.reset();
  
  // Add sensor payload
  xlpp.addSwitch(0, proximity);
  /******************************************/

  // 3.
  // Send payload uplink with LoRaWAN.
  serialPrintf("LoRaWAN send ... ");
  err = wazidev.sendLoRaWAN(xlpp.buf, xlpp.len);
  if (err != 0)
  {
    serialPrintf("Err %d\n", err);
    return err;
  }
  serialPrintf("OK\n");
  return 0;
}

uint8_t downlink(uint16_t timeout)
{
  uint8_t err;

  // 1.
  // Receive LoRaWAN downlink message.
  serialPrintf("LoRa receive ... ");
  uint8_t offs = 0;
  long startSend = millis();
  err = wazidev.receiveLoRaWAN(xlpp.buf, &xlpp.offset, &xlpp.len, timeout);
  long endSend = millis();
  if (err)
  {
    if (err == ERR_LORA_TIMEOUT)
      serialPrintf("nothing received\n");
    else 
      serialPrintf("Err %d\n", err);
    return err;
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