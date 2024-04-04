#include <DNSServer.h>
#include <ESPUI.h>
#include <WiFi.h>
#include <Arduino.h>
#include <time.h>
#include <LittleFS.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);
DNSServer dnsServer;



const char* ssid = "Snozzberries";
const char* password = "pinkpotato003";

const char* hostname = "espui";

int statusLabelId;
int millisLabelId;
int testSwitchId;
uint16_t alarmID;
uint16_t currentTimeLabelId;
uint16_t currentDayLabelId;
uint16_t ledControlId;
uint16_t SuId, MId, TId, WId, ThId, FId, SaId;
struct Daymapping {
  const char* name;
  bool value;
};
Daymapping days[] ={
  {"Sunday", false},
  {"Monday", true},
  {"Tuesday", true},
  {"Wednesday", true},
  {"Thursday", true},
  {"Friday", false},
  {"Saturday", false}
};
String alarmTime = "Unknown";
String currentTime = "Unknown";
String currentDay = "Unknown";
const int buttonPin = 21;
const int ledPin = 19;
const int ledBuiltin = 2;

// set PWM properties
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
// Set varibles
int lampDutyCycle = 0;
bool lampState = LOW;
int buttonState;       // the current state of button
int lastButtonState;  // the previous state of button
const char * ntpServer = "pool.ntp.org";
// Minneapolis is 6 hours behind GMT
const long gmtOffset_sec = -21600; 
const int daylightOffset_sec = 3600;

void numberCall(Control* sender, int type)
{
    Serial.println(sender->value);
}

void textCall(Control* sender, int type)
{
    Serial.print("Text: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
}
void alarmCall(Control* sender, int type)
{
    Serial.print("Text: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
    alarmTime = sender->value;
}

void slider(Control* sender, int type)
{
    Serial.print("Slider: ID: ");
    Serial.print(sender->id);
    Serial.print(", Value: ");
    Serial.println(sender->value);
    // Like all Control Values in ESPUI slider values are Strings. To use them as int simply do this:
    int sliderValueWithOffset = sender->value.toInt() + 100;
    Serial.print("SliderValue with offset");
    Serial.println(sliderValueWithOffset);
}

void buttonCallback(Control* sender, int type)
{
    switch (type)
    {
    case B_DOWN:
        Serial.println("Button DOWN");
        break;

    case B_UP:
        Serial.println("Button UP");
        break;
    }
}

void buttonExample(Control* sender, int type, void* param)
{
    Serial.println(String("param: ") + String(long(param)));
    switch (type)
    {
    case B_DOWN:
        Serial.println("Status: Start");
        ESPUI.print(statusLabelId, "Start");
        break;

    case B_UP:
        Serial.println("Status: Stop");
        ESPUI.print(statusLabelId, "Stop");
        break;
    }
}
void padExample(Control* sender, int value)
{
    switch (value)
    {
    case P_LEFT_DOWN:
        Serial.print("left down");
        break;

    case P_LEFT_UP:
        Serial.print("left up");
        break;

    case P_RIGHT_DOWN:
        Serial.print("right down");
        break;

    case P_RIGHT_UP:
        Serial.print("right up");
        break;

    case P_FOR_DOWN:
        Serial.print("for down");
        break;

    case P_FOR_UP:
        Serial.print("for up");
        break;

    case P_BACK_DOWN:
        Serial.print("back down");
        break;

    case P_BACK_UP:
        Serial.print("back up");
        break;

    case P_CENTER_DOWN:
        Serial.print("center down");
        break;

    case P_CENTER_UP:
        Serial.print("center up");
        break;
    }

    Serial.print(" ");
    Serial.println(sender->id);
}

void switchExample(Control* sender, int value)
{
    switch (value)
    {
    case S_ACTIVE:
        Serial.print("Active:");
        break;

    case S_INACTIVE:
        Serial.print("Inactive");
        break;
    }

    Serial.print(" ");
    Serial.println(sender->id);
}

void daySwitchCallback(Control* sender, int value)
{
    switch (value)
    {
    case S_ACTIVE:
        Serial.print("Active:");
        // Logic to change the right sliders to the right state
        if (sender->id == SuId)
        {
          days[0].value = true;
        }
        if (sender->id == MId)
        {
          days[1].value = true;
        }
        if (sender->id == TId)
        {
          days[2].value = true;
        }
        if (sender->id == WId)
        {
          days[3].value = true;
        }
        if (sender->id == ThId)
        {
          days[4].value = true;
        }
        if (sender->id == FId)
        {
          days[5].value = true;
        }
        if (sender->id == SaId)
        {
          days[6].value = true;
        }
        break;
        

    case S_INACTIVE:
        Serial.print("Inactive");
        if (sender->id == SuId)
        {
          days[0].value = false;
        }
        if (sender->id == MId)
        {
          days[1].value = false;
        }
        if (sender->id == TId)
        {
          days[2].value = false;
        }
        if (sender->id == WId)
        {
          days[3].value = false;
        }
        if (sender->id == ThId)
        {
          days[4].value = false;
        }
        if (sender->id == FId)
        {
          days[5].value = false;
        }
        if (sender->id == SaId)
        {
          days[6].value = false;
        }
        break;
    }

    Serial.print(" ");
    Serial.println(sender->id);
}
void lampSwitchCallback(Control* sender, int value)
{
    switch (value)
    {
    case S_ACTIVE:
        Serial.print("Active:");
        lampState = HIGH;
        break;

    case S_INACTIVE:
        Serial.print("Inactive");
        lampState = LOW;
        break;
    }

    Serial.print(" ");
    Serial.println(sender->id);
}

void timeCallback(Control *sender, int type) {
  if(type == TM_VALUE) { 
    Serial.println(sender->value);
  }
}

void printLocalTime(){
  struct tm timeinfo;
  if(!getLocalTime(&timeinfo)){
    Serial.println("Failed to obtain time");
    return;
  }
  Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");
  // Serial.println("Time variables");
  char timeHour[10];
  strftime(timeHour,10, "%H:%M", &timeinfo);
  // Serial.println(timeHour);
  currentTime = timeHour;
  char timeWeekDay[10];
  strftime(timeWeekDay,10, "%A", &timeinfo);
  // Serial.println(timeWeekDay);
  currentDay = timeWeekDay;
  Serial.println();
}

// function to get the boolean value for a specific day
bool getDayValue(String dayName) {
    for (int i = 0; i < sizeof(days) / sizeof(days[0]); i++) {
        if (dayName.equals(days[i].name)) {
            return days[i].value;
        }
    }
    return false; // Default value if dayName not found
}

// Function to change the Boolean value for a specific day
void setDayValue(String dayName, bool newValue) {
    for (int i = 0; i < sizeof(days) / sizeof(days[0]); i++) {
        if (dayName.equals(days[i].name)) {
            days[i].value = newValue;
            Serial.print(dayName);
            Serial.print(" was changed to");
            Serial.println(newValue);
            return; // Day found and value updated
        }
    }
}
// lamp on function
void lampOn() {
  delay(30);
  lampDutyCycle = lampDutyCycle + 1;
}

void setup(void)
{
    // ESPUI.setVerbosity(Verbosity::VerboseJSON);
    // ChatGPT suggested putting in a setup delay to allow the board to stabilize the power input
    delay(2000);
    // init stuff not dependent on wifi
    Serial.begin(115200);
    ledcSetup(ledChannel,freq,resolution);
    // attach the channel to the GPIO to be controlled
    ledcAttachPin(ledPin, ledChannel);
    pinMode(buttonPin, INPUT_PULLUP); // set ESP32 pin to input pull-up mode
    pinMode(ledBuiltin, OUTPUT);
    buttonState = digitalRead(buttonPin);

#if defined(ESP32)
    WiFi.setHostname(hostname);
#else
    WiFi.hostname(hostname);
#endif

    // try to connect to existing network
    WiFi.begin(ssid, password);
    Serial.print("\n\nTry to connect to existing network");

    {
        int timeout = 1000;

        // Wait for connection, 5s timeout
        do
        {
            delay(500);
            Serial.print(".");
            digitalWrite(ledBuiltin, HIGH);
            delay(1000);
            digitalWrite(ledBuiltin, LOW);
            timeout--;
        } while (timeout && WiFi.status() != WL_CONNECTED);

        // not connected -> create hotspot
        if (WiFi.status() != WL_CONNECTED)
        {
            Serial.print("\n\nCreating hotspot");

            WiFi.mode(WIFI_AP);
            delay(100);
            WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
#if defined(ESP32)
            uint32_t chipid = 0;
            for (int i = 0; i < 17; i = i + 8)
            {
                chipid |= ((ESP.getEfuseMac() >> (40 - i)) & 0xff) << i;
            }
#else
            uint32_t chipid = ESP.getChipId();
#endif
            char ap_ssid[25];
            snprintf(ap_ssid, 26, "ESPUI-%08X", chipid);
            WiFi.softAP(ap_ssid);

            timeout = 5;

            do
            {
                delay(500);
                Serial.print(".");
                timeout--;
            } while (timeout);
        }
    }

    dnsServer.start(DNS_PORT, "*", apIP);

    Serial.println("\n\nWiFi parameters:");
    Serial.print("Mode: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? "Station" : "Client");
    Serial.print("IP address: ");
    Serial.println(WiFi.getMode() == WIFI_AP ? WiFi.softAPIP() : WiFi.localIP());

#ifdef ESP8266
    { HeapSelectIram doAllocationsInIRAM;
#endif

    // ESPUI.separator("Example controls");
    // statusLabelId = ESPUI.label("Status:", ControlColor::Turquoise, "Stop");
    // millisLabelId = ESPUI.label("Millis:", ControlColor::Emerald, "0");
    // ESPUI.button("Push Button", &buttonCallback, ControlColor::Peterriver, "Press");
    // ESPUI.button("Other Button", &buttonExample, ControlColor::Wetasphalt, "Press", (void*)19);
    // testSwitchId = ESPUI.switcher("OnOff", &switchExample, ControlColor::Alizarin, false);
    currentTimeLabelId = ESPUI.label("Current Time", ControlColor::Turquoise, "Unknown");
    currentDayLabelId = ESPUI.label("Current Day", ControlColor::Turquoise, "Unknown");
    ledControlId = ESPUI.switcher("LED Control", &lampSwitchCallback, ControlColor::None, false);
    ESPUI.separator("Alarm 1");
    alarmID = ESPUI.text("Time", alarmCall, ControlColor::Dark, "06:30");
    ESPUI.setInputType(alarmID, "time");
    SuId = ESPUI.addControl(ControlType::Switcher, "Alarm Days", "Su", ControlColor::Turquoise, Control::noParent, &daySwitchCallback);
    MId = ESPUI.addControl(ControlType::Switcher, "", "M", ControlColor::None, SuId, &daySwitchCallback);
    TId = ESPUI.addControl(ControlType::Switcher, "", "T", ControlColor::None, SuId, &daySwitchCallback);
    WId = ESPUI.addControl(ControlType::Switcher, "", "W", ControlColor::None, SuId, &daySwitchCallback);
    ThId = ESPUI.addControl(ControlType::Switcher, "", "Th", ControlColor::None, SuId, &daySwitchCallback);
    //To label these switchers we need to first go onto a "new line" below the line of switchers
	  //To do this we add an empty label set to be clear and full width (with our clearLabelStyle)
    String clearLabelStyle = "background-color: unset; width: 100%;";
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "", None, SuId), clearLabelStyle);
  	//We will now need another label style. This one sets its width to the same as a switcher (and turns off the background)
	  String switcherLabelStyle = "width: 60px; margin-left: .3rem; margin-right: .3rem; background-color: unset;";
	  //We can now just add the styled labels.
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Su", None, SuId), switcherLabelStyle);
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Mo", None, SuId), switcherLabelStyle);
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Tu", None, SuId), switcherLabelStyle);
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "W", None, SuId), switcherLabelStyle);
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Th", None, SuId), switcherLabelStyle);
    // another break
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "", None, SuId), clearLabelStyle);
    FId = ESPUI.addControl(ControlType::Switcher, "", "F", ControlColor::None, SuId, &daySwitchCallback);
    SaId = ESPUI.addControl(ControlType::Switcher, "", "Sa", ControlColor::None, SuId, &daySwitchCallback);
    // another break
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "", None, SuId), clearLabelStyle);
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "F", None, SuId), switcherLabelStyle);
	  ESPUI.setElementStyle(ESPUI.addControl(Label, "", "Sa", None, SuId), switcherLabelStyle);
    // ESPUI.slider("Slider one", &slider, ControlColor::Alizarin, 30);
    // ESPUI.slider("Slider two", &slider, ControlColor::None, 100);
    // ESPUI.text("Text Test:", &textCall, ControlColor::Alizarin, "a Text Field");
    // ESPUI.number("Numbertest", &numberCall, ControlColor::Alizarin, 5, 0, 10);

    /*
     * .begin loads and serves all files from PROGMEM directly.
     * If you want to serve the files from LITTLEFS use ESPUI.beginLITTLEFS
     * (.prepareFileSystem has to be run in an empty sketch before)
     */

    // Enable this option if you want sliders to be continuous (update during move) and not discrete (update on stop)
    // ESPUI.sliderContinuous = true;

    /*
     * Optionally you can use HTTP BasicAuth. Keep in mind that this is NOT a
     * SECURE way of limiting access.
     * Anyone who is able to sniff traffic will be able to intercept your password
     * since it is transmitted in cleartext. Just add a string as username and
     * password, for example begin("ESPUI Control", "username", "password")
     */
    ESPUI.begin("ESPUI Control");

#ifdef ESP8266
    } // HeapSelectIram
#endif
    // init stuff that is dependent on WiFi
      // time stuff
  configTime(gmtOffset_sec,daylightOffset_sec, ntpServer);
  printLocalTime();

}

void loop(void)
{
    dnsServer.processNextRequest();

    static long oldTime = 0;


    // 5000 millisecond loop
    if (millis() - oldTime > 5000)
    {
        ESPUI.print(currentTimeLabelId, currentTime);
        ESPUI.print(currentDayLabelId, currentDay);
        printLocalTime();
        oldTime = millis();
    }
    if (millis() - oldTime > 50)
    {
        // slow dimming on logic
        lastButtonState = buttonState;
        buttonState = digitalRead(buttonPin);
        if (lastButtonState == HIGH && buttonState == LOW){
          lampState = !lampState;
        }
        if (lampDutyCycle < 255 && lampState == HIGH){
        lampOn();
        ledcWrite(ledChannel, lampDutyCycle);
        }
        if (lampState == LOW){
        lampDutyCycle = 0;
        ledcWrite(ledChannel, lampDutyCycle);
        }
        if (getDayValue(currentDay) && currentTime == alarmTime)
        {
          Serial.println("ALARM");
          Serial.println(lampState);
          Serial.println(lampDutyCycle);
          lampState = HIGH;

        }
        // we should update the slider if we turn on the led with the physical button
        ESPUI.updateSwitcher(ledControlId, lampState);
    }
}
