#include <Arduino.h>
#include <LittleFS.h>
#include <array.h>
#define LFS LittleFS
void listDir(FS *fs, String dirname, uint8_t levels) // Could be removed for diags only. Displayed on terminal
{
    Serial.println("Listing directory: " + dirname);

    File root = fs->open("/" + dirname, "r");
    if (!root)
    {
        Serial.println("- failed to open directory");
        return;
    }
    if (!root.isDirectory())
    {
        Serial.println(" - not a directory");
        return;
    }

    File file = root.openNextFile();
    while (file)
    {
        if (file.isDirectory())
        {
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if (levels)
            {
                listDir(fs, file.name(), levels - 1);
            }
        }
        else
        {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("\tSIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}

String readAllLines(String file) // GPF on large files
{
    File tbl = LFS.open("/" + file, "r");
    String buffer = "";
    int i = 0;
    delay(1);
    while (tbl.available())
    {
        buffer += tbl.readStringUntil('\0') + '\n'; // Until(EOF);
        i++;
    }
    tbl.close();
    Serial.println(String(i)+ " lines read");
    return buffer;
}

void serialAllLines(String file)
{
    File tbl = LFS.open("/" + file, "r");
    String buffer = "";
    delay(1);
    while (tbl.available())
    {
        Serial.print(tbl.readStringUntil('\0'));
    }
    tbl.close();
}

String readFile(String file)
{
    //  long mils = millis();

    File tbl = LFS.open("/" + file, "r");
    String buffer;
    delay(1);
    buffer = tbl.readString(); // Until(EOF);
    tbl.close();
    //  Serial.println("readFile run time = " + String(millis()-mils));
    return buffer;
}
bool writeFile(String filename, String content, bool append = false)
{
    File file;
    if (!LFS.exists("/" + filename)){
        append = false;
        Serial.println(filename+" does not exist!!!");
    }
    if (append)
        file = LFS.open("/" + filename, "r+");
    else
        file = LFS.open("/" + filename, "w");
        
    delay(1);
    if (file)
    {
        if (append)
            file.seek((uint32_t)(file.size()), SeekSet);

        file.write(content.c_str());

        file.close();
        return true;
    }
    else{
        Serial.println(filename+" Did not open"); 
        return false;
    }
}

void addConfigItem(String Setting, String Value)
{
    File file = LFS.open("/config.txt", "r+");
    file.seek((file.size() - 1), SeekSet);
    // file.print("," + Setting + "=" + Value);
    String s = "," + Setting + "=" + Value;
    file.write(s.c_str());
    file.close();
}

void setConfigItem(String Setting, String Value)
{
    // long mils = millis();
    String buffer = readFile("config.txt");
    int count = CountChar(buffer, ',') + 1;
    String s[count];
    Split(buffer, ',', s);

    if (Replace(Setting, Value, s, count, '='))
    {
        buffer = Join(s, count);
        writeFile("config.txt", buffer);
    }
    else
    {
        addConfigItem(Setting, Value);
    }
    // Serial.println("setConfigItem run time = " + String(millis()-mils));
}
String getConfigItem(String item, String _default = "")
{
    // long mils = millis();
    String buffer = readFile("config.txt");
    int count = CountChar(buffer, ',') + 1;
    String s[count];
    Split(buffer, ',', s);
    int x = Find(item, s, count);

    if (x > -1)
    {
        String f[2];
        Split(s[x], '=', f);
        // Serial.println("getConfigItem for " + item + " = " + String(x) + " is " + f[1]);
        // Serial.println("getConfigItem run time = " + String(millis()-mils));
        return f[1];
    }
    // Serial.println("getConfigItem for " + item + " (default) = " + String(x) + " is " + _default);
    // Serial.println("getConfigItem run time = " + String(millis()-mils));
    return _default;
}
