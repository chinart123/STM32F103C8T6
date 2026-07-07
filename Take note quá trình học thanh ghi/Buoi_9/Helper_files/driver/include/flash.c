#include <flash.h>
#include <bitband.h>
//==============================================================================================================================================================
__no_init volatile FLASH_TypeDef FLASH @ 0x40022000UL;
__no_init volatile FLASH_BITBAND_TypeDef FLASH_BITBAND @ 0x42440000UL;
void Flash_Lock()
{
  while (FLASH_BITBAND.SR.BSY);
  FLASH_BITBAND.CR.PG = 0;
  FLASH_BITBAND.CR.LOCK = 1;
}
//==============================================================================================================================================================
void Flash_Unlock()
{
  do
  {
    FLASH.KEYR = 0x45670123;
    FLASH.KEYR = 0xCDEF89AB;
  } while (FLASH_BITBAND.CR.LOCK);
  FLASH_BITBAND.CR.PG = 1;
}
//==============================================================================================================================================================
unsigned long Flash_CheckOccupied(unsigned long Address, unsigned long Length)
{
  unsigned long sector = (Address - 0x08000000) / 1024;
  Length = (Length >> 2) + (!!(Length & 3));
  while (Length--)
  {
    if (*((unsigned long*)Address) != 0xFFFFFFFF)
      return 1;
    Address += 4;
    if (((Address - 0x08000000) / 1024) != sector)
      break;
  }
  return 0;
}
//==============================================================================================================================================================
void Flash_WriteShort(unsigned long Address, unsigned long Data)
{
  *((unsigned short*)Address) = Data;
  while(!FLASH_BITBAND.SR.EOP);
  FLASH_BITBAND.SR.EOP = 1;
}
//==============================================================================================================================================================
void Flash_WriteShorts(unsigned long Address, unsigned short *Data, unsigned long Length)
{  
  while (Length--)
  {
    if (Data)
      *((unsigned short*)Address) = *Data++;
    else
      *((unsigned short*)Address) = 0;
    while(!FLASH_BITBAND.SR.EOP);
    FLASH_BITBAND.SR.EOP = 1;
    Address += 2;
  }
  while (FLASH_BITBAND.SR.BSY);
}
//==============================================================================================================================================================
void Flash_Erase(unsigned long Address)
{
  FLASH_BITBAND.CR.PG = 0;
  FLASH_BITBAND.CR.PER = 1;
  FLASH.AR = Address;
  FLASH_BITBAND.CR.STRT = 1;
//  while (FLASH_BITBAND.SR.BSY);
//  FLASH_BITBAND.SR.EOP = 1;
//  FLASH_BITBAND.CR.PER = 0;
//  FLASH_BITBAND.CR.PG = 1;
}
//==============================================================================================================================================================
unsigned long Flash_CheckErase()
{
  if (FLASH_BITBAND.SR.BSY)
    return 1;
  FLASH_BITBAND.SR.EOP = 1;
  FLASH_BITBAND.CR.PER = 0;
  FLASH_BITBAND.CR.PG = 1;
  return 0;
}