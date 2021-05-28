/**************************************************************************************************
Filename:       hal_led.c
Revised:        $Date: 2011-09-15 16:51:11 -0700 (Thu, 15 Sep 2011) $
Revision:       $Revision: 27590 $

Description:    This file contains the interface to the HAL LED Service.


Copyright 2006-2011 Texas Instruments Incorporated. All rights reserved.

IMPORTANT: Your use of this Software is limited to those specific rights
granted under the terms of a software license agreement between the user
who downloaded the software, his/her employer (which must be your employer)
and Texas Instruments Incorporated (the "License").  You may not use this
Software unless you agree to abide by the terms of the License. The License
limits your use, and you acknowledge, that the Software may not be modified,
copied or distributed unless embedded on a Texas Instruments microcontroller
or used solely and exclusively in conjunction with a Texas Instruments radio
frequency transceiver, which is integrated into your product.  Other than for
the foregoing purpose, you may not use, reproduce, copy, prepare derivative
works of, modify, distribute, perform, display or sell this Software and/or
its documentation for any purpose.

YOU FURTHER ACKNOWLEDGE AND AGREE THAT THE SOFTWARE AND DOCUMENTATION ARE
PROVIDED ¡°AS IS¡± WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED,
INCLUDING WITHOUT LIMITATION, ANY WARRANTY OF MERCHANTABILITY, TITLE,
NON-INFRINGEMENT AND FITNESS FOR A PARTICULAR PURPOSE. IN NO EVENT SHALL
TEXAS INSTRUMENTS OR ITS LICENSORS BE LIABLE OR OBLIGATED UNDER CONTRACT,
NEGLIGENCE, STRICT LIABILITY, CONTRIBUTION, BREACH OF WARRANTY, OR OTHER
LEGAL EQUITABLE THEORY ANY DIRECT OR INDIRECT DAMAGES OR EXPENSES
INCLUDING BUT NOT LIMITED TO ANY INCIDENTAL, SPECIAL, INDIRECT, PUNITIVE
OR CONSEQUENTIAL DAMAGES, LOST PROFITS OR LOST DATA, COST OF PROCUREMENT
OF SUBSTITUTE GOODS, TECHNOLOGY, SERVICES, OR ANY CLAIMS BY THIRD PARTIES
(INCLUDING BUT NOT LIMITED TO ANY DEFENSE THEREOF), OR OTHER SIMILAR COSTS.

Should you have any questions regarding your right to use this Software,
contact Texas Instruments Incorporated at www.TI.com.
 **************************************************************************************************/

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
  u8 mode;       /* Operation mode */
  u8 todo;       /* Blink cycles left */
  u8 onPct;      /* On cycle percentage */
  u32 time;      /* On/off cycle time (msec) */
  u32 next;      /* Time for next change */
  u32 step;
  u32 raw_time;
  u8 bright;
} HalLedControl_t;

typedef struct
{
  HalLedControl_t HalLedControlTable[MAX_LEDS];
  u8           sleepActive;
} HalLedStatus_t;

_attribute_data_retention_ static hal_led_arry_t hal_led;
/***************************************************************************************************
 *                                           GLOBAL VARIABLES
 ***************************************************************************************************/


static u32 HalLedState;              // LED state at last set/clr/blink update

#if HAL_LED == TRUE
_attribute_data_retention_ static u8 HalSleepLedState;         // LED state at last set/clr/blink update
_attribute_data_retention_ static u32 preBlinkState;            // Original State before going to blink mode
// bit 0, 1, 2, 3 represent led 0, 1, 2, 3
#endif

#ifdef BLINK_LEDS
_attribute_data_retention_ static HalLedStatus_t HalLedStatusControl;
#endif

/***************************************************************************************************
 *                                            LOCAL FUNCTION
 ***************************************************************************************************/
#if (HAL_LED == TRUE)
int HalLedUpdate (void *data);
void HalLedOnOff (u32 leds, u8 mode);
void HalLedbreathe (u32 leds, u8 tim);
#endif /* HAL_LED */

void register_led(const hal_led_t *led, u8 num)
{
  hal_led.led = led;
  hal_led.num = num;
}

/***************************************************************************************************
 *                                            FUNCTIONS - API
 ***************************************************************************************************/

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
#if (HAL_LED == TRUE)
  /* Initialize all LEDs to OFF */
  HalLedSet (HAL_LED_ALL, HAL_LED_MODE_OFF);
#endif /* HAL_LED */
#ifdef BLINK_LEDS
  /* Initialize sleepActive to FALSE */
  HalLedStatusControl.sleepActive = FALSE;
#endif
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
#if (defined (BLINK_LEDS)) && (HAL_LED == TRUE)
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

#elif (HAL_LED == TRUE)
  LedOnOff(leds, mode);
#else
  // HAL LED is disabled, suppress unused argument warnings
  (void) leds;
  (void) mode;
#endif /* BLINK_LEDS && HAL_LED   */

  return ( HalLedState );

}

void HalLedBreathe (u32 leds, u8 numBreathe, u32 step, u32 period)
{
#if (defined (BREATHE_LEDS)) && (HAL_LED == TRUE)
  u32 led;
  HalLedControl_t *sts;

  if (leds && step && period)
  {
    led = HAL_LED_1;
    leds &= HAL_LED_ALL;
    sts = HalLedStatusControl.HalLedControlTable;
    while (leds)
    {
      if (leds & led)
      {
        /* Store the current state of the led before going to blinking if not already blinking */
        if(sts->mode < HAL_LED_MODE_BLINK )
          preBlinkState |= (led & HalLedState);

        sts->mode      = HAL_LED_MODE_OFF;                    /* Stop previous blink */
        sts->bright    = 0;
        sts->time      = period;                              /* Time for one on/off cycle */
        sts->raw_time  = period;
        sts->step      = step;                             /* % of cycle LED is on */
        sts->todo      = numBreathe;                           /* Number of blink cycles */
        if (!numBreathe) sts->mode |= HAL_LED_MODE_CONTINUE_BREATHE;  /* Continuous */
        //  sts->next = osal_GetSystemClock();                /* Start now */
        sts->next  = clock_time();
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
#elif (HAL_LED == TRUE)
  percent = (leds & HalLedState) ? HAL_LED_MODE_OFF : HAL_LED_MODE_ON;
  HalLedOnOff (leds, percent);                              /* Toggle */
#else
  // HAL LED is disabled, suppress unused argument warnings
  (void) leds;
  (void) numBlinks;
  (void) percent;
  (void) period;
#endif /* BLINK_LEDS && HAL_LED */
}

int HalLedUpdateBreath (void *data)
{
  u32 led;
  u32 leds;
  u32 pct = 0;
  u32 time = 0;
  u32 next;
  u32 wait;
  u8 iter;
  u32 us = 0;
  HalLedControl_t *sts;

  next = 0;
  led  = HAL_LED_1;
  leds = HAL_LED_ALL;

  sts = HalLedStatusControl.HalLedControlTable;
  while (leds)
  {
    if (leds & led)
    {
      if (sts->mode & HAL_LED_MODE_BREATHE)
      {
        time = clock_time();
        if (time >= sts->next)
        {
          iter = sts->time/sts->step;

          if(sts->time)
          {
            pct = sts->step;

            if(sts->bright > LED_BRIGHT_LEVEL)
              sts->bright = LED_BRIGHT_LEVEL;
            HalLedbreathe (led, sts->bright++);
            sts->time -= sts->step;
          }
          else if(!(sts->mode & HAL_LED_MODE_CONTINUE_BREATHE))
          {
            sts->todo--;
            sts->time = sts->raw_time;

            if(!sts->todo)
            {
              sts->mode ^= HAL_LED_MODE_BREATHE;      /* No more blinks */
            }
          }
          else
          {
            sts->time = sts->raw_time;
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
            preBlinkState &= (led ^ 0xFF);
          }
        }
        else
        {
          wait = sts->next - time;  /* Time left */
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

  if (next)
  {
    us = TICK2US(next);

  }

  return us;
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
#if (defined (BLINK_LEDS)) && (HAL_LED == TRUE)
  u32 led;
  HalLedControl_t *sts;

  if (leds && percent && period)
  {
    if (percent <= 100)
    {
      led = HAL_LED_1;
      leds &= HAL_LED_ALL;
      sts = HalLedStatusControl.HalLedControlTable;

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
          //  sts->next = osal_GetSystemClock();                /* Start now */
          sts->next = clock_time();
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
#elif (HAL_LED == TRUE)
  percent = (leds & HalLedState) ? HAL_LED_MODE_OFF : HAL_LED_MODE_ON;
  HalLedOnOff (leds, percent);                              /* Toggle */
#else
  // HAL LED is disabled, suppress unused argument warnings
  (void) leds;
  (void) numBlinks;
  (void) percent;
  (void) period;
#endif /* BLINK_LEDS && HAL_LED */
}

#if (HAL_LED == TRUE)
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
  u8 pct = 0;
  u32 leds;
  HalLedControl_t *sts;
  u32 time = 0;
  u32 next;
  u32 wait;
  u32 us = 0;

  next = 0;
  led  = HAL_LED_1;
  leds = HAL_LED_ALL;
  sts = HalLedStatusControl.HalLedControlTable;

  while (leds)
  {
    if (leds & led)
    {
      if (sts->mode & HAL_LED_MODE_BLINK)
      {
        time = clock_time();
        if (time >= sts->next)
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
            //  wait = (((unsigned long )pct * (unsigned long)sts->time) / 100);
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
            preBlinkState &= (led ^ 0xFF);
          }
        }
        else
        {
          wait = sts->next - time;  /* Time left */
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

  if (next)
  {
    us = TICK2US(next);
  }

  return us;
}

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
    HalLedState &= (leds ^ 0xFF);
  }
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
    HalLedState &= (leds ^ 0xFF);
  }
}
#endif /* HAL_LED */

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
#if (HAL_LED == TRUE)
  return HalLedState;
#else
  return 0;
#endif
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
#ifdef BLINK_LEDS
  /* Sleep ON */
  HalLedStatusControl.sleepActive = TRUE;
#endif /* BLINK_LEDS */

#if (HAL_LED == TRUE)
  /* Save the state of each led */
  //  HalSleepLedState = 0;
  //  HalSleepLedState |= HAL_STATE_LED1();
  //  HalSleepLedState |= HAL_STATE_LED2() << 1;
  //  HalSleepLedState |= HAL_STATE_LED3() << 2;
  //  HalSleepLedState |= HAL_STATE_LED4() << 3;

  /* TURN OFF all LEDs to save power */
  HalLedOnOff (HAL_LED_ALL, HAL_LED_MODE_OFF);
#endif /* HAL_LED */

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
#if (HAL_LED == TRUE)
  /* Load back the saved state */
  HalLedOnOff(HalSleepLedState, HAL_LED_MODE_ON);

  /* Restart - This takes care BLINKING LEDS */
  HalLedUpdate(NULL);
#endif /* HAL_LED */

#ifdef BLINK_LEDS
  /* Sleep OFF */
  HalLedStatusControl.sleepActive = FALSE;
#endif /* BLINK_LEDS */
}

/***************************************************************************************************
 ***************************************************************************************************/
#endif



