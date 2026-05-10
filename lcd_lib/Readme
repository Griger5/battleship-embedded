Biblioteka obsługująca wyświetlacze LCD

1. Wybierz właściwy plik dla używanej płytki ewaluacyjnej
LandTiger_LCD.c lub Open1768_LCD.c i dołącz go do projektu.

2. Zainicjuj komunikację (piny) procedurą
   lcdConfiguration()

3. Sprawdź jaki układ sterujący obsługuje wyświetlacz odczytując jego rejestr OSCIL_ON (0x00).
  W zależności od wyświetlacza należy zdefiniować odpowiedni symbol dla kompilatora.
  Jeżeli wyświetlacz zwróci
    a) 0x8989 tzn. że układem sterującym jest SDD1289 i symbol ma być LCD_SDD1289
    b) 0x9325 lub 0x9328 tzn. że układem sterującym jest ILI9325 lub ILI9328
       i symbol ma być LCD_ILI9325

4. Dołącz plik wyświetlacza do projektu: LCD_ILI9325.c lub LCD_SDD1289.c
  (można dołączyć oba, ale jest to zbędne) i wywołał procedurę ustawiającą jego parametry z dołączonego pliku.


Uwaga: kombinacja LandTiger + ILI może nie zadziałać ze względu na opoźnienia które należy zwiększyć.
