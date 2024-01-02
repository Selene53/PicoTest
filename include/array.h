#include <Arduino.h>

int Split(String sInput, char cDelim, String aPass[],int max){
  int StringCount = 0;
  while (sInput.length() > 0)
  {
    int index = sInput.indexOf(cDelim);
    if (index == -1) // No space found
    {
      aPass[StringCount++] = sInput;
      break;
    }
    else
    {
      aPass[StringCount++] = sInput.substring(0, index);
      sInput = sInput.substring(index + 1);
    }
    if(StringCount > max)
      break;
  }
  return StringCount;
}

int Split(String sInput, char cDelim, String aPass[])
{
  int StringCount = 0;
  while (sInput.length() > 0)
  {
    int index = sInput.indexOf(cDelim);
    if (index == -1) // No space found
    {
      aPass[StringCount++] = sInput;
      break;
    }
    else
    {
      aPass[StringCount++] = sInput.substring(0, index);
      sInput = sInput.substring(index + 1);
    }
  }
  return StringCount;
}
int CountChar(String buffer, char cDelim)
{
  // If you are counting the size of an array you must add 1 IE: A,B,C with return 2 but the array is 3
  int count = 0;
  for (unsigned int i = 0; i < buffer.length(); i++)
  {
    if (buffer[i] == cDelim)
      count++;
  }
  return count;
}

int Find(String thisStr, String thisArr[], int thisArrLength, char thisDelim = '=')
{
  String s[2];
  for (int i = 0; i < thisArrLength; i++)
  {
    Split(thisArr[i], thisDelim, s);
    if (s[0] == thisStr)
    {
      // Serial.println("Find="+String(i)+" "+s[1]);
      return i;
    }
  }
  return -1;
}
String Join(String thisArray[], int thisArrayLength, char withThisDelim = ',')
{
  String ret = "";
  for (int i = 0; i < thisArrayLength; i++)
  { 
    if (i == (thisArrayLength-1)){
       ret += thisArray[i];
    }
    else{
       ret += thisArray[i]+withThisDelim;
    }
  }
  //Serial.println("returning"+ret);
  return ret;
}

bool Replace(String findThisStr, String replacementStr, String inThisArr[], int thisArrLength, char thisDelim = '=')
{
  int f = Find(findThisStr, inThisArr, thisArrLength, thisDelim);
  if (f > -1)
  {
    String s[2];
    Split(inThisArr[f], thisDelim, s);
    s[1] = replacementStr;
    inThisArr[f] = Join(s, 2, thisDelim);
    return true;
  }
  return false;
}

void aAdd(String oldArray[], int oldArrayLen, String newArray[], String newEliment)
{
  for (int i = 0; i < oldArrayLen; i++)
  {
    newArray[i] = oldArray[i];
  }
  newArray[oldArrayLen] = newEliment;
}

void aDel(String aRay[], int aRayLength, int index)
{
  for (int i = 0; i < aRayLength; i++)
  {
    if (i == index)
      aRay[i] = "";
  }
}
