#define ONE_BYTE_RECEIVED     (1)
#define NO_BYTE_RECEIVED      (1)
#define PACKET_BYTE_COUNT     (6)
#define RECEIVE_TIMEOUT       (100)
#define RECEIVE_BYTE_IDX      (0)
#define RECEIVE_BUFFER_SIZE   (128)
#define TIMER0_PRESCALER      (80)
#define TIMER0_1MS_COUNTVALUE (1000)
#define PACKET_HEADER         ("H20")
#define PACKET_HEADER_SIZE    (3)

hw_timer_t *Timer0_Cfg = NULL;
String inputString = "";         // a String to hold incoming data
bool stringComplete = false;  // whether the string is complete
char ucReceiveBuffer[RECEIVE_BUFFER_SIZE] = {0};
unsigned int uiRecevCount = {0};
char ucDataBuffer[RECEIVE_BUFFER_SIZE] = {0};
unsigned long long uiCurrentTickCount = 0;
unsigned long long uiTickStart = 0;
byte ucNoOperationPacket[] = {0x48, 0x32, 0x30, 0x02, 0x00, 0x47};

/* Timer 0 service routine 
* Increment the tick count ms.
*/
void IRAM_ATTR Timer0_ISR()
{
  uiCurrentTickCount++;
  digitalWrite(LED_BUILTIN, !digitalRead(2));
}

unsigned long long GetTickCount( void );

void setup() {
  // initialize serial:
  Serial.begin(115200);                                   /* Initialize serial port with baud rate 9600 */
  pinMode(LED_BUILTIN, OUTPUT);
  Timer0_Cfg = timerBegin(0, TIMER0_PRESCALER, true);                 /* Configure the up count timer with a presscaler of 80 */
  timerAttachInterrupt(Timer0_Cfg, &Timer0_ISR, true);  /* Attaching a timer isr invoking every 1ms */
  timerAlarmWrite(Timer0_Cfg, TIMER0_1MS_COUNTVALUE, true);              /* Setting the timer for 1ms tick increment */
  timerAlarmEnable(Timer0_Cfg);                         /* Enable the timer 0 */
}

void loop() {
  if (stringComplete) {                                 /* Check whether a new string is to process */
    memcpy(ucDataBuffer, ucReceiveBuffer, RECEIVE_BUFFER_SIZE);         /* Copy the data receive buffer to the data processing buffer */
    memset(ucReceiveBuffer, 0x00, RECEIVE_BUFFER_SIZE);                 /* Clear the data receive buffer */
    if (0 == memcmp(ucDataBuffer, PACKET_HEADER, PACKET_HEADER_SIZE))
    {
      Serial.write(ucNoOperationPacket, 6);
    }
    uiRecevCount = 0;                                   /* Reset the data receive buffer index */
    stringComplete = false;                             /* Reset the flag for further data reception */
    memcpy(ucDataBuffer, ucReceiveBuffer, RECEIVE_BUFFER_SIZE);
    
  }

  /* Reset the receice data buffer and index when timeout occurs after first byte reception and 
      incomplete string after first byte reception */
  if (((GetTickCount() - uiTickStart) > RECEIVE_TIMEOUT) && (false == stringComplete)
                    &&(NO_BYTE_RECEIVED != uiRecevCount))
  {
    memset(ucReceiveBuffer, 0x00, RECEIVE_BUFFER_SIZE);
    uiRecevCount = 0;
  }
}

/*
  SerialEvent occurs whenever a new data comes in the hardware serial RX. This
  routine is run between each time loop() runs, so using delay inside loop can
  delay response. Multiple bytes of data may be available.
*/
void serialEvent() {
  while (Serial.available()) 
  {
    char inChar = (char)Serial.read();                    /* Get the new byte */
    ucReceiveBuffer[uiRecevCount++] = inChar;             /* Copy the new byte to receive buffer */
    if ('H' != ucReceiveBuffer[0])
    {                                                     /* Clear the receive buffer and index if first byte received is not 'H' */
      memset(ucReceiveBuffer, 0x00, RECEIVE_BUFFER_SIZE);
      uiRecevCount = 0;
    }
    else
    {
      if (ONE_BYTE_RECEIVED == uiRecevCount)                              /* When the first byte is 'H' Start the tick count for timeout calculation. */
      {
        uiTickStart = GetTickCount();        
      }
    }
    if (PACKET_BYTE_COUNT == uiRecevCount)                               /* When total bytes are received set the flag to indicate the data processing */                        
    {
      stringComplete = true;
    }
  }
}

/* Get the curernt tick count */
unsigned long long GetTickCount( void )
{
  return uiCurrentTickCount;
}
