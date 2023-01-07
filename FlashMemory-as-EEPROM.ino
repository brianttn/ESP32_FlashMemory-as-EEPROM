#include <EEPROM.h>

/* - - - - - - - - - - -     Flash as EEPROM Setting     - - - - - - - - - - - */
#define EEPROM_SIZE 17

#define uvTime_Addr 0
#define uvTime_SIZE 2

#define rfAddrID_Addr 2
#define rfAddrID_SIZE 3

#define fanGear_Addr 5
#define fanGear_SIZE 12

uint8_t flashWrBuf[12];
uint8_t flashRdBuf[12];

typedef struct{
    uint8_t uvEnable;           // UV燈：致能Flag
    uint8_t uvCycleTime;        // Default:60, Max value:180 (Rpi Command)(Unit:Min)
    uint8_t uvOnTime;           // Default:3, Max value:50 (Rpi Command)(Unit:Min)
}uvParams_t;
volatile uvParams_t uvParams = {0, 0, 0};

volatile uint32_t rfAddr = 0;

typedef struct{
    uint16_t Gear1;
    uint16_t Gear2;
    uint16_t Gear3;
    uint16_t Gear4;
    uint16_t Gear5;
    uint16_t Turbo;
}fanGear_t;
volatile fanGear_t fanGears = {2200, 2400, 2600, 2800, 3000, 3500};		// 初次開機「預設值」

/* - - - - - - - - - - -     Read data from EEPROM     - - - - - - - - - - - */
void readEEPROM(uint8_t flashAddr, uint8_t flashSize)
{
    /* - - - - - - - -   「讀取」EEPROM內存資料   - - - - - - - - */
    for(int i = 0; i < flashSize; i++)
        flashRdBuf[i] = uint8_t(EEPROM.read(flashAddr+i));

    // MSB值為0xFF => EEPROM不是「未曾寫入」，就是「資料寫入錯誤(負值)」
    if(flashRdBuf[0]==0xFF || ((flashRdBuf[0]==0 || flashRdBuf[1]==0) && (flashAddr==uvTime_Addr || flashAddr==fanGear_Addr)))
    {
        if(flashAddr == uvTime_Addr)
        {
            Serial.println("Initialize EEPROM UVTime section.");
            uvParams.uvCycleTime = 60;
            uvParams.uvOnTime = 3;
            writeEEPROM(uvTime_Addr, uvTime_SIZE);
            uvParams.uvEnable = 1;      // UV燈：Enable
        }
        else if(flashAddr == rfAddrID_Addr)
        {
            Serial.println("需要進行遙控器Address ID Scan!!");
        }
        else if(flashAddr == fanGear_Addr)
        {
            Serial.println("寫入6段轉速初始值");
            writeEEPROM(fanGear_Addr, fanGear_SIZE);
	    }
    }
    else if(flashAddr == uvTime_Addr)
    {
        Serial.println("Reading UVTime data from EEPROM.");

        /* - - - - - - - -   「解析」從EEPROM讀出的資料   - - - - - - - - */
        for(int i = 0; i<flashSize; i++)		// 2Bytes
        {
            if(i == 0)
                uvParams.uvCycleTime = flashRdBuf[i];
            else if(i == 1)
                uvParams.uvOnTime = flashRdBuf[i];
        }

        if(uvParams.uvOnTime != 0)
            uvParams.uvEnable = 1;      // UV燈：Enable
        else
            uvParams.uvEnable = 0;      // UV燈：Disable

        Serial.printf("uvCycleTime: %d\n", uvParams.uvCycleTime);
        Serial.printf("uvOnTime: %d\n\n", uvParams.uvOnTime);
    }
    else if(flashAddr == rfAddrID_Addr)
    {
        Serial.println("Reading remoteRfAddrID data from EEPROM.");

        for(int i = 0; i<flashSize; i++)		// 3Bytes
        {
            rfAddr += flashRdBuf[i] * pow(2, 8*(2-i));
        }
        Serial.printf("remoteRfAddrID: %d\n\n", rfAddr);
    }
    else if(flashAddr == fanGear_Addr)
    {
        Serial.println("Reading fanGears data from EEPROM.");

    	fanGears.Gear1 = flashRdBuf[0]*256 + flashRdBuf[1];		// 12Bytes
    	fanGears.Gear2 = flashRdBuf[2]*256 + flashRdBuf[3];
    	fanGears.Gear3 = flashRdBuf[4]*256 + flashRdBuf[5];
    	fanGears.Gear4 = flashRdBuf[6]*256 + flashRdBuf[7];
    	fanGears.Gear5 = flashRdBuf[8]*256 + flashRdBuf[9];
    	fanGears.Turbo = flashRdBuf[10]*256 + flashRdBuf[11];

    	Serial.printf("Gear1: %d\n", fanGears.Gear1);
    	Serial.printf("Gear2: %d\n", fanGears.Gear2);
    	Serial.printf("Gear3: %d\n", fanGears.Gear3);
    	Serial.printf("Gear4: %d\n", fanGears.Gear4);
    	Serial.printf("Gear5: %d\n", fanGears.Gear5);
    	Serial.printf("Turbo: %d\n", fanGears.Turbo);
    }

    /* - - - - - - - -   清除EEPROM「讀取」暫存器   - - - - - - - - */
    for(int i = 0; i<12; i++)
        flashRdBuf[i] = 0;
}

/* - - - - - - - - - - -     Write data to EEPROM     - - - - - - - - - - - */
void writeEEPROM(uint8_t flashAddr, uint8_t flashSize)
{
    Serial.println();		// 空一行

    /* - - - 將資料轉換成「可寫入EEPROM的格式」，並存入「寫入暫存陣列:flashWrBuf」 - - - */
    if(flashAddr == uvTime_Addr)
    {
        Serial.println("Writing UVTime data to EEPROM.");

        for(int i = 0; i<flashSize; i++)		// 2Bytes
        {
            if(i == 0)
                flashWrBuf[i] = uvParams.uvCycleTime;
            else if(i == 1)
                flashWrBuf[i] = uvParams.uvOnTime;
	    }
    }
    else if(flashAddr == rfAddrID_Addr)
    {
        Serial.println("Writing remoteRfAddrID data to EEPROM.");

        for(int i = 0; i<flashSize; i++)
            flashWrBuf[i] = (rfAddr >> (8*(2-i))) % 256;        // 3Bytes
    }
    else if(flashAddr == fanGear_Addr)
    {
        Serial.println("Writing fanGears data to EEPROM.");

    	flashWrBuf[0] = fanGears.Gear1 / 256;       // 12Bytes
    	flashWrBuf[1] = fanGears.Gear1 % 256;
    	flashWrBuf[2] = fanGears.Gear2 / 256;
    	flashWrBuf[3] = fanGears.Gear2 % 256;
    	flashWrBuf[4] = fanGears.Gear3 / 256;
    	flashWrBuf[5] = fanGears.Gear3 % 256;
    	flashWrBuf[6] = fanGears.Gear4 / 256;
    	flashWrBuf[7] = fanGears.Gear4 % 256;
    	flashWrBuf[8] = fanGears.Gear5 / 256;
    	flashWrBuf[9] = fanGears.Gear5 % 256;
    	flashWrBuf[10] = fanGears.Turbo / 256;
    	flashWrBuf[11] = fanGears.Turbo % 256;
    }

    /* - - - - - - - -   「寫入」EEPROM:flashAddr區段   - - - - - - - - */
    for(int i = 0; i < flashSize; i++)
    {
        EEPROM.write(flashAddr+i, flashWrBuf[i]);
        Serial.print("Wrote: ");
        Serial.println(flashWrBuf[i]);
    }

    EEPROM.commit();        // 一次寫入

    /* - - - - - - - -   清除EEPROM「寫入」暫存器   - - - - - - - - */
    for(int i = 0; i<12; i++)
        flashWrBuf[i] = 0;
}

/* - - - - - - - - - - -     Execution Environment Setting     - - - - - - - - - - - */
void setup()
{
    /* - - - - - - - -   Flash(EEPROM) Initialization   - - - - - - - - */
    EEPROM.begin(EEPROM_SIZE);                      // 宣告要使用的EEPROM大小:EEPROM_SIZE
    readEEPROM(uvTime_Addr, uvTime_SIZE);           // 從Flash讀出uvCycleTime, uvOnTime數值
    readEEPROM(rfAddrID_Addr, rfAddrID_SIZE);       // 從Flash讀出remoteRfAddrID數值
    readEEPROM(fanGear_Addr, fanGear_SIZE);         // 從Flash讀出fanGears數值
}

void loop(){}
