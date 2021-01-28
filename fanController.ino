#include <OneWire.h> 
#include <DallasTemperature.h>
#include <EEPROM.h>

#define ONE_WIRE_BUS 11

String data = "";
int percent = 0;
int analogValue = 0; // fan off at beginning
int lastAnalogValue = 666; 

// Default temperature profile
// temp, fan speed in %, temp, %, temp, %, temp, %, ...
int profile[] = {20, 0, 25, 25, 30, 50, 40, 75, 50, 100};

// valid commands and some hint about each one
char *commands[] = 
{
    " - GET_FAN","\t\tGet current fan speed.",
    " - GET_TEMP","\t\tGet current temperature.",
    " - GET_ALL","\t\tGet current fan & temperature.",
    " - OVERRIDE(value)","\tOverride temperature profile with supplied 'value', given in %.",
    " - OVERRIDE_OFF","\tDisable temperature override.",
    " - GET_PROFILE","\tGet current temperature profile.",
    " - SET_PROFILE(profile)","\tSet new temperature profile (see below).",
    " - DEFAULT_PROFILE","\tRevert to 'factory' default temperature profile."
};

bool overrideProfile = false;

int temp = 0;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() 
{
    Serial.begin(9600);
    pinMode(9, OUTPUT);

    Serial.println();
    Serial.println("Arduino DC Fan controller ");
    Serial.println("https://github.com/bdelphin/fanduino");
    Serial.println();

    printValidCommands();

    if(EEPROM.read(10) == 0)
    {
        // first run of this program on this arduino board
        // store needed values on EEPROM
        for (int i = 0; i < sizeof(profile); i++) 
        {
            EEPROM.write(i, profile[i]);
        }
        // set 10th bit to 1
        EEPROM.write(10, 1);
    }
    else
    {
        // not the first run
        // get EEPROM stored profile
        for (int i = 0; i < 10; i++) 
        {
            byte val = EEPROM.read(i);
            profile[i] = (int)val;
        }
        printProfile("Restored last temperature profile : ");
    }

}

void loop() 
{
    if(overrideProfile)
    {
        // override temperature profile with received value
        setFanSpeed();
    }
    else
    {
        // normal mode, set fan speed according to temperature profile
        sensors.requestTemperatures();
        temp = sensors.getTempCByIndex(0);
    
        // temp threshold in on even indexes (0, 2, 4, 6, 8, ...)
        // so let's check each temperature threshold
        for(int i = 0; i < sizeof(profile)/2; i += 2)
        {
            // if temperature is below threshold, set corresponding fan speed
            // and break the loop
            if (temp < profile[i])
            {
                percent = profile[i+1];
                analogValue= map(percent, 0, 100, 0, 255); 

                setFanSpeed();
                
                break;
            }
        }
    }
    
    // monitor serial for commands
    if (Serial.available() > 0) 
    {
        // read received data
        data = Serial.readStringUntil('\n');
        
        // if there's a : check command
        if(data.indexOf("(") > 0)
        {
            int index = data.indexOf("(");
            int indexEnd = data.indexOf(")");

            String command = data.substring(0, index);
            String argument = data.substring(index+1, indexEnd);
 
            //Serial.println(command + " -> " + argument);
            
            if (command == "SET_PROFILE")
            {
                Serial.println("New temperature profile received : " + argument);
                Serial.println();
                
                // TODO: improve for random profile size ? -> is it really needed ?

                // fixed profile size :
                for(int i = 0; i < 10; i++)
                {
                    int argIndex = argument.indexOf(",");
                    int argIndexEnd = argument.indexOf(")");
        
                    String temp = argument.substring(0, argIndex);
                    String remaining = argument.substring(argIndex+1, argIndexEnd);
                    profile[i] = temp.toInt();

                    argument = remaining;
                }

                // print new temperature profile
                printProfile("New temperature profile set : ");

                // store new profile in EEPROM
                for (int i = 0; i < sizeof(profile); i++) 
                {
                    EEPROM.write(i, profile[i]);
                }

                // print some hints
                Serial.println("New profile saved in EEPROM.");
                Serial.println("It will be restored on reboot. If you want to revert to default profile, launch DEFAULT_PROFILE");
                Serial.println();

                // NOT A GOOD IDEA :
                // sizeof argument
                // for loop on each argument char, count ','
                // malloc array : number of ','
                // realloc profile array ?
                // loop through argument array
                
            }
            else if (command == "OVERRIDE")
            {
                // enable override
                overrideProfile = true;

                // convert string to int
                percent = argument.toInt();
    
                // handle wrong values
                if (percent > 100)
                    percent =100;
                else if (percent < 0)
                    percent = 0;
                
                Serial.print("Temperature profile overriden (");
                Serial.print(percent, DEC);
                Serial.println("%), please be carefull.");
                Serial.println();
                
                // convert fan value from % to 0-255
                analogValue= map(percent, 0, 100, 0, 255); 
            }

            
        }
        else // regular commands
        {
            if (data == "GET_ALL")
            {
                // return current fan speed & temperature
                printFan();
                printTemp();
                Serial.println();
            }
            else if (data == "GET_FAN")
            {
                // return current fan speed
                printFan();
                Serial.println();
            }
            else if (data == "GET_TEMP")
            {
                // return current temperature
                printTemp();
                Serial.println();
            }
            else if (data == "GET_PROFILE")
            {
                // return current profile
                printProfile("Current temperature profile : ");
                /*Serial.print("Current profile : "); 
                Serial.println("(Max temperature, fan speed)");
                for(int i = 0; i < sizeof(profile)/2; i += 2)
                {
                    Serial.println("(" + String(profile[i]) + ", " + String(profile[i+1]) + ")");
                }
                Serial.println();*/
            }
            else if (data == "DEFAULT_PROFILE")
            {
                // revert to "factory" default temperature profile
                int profile[] = {20, 0, 25, 25, 30, 50, 40, 75, 50, 100};
                printProfile("Factory default temperature profile reseted : ");
            }
            else if (data == "OVERRIDE_OFF")
            {
                Serial.println("Temperate profile override disabled.");
                overrideProfile = false;
                Serial.println();

                // return current profile
                printProfile("Current temperature profile : ");
                /*Serial.print("Current profile : "); 
                Serial.println("(Max temperature, fan speed)");
                for(int i = 0; i < sizeof(profile)/2; i += 2)
                {
                    Serial.println("(" + String(profile[i]) + ", " + String(profile[i+1]) + ")");
                }
                Serial.println();*/
            }
            else
            {
                Serial.println("Unknown command.");
                printValidCommands();        
            }
        }
    }
}

void printProfile(String introText)
{
    // return current profile
    Serial.print(introText); 
    Serial.println("(Max temperature, fan speed)");
    for(int i = 0; i < sizeof(profile)/2; i += 2)
    {
        Serial.println("(" + String(profile[i]) + ", " + String(profile[i+1]) + ")");
    }
    Serial.println();
}

void printFan()
{
    Serial.print("Current fan speed : ");
    Serial.print(map(analogValue, 0, 255, 0, 100));
    Serial.println("%");
}

void printTemp()
{
    sensors.requestTemperatures();
    Serial.print("Temperature : "); 
    Serial.print(sensors.getTempCByIndex(0));
    Serial.println("°C");
}

void printValidCommands()
{
    Serial.println("Valid commands are : ");
    for(int i = 0; i < sizeof(commands)/2; i += 2)
    {
        Serial.print(commands[i]);
        //Serial.print(" : ");
        Serial.println(commands[i+1]);
    }
    Serial.println();
    Serial.println("Temperature profile is composed of five temperature threshold / fan speed in % pairs, each element separated by a comma.");
    //Serial.println("Structure: temp1,fan%1,temp2,fan%2,temp3,fan%3,temp4,fan%4,temp5,fan%5");
    Serial.println("Example (default profile): 20,0,25,25,30,50,40,75,50,100");
    Serial.println("Read the doc at https://github.com/bdelphin/fanduino");
    //Serial.println("It means that for a temperature below 20°C, fan speed should be 0%. For a temperature below 25°C, fan speed should be 25%, and so on.");
    Serial.println();
}

void setFanSpeed()
{
    // only if it has changed
    if (analogValue != lastAnalogValue)
    {
        Serial.print("PWM value sent to fans : ");
        Serial.println(analogValue);
        Serial.println();
        
        analogWrite(9, analogValue);
        lastAnalogValue = analogValue;
    }
 }
