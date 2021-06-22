/***************************************************************************************************
 *                                             INCLUDES
 ***************************************************************************************************/
#if defined(LED)
#include "led.h"
#include "board.h"
#include "gpio_led.h"
#include "n_timer.h"
#include "app.h"
#include "main.h"
/***************************************************************************************************
 *                                             CONSTANTS
 ***************************************************************************************************/

/***************************************************************************************************
 *                                              MACROS
 ***************************************************************************************************/

/***************************************************************************************************
 *                                              TYPEDEFS
 ***************************************************************************************************/
/* LED control structure */
typedef struct {
  u8  mode;       /* Operation mode */
  u8  todo;       /* Blink cycles left */
  u8  onPct;      /* On cycle percentage */
  u8  bright;
  u32 time;      /* On/off cycle time (msec) */
  u32 next;      /* Time for next change */
  u32 step;
  u32 raw;       /*store raw time for breathe*/
} HalLedControl_t;

typedef struct
{
  HalLedControl_t HalLedControlTable[MAX_LEDS];
} HalLedStatus_t;

/***************************************************************************************************
 *                                           GLOBAL VARIABLES
 ***************************************************************************************************/
_attribute_data_retention_ static hal_led_arry_t hal_led;
static u32 HalLedState;                                        // LED state at last set/clr/blink update

_attribute_data_retention_ static u8 HalSleepLedState;         // LED state at last set/clr/blink update
_attribute_data_retention_ static u32 preBlinkState;           // Original State before going to blink mode
//bit 0, 1, 2, 3 represent led 0, 1, 2, 3

_attribute_data_retention_ static HalLedStatus_t HalLedStatusControl;
/***************************************************************************************************
 *                                            LOCAL FUNCTION
 ***************************************************************************************************/

int HalLedUpdate (void *data);
void HalLedOnOff (u32 leds, u8 mode);
void HalLedbreathe (u32 leds, u8 tim);

/***************************************************************************************************
 *                                            FUNCTIONS - API
 ***************************************************************************************************/
void register_led (const hal_led_t *led, u8 num)
{
  hal_led.led = led;
  hal_led.num = num;
}
/***************************************************************************************************
 * @fn      HalLedInit
 *
 * @brief   Initialize LED Service
 *
 * @param   init - pointer to void that contains the initialized value
 *
 * @return  None
 ***************************************************************************************************/
void HalLedInit (void)
{
  /* Initialize all LEDs to OFF */
  HalLedSet (HAL_LED_ALL, HAL_LED_MODE_OFF);
}
/***************************************************************************************************
 * @fn      HalLedSet
 *
 * @brief   Tun ON/OFF/TOGGLE given LEDs
 *
 * @param   led - bit mask value of leds to be turned ON/OFF/TOGGLE
 *          mode - BLINK, FLASH, TOGGLE, ON, OFF
 * @return  None
 ***************************************************************************************************/
u32 HalLedSet (u32 leds, u8 mode)
{
  u32 led;
  HalLedControl_t *sts;

  switch (mode)
  {
    case HAL_LED_MODE_BLINK:
      /* Default blink, 1 time, D% duty cycle */
      HalLedBlink (leds, 1, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME);
      break;

    case HAL_LED_MODE_FLASH:
      /* Default flash, N times, D% duty cycle */
      HalLedBlink (leds, HAL_LED_DEFAULT_FLASH_COUNT, HAL_LED_DEFAULT_DUTY_CYCLE, HAL_LED_DEFAULT_FLASH_TIME);
      break;
    case HAL_LED_MODE_BREATHE:
      HalLedBreathe (leds, HAL_LED_DEFAULT_BREATHE_COUNT, HAL_LED_DEFAULT_BREATHE_STEP, HAL_LED_DEFAULT_BREATHE_TIME);
      break;
    case HAL_LED_MODE_ON:
    case HAL_LED_MODE_OFF:
    case HAL_LED_MODE_TOGGLE:

      led = HAL_LED_1;
      leds &= HAL_LED_ALL;
      sts = HalLedStatusControl.HalLedControlTable;

      while (leds)
      {
        if (leds & led)
        {
          if (mode != HAL_LED_MODE_TOGGLE)
          {
            sts->mode = mode;  /* ON or OFF */
          }
          else
          {
            sts->mode ^= HAL_LED_MODE_ON;  /* Toggle */
          }
          HalLedOnOff (led, sts->mode);
          leds ^= led;
        }
        led <<= 1;
        sts++;
      }
      break;

    default:
      break;
  }

  return ( HalLedState );
}
/***************************************************************************************************
 * @fn      HalLedBlink
 *
 * @brief   Blink the leds
 *
 * @param   leds       - bit mask value of leds to be blinked
 *          numBlinks  - number of blinks
 *          percent    - the percentage in each period where the led
 *                       will be on
 *          period     - length of each cycle in milliseconds
 *
 * @return  None
 ***************************************************************************************************/
void HalLedBlink (u32 leds, u8 numBlinks, u8 percent, u32 period)
{
  u32 led;
  HalLedControl_t *sts;

  if (leds && percent && period)
  {
    if (percent <= 100)
    {
      led   = HAL_LED_1;
      leds &= HAL_LED_ALL;
      sts   = HalLedStatusControl.HalLedControlTable;

      while (leds)
      {
        if (leds & led)
        {
          /* Store the current state of the led before going to blinking if not already blinking */
          if(sts->mode < HAL_LED_MODE_BLINK )
            preBlinkState |= (led & HalLedState);

          sts->mode  = HAL_LED_MODE_OFF;                    /* Stop previous blink */
          sts->time  = period;                              /* Time for one on/off cycle */
          sts->onPct = percent;                             /* % of cycle LED is on */
          sts->todo  = numBlinks;                           /* Number of blink cycles */
          if (!numBlinks) sts->mode |= HAL_LED_MODE_FLASH;  /* Continuous */
          sts->next = clock_time();                         /* Start now */
          sts->mode |= HAL_LED_MODE_BLINK;                  /* Enable blinking */
          leds ^= led;
        }
        led <<= 1;
        sts++;
      }
    }
  }
  else
  {
    HalLedSet (leds, HAL_LED_MODE_OFF);                     /* No on time, turn off */
  }
}
/***************************************************************************************************
 * @fn      HalLedBreathe
 *
 * @brief   Breathe the leds
 *
 * @param   leds        - bit mask value of leds to be blinked
 *          numBreathe  - number of breathe
 *          step        - the length in each period where the led
 *                        brightness last time
 *          period      - length of each cycle in milliseconds
 *
 * @return  None
 ***************************************************************************************************/
void HalLedBreathe (u32 leds, u8 numBreathe, u32 step, u32 period)
{
  u32 led;
  HalLedControl_t *sts;

  if (leds && step && period)
  {
    led   = HAL_LED_1;
    leds &= HAL_LED_ALL;
    sts   = HalLedStatusControl.HalLedControlTable;

    while (leds)
    {
      if (leds & led)
      {
        /* Store the current state of the led before going to breath if not already breath */
        if(sts->mode < HAL_LED_MODE_BLINK )
          preBlinkState |= (led & HalLedState);

        sts->mode      = HAL_LED_MODE_OFF;                    /* Stop previous blink */
        sts->bright    = 0;
        sts->time      = period;                              /* Time for one on/off cycle */
        sts->raw       = period;
        sts->step      = step;                                /* % of cycle LED is on */
        sts->todo      = numBreathe;                           /* Number of blink cycles */
        if (!numBreathe) sts->mode |= HAL_LED_MODE_CONTINUE_BREATHE;  /* Continuous */
        sts->next  = clock_time();                                  /* Start now */
        sts->mode |= HAL_LED_MODE_BREATHE;                  /* Enable blinking */
        leds ^= led;
      }
      led <<= 1;
      sts++;
    }
  }
  else
  {
    HalLedSet (leds, HAL_LED_MODE_OFF);                     /* No on time, turn off */
  }
}
/***************************************************************************************************
 * @fn      HalLedUpdate
 *
 * @brief   Update leds to work with blink
 *
 * @param   none
 *
 * @return  none
 ***************************************************************************************************/
int HalLedUpdate (void *data)
{
  u32 led;
  u8  pct = 0;
  u32 leds;
  HalLedControl_t *sts;
  u32 time = 0;
  u32 next = 0;
  u32 wait;

  void (*cb)();

  led  = HAL_LED_1;
  leds = HAL_LED_ALL;
  sts  = HalLedStatusControl.HalLedControlTable;

  time = clock_time();

  while (leds)
  {
    if (leds & led)
    {
      if (sts->mode & HAL_LED_MODE_BLINK)
      {
        if(time_after(time, sts->next))
        {
          if (sts->mode & HAL_LED_MODE_ON)
          {
            pct = 100 - sts->onPct;               /* Percentage of cycle for off */
            sts->mode &= ~HAL_LED_MODE_ON;        /* Say it's not on */
            HalLedOnOff (led, HAL_LED_MODE_OFF);  /* Turn it off */

            if (!(sts->mode & HAL_LED_MODE_FLASH))
            {
              sts->todo--;                        /* Not continuous, reduce count */
            }
          }
          else if ( (!sts->todo) && !(sts->mode & HAL_LED_MODE_FLASH) )
          {
            sts->mode ^= HAL_LED_MODE_BLINK;      /* No more blinks */
          }
          else
          {
            pct = sts->onPct;                     /* Percentage of cycle for on */
            sts->mode |= HAL_LED_MODE_ON;         /* Say it's on */
            HalLedOnOff (led, HAL_LED_MODE_ON);   /* Turn it on */
          }
          if (sts->mode & HAL_LED_MODE_BLINK)
          {
            //warning out of array!!!
            wait = (u32)sts->time/100;
            wait = wait * pct;
            sts->next = time + wait;
          }
          else
          {
            /* no more blink, no more wait */
            wait = 0;
            /* After blinking, set the LED back to the state before it blinks */
            // HalLedSet (led, ((preBlinkState & led)!=0)?HAL_LED_MODE_ON:HAL_LED_MODE_OFF);
            HalLedSet (led, HAL_LED_MODE_OFF);
            /* Clear the saved bit */
            preBlinkState &= (led ^ 0xFFFFFFFF);
          }
        }
        else
        {
          wait = (u32)((int)sts->next - (int)time);  /* Time left */
        }

        if (!next || ( wait && (wait < next) ))
        {
          next = wait;
        }
      }
      leds ^= led;
    }
    led <<= 1;
    sts++;
  }

  if(!next){
    if(data){
      cb = (void (*)())data;
      cb();
    }
  }

  return next;
}

/***************************************************************************************************
 * @fn      HalLedUpdateBreath
 *
 * @brief   Update leds to work with breath
 *
 * @param   none
 *
 * @return  none
 ***************************************************************************************************/
int HalLedUpdateBreath (void *data)
{
  u32 led;
  u32 leds;
  u32 pct = 0;
  u32 time = 0;
  u32 next = 0;
  u32 wait;

  HalLedControl_t *sts;

  led  = HAL_LED_1;
  leds = HAL_LED_ALL;

  sts = HalLedStatusControl.HalLedControlTable;

  time = clock_time();

  while (leds)
  {
    if (leds & led)
    {
      if (sts->mode & HAL_LED_MODE_BREATHE)
      {
        if (time_after(time, sts->next))
        {
          if(sts->time)
          {
            pct = sts->step;

            sts->bright++;
            if(sts->bright > LED_BRIGHT_LEVEL)
              sts->bright = LED_BRIGHT_LEVEL;

            if((led == HAL_LED_8) && ((sts->bright == 31) || (sts->bright == 32) ||\
                  (sts->bright == 28) || (sts->bright == 29))){
              ;
            }else
              HalLedbreathe (led, sts->bright);
            sts->time -= sts->step;
          }
          else if(!(sts->mode & HAL_LED_MODE_CONTINUE_BREATHE))
          {
            sts->todo--;
            sts->time = sts->raw;

            if(!sts->todo)
            {
              sts->mode ^= HAL_LED_MODE_BREATHE;      /* No more breathe */
            }
          }
          else
          {
            sts->time = sts->raw;
          }

          if (sts->mode & HAL_LED_MODE_BREATHE)
          {
            //warning overflow!!!
            wait = pct;
            sts->next = time + wait;
          }
          else
          {
            /* no more blink, no more wait */
            wait = 0;
            /* After blinking, set the LED back to the state before it blinks */
            //HalLedSet (led, ((preBlinkState & led)!=0)?HAL_LED_MODE_ON:HAL_LED_MODE_OFF);
            HalLedSet (led, HAL_LED_MODE_ON);
            /* Clear the saved bit */
            preBlinkState &= (led ^ 0xFFFFFFFF);
          }
        }
        else
        {
          wait = (u32)((int)sts->next - (int)time);  /* Time left */
        }

        if (!next || ( wait && (wait < next) ))
        {
          next = wait;
        }
      }
      leds ^= led;
    }
    led <<= 1;
    sts++;
  }
  return next;
}
/***************************************************************************************************
 * @fn      HalLedOnOff
 *
 * @brief   Turns specified LED ON or OFF
 *
 * @param   leds - LED bit mask
 *          mode - LED_ON,LED_OFF,
 *
 * @return  none
 ***************************************************************************************************/
void HalLedOnOff (u32 leds, u8 mode)
{
  if(!hal_led.led)
  {
    return;
  }

  for(u8 i = 0; i < hal_led.num; i++)
  {
    if(hal_led.led[i].leds == leds)
    {
      hal_led.led[i].led_on_off(leds, mode);
    }
  }

  /* Remember current state */
  if (mode)
  {
    HalLedState |= leds;
  }
  else
  {
    HalLedState &= (leds ^ 0xFFFFFFFF);
  }
}

/***************************************************************************************************
 * @fn      HalLedbreathe
 *
 * @brief   Turns specified LED breahte ON
 *
 * @param   leds - LED bit mask
 *          mode - LED_ON,LED_OFF,
 *
 * @return  none
 ***************************************************************************************************/
void HalLedbreathe (u32 leds, u8 tim)
{
  if(!hal_led.led)
  {
    return;
  }

  for(u8 i = 0; i < hal_led.num; i++)
  {
    if(hal_led.led[i].leds == leds)
    {
      hal_led.led[i].led_breath(leds, tim);
    }
  }

  /* Remember current state */
  if (tim)
  {
    HalLedState |= leds;
  }
  else
  {
    HalLedState &= (leds ^ 0xFFFFFFFF);
  }
}
/***************************************************************************************************
 * @fn      HalGetLedState
 *
 * @brief   Dim LED2 - Dim (set level) of LED2
 *
 * @param   none
 *
 * @return  led state
 ***************************************************************************************************/
u32 HalLedGetState ()
{
  return HalLedState;
}
/***************************************************************************************************
 * @fn      HalLedEnterSleep
 *
 * @brief   Store current LEDs state before sleep
 *
 * @param   none
 *
 * @return  none
 ***************************************************************************************************/
void HalLedEnterSleep( void )
{
  /* TURN OFF all LEDs to save power */
  HalLedOnOff (HAL_LED_ALL, HAL_LED_MODE_OFF);
}
/***************************************************************************************************
 * @fn      HalLedExitSleep
 *
 * @brief   Restore current LEDs state after sleep
 *
 * @param   none
 *
 * @return  none
 ***************************************************************************************************/
void HalLedExitSleep( void )
{
  /* Load back the saved state */
  HalLedOnOff(HalSleepLedState, HAL_LED_MODE_ON);

  /* Restart - This takes care BLINKING LEDS */
  HalLedUpdate(NULL);
}
/***************************************************************************************************
 ***************************************************************************************************/
#endif



