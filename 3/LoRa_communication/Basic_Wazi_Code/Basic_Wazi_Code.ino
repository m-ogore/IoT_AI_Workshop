#include <WaziDev.h>
#include <xlpp.h>

unsigned char loRaWANKey[16] = {0x23, 0x15, 0x8D, 0x3B, 0xBC, 0x31, 0xE6, 0xAF,
                                0x67, 0x0D, 0x19, 0x5B, 0x5A, 0xED, 0x55, 0x25};
unsigned char devAddr[4] = {0x26, 0x01, 0x1D, 0x89};

WaziDev wazidev;


void setup()
{
  Serial.begin(9600);
  wazidev.setupLoRaWAN(devAddr, loRaWANKey);

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

  // 1.
  // Read sensor values.
  int btn1 = 0;
  int btn2 = 0;
  int btn3 = 0;

  // 2.
  // Create xlpp payload for uplink.
  xlpp.reset();
  
  // Add sensor payload
  xlpp.addSwitch(0, btn1);
  xlpp.addSwitch(1, btn2);
  xlpp.addSwitch(2, btn3);

  Serial.print("Button1 val: ");
  Serial.print(btn1);
  Serial.print(" | Button2 val: ");
  Serial.print(btn2);
  Serial.print(" | Button3 val: ");
  Serial.println(btn3);
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
  
  // serialPrintf("Time On Air: %d ms\n", endSend-startSend);
  // serialPrintf("LoRa SNR: %d\n", wazidev.loRaSNR);
  // serialPrintf("LoRa RSSI: %d\n", wazidev.loRaRSSI);
  // serialPrintf("LoRaWAN frame size: %d\n", xlpp.offset+xlpp.len);
  // serialPrintf("LoRaWAN payload len: %d\n", xlpp.len);
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