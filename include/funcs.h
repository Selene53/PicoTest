#include <Arduino.h>
#include <ezTime.h>
const int rowDataSize = 39;
class CircleQ
{
public:
  String fileName;
  int maxRows;
  int rowSize;
  int currentRow;
  File tbl;

  void begin(String filename, int maxrows, int rowsize, int currentrow = -1)
  {
    fileName = filename;
    maxRows = maxrows;
    rowSize = rowsize;
    if (!LFS.exists("/" + filename))
    {
      Serial.println(filename + " Does not exist");
      tbl = LFS.open("/" + filename, "w");
      tbl.flush();
      tbl.close();
    }
    if (currentrow == -1)
      currentRow = getRows();
    else
      currentRow = currentrow;
  }

  String get(int record)
  {
    int moveTo = (record * rowSize);
    String buff = "";
    tbl = LFS.open("/" + fileName, "r");
    delay(10);
    if (tbl.available())
    {
      tbl.seek(moveTo, SeekSet);
      buff = tbl.readStringUntil('\n');
    }
    else
    {
      Serial.println("Cant seek");
    }
    tbl.close();
    return String(buff);
  }
  void put(int record, const char *s)
  {
    tbl = LFS.open("/" + fileName, "r+");
    tbl.seek(record * rowSize, SeekSet);
    // unsigned char buff[rowSize];
    // s.getBytes(buff, rowSize);
    tbl.write(s, rowSize);
    tbl.flush();
    tbl.close();
  }

  int Add(const char *s)
  {
    int mils = millis();

    currentRow += 1;
    if (currentRow > maxRows)
      currentRow = 0;

    tbl = LFS.open("/" + fileName, "r+");
    // tbl.seek((currentRow * rowSize) - currentRow, SeekSet);
    tbl.seek((currentRow * rowSize), SeekSet);
    // unsigned char buff[rowSize];
    // s.getBytes(buff, rowSize);
    tbl.write(s, rowSize); // Dont pass a string here otherwise you get unwanted nulls
    tbl.flush();
    tbl.close();

    setConfigItem("CurrentRow", String(currentRow));
    Serial.println("CurrentRow=" + String(currentRow) + " " + String(millis() - mils) + "mils");
    return currentRow;
  }

  long getRows()
  {
    tbl = LFS.open("/" + fileName, "r+");
    int tblSize = tbl.size();
    tbl.close();
    return tblSize / rowSize;
  }

private:
};
CircleQ cQ;

String fmtNum(float num, int precision, int padleft, int width)
{
  String s = String(num, precision);
  s = "0000000000" + s;
  s = s.substring(s.length() - width, s.length());
  return s;
}
String replicate(String str, int cnt)
  {
      String s = "";
      for (int i = 0; i < cnt; i++)
      {
          s = s + str;
      }
      return s;
}
String padl(String str, int len, String fill)
{
    String s = replicate(fill, len) + str;
    return s.substring(s.length() - len, s.length());
}

String mynow(Timezone myTZ){
    	return String(myTZ.year()) + "-" +padl(String(myTZ.month()),2,"0") + "-" + padl(String(myTZ.day()),2,"0")+ " "+ padl(String(myTZ.hour()),2,"0")+":"+padl(String(myTZ.minute()),2,"0") + ":" + padl(String(myTZ.second()),2,"0");
  
}