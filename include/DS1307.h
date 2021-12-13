#pragma once

#include <stdlib.h>
#include "main.h"
#include "I2C_Polling.h"

#define DS1307_I2C_TIMEOUT 100

#define HOURS_IN_24H_FORMAT 24
#define HOURS_IN_12H_FORMAT 12
#define MINUTES_IN_HOUR		60
#define SECONDS_IN_MINUTE	60
#define SECONDS_IN_HOUR     3600
#define MIN_YEAR			2000
#define MAX_YEAR			2099
#define FIRST_DAY_IN_MONTH  1
#define DAYS_IN_WEEK		7

typedef enum HourFormat {
    HOUR_FORMAT_12,
    HOUR_FORMAT_24
} HourFormat;

typedef enum AmPm {
    AM, PM
} AmPm;

typedef enum WeekDay {
    SUNDAY    = 1,
    MONDAY    = 2,
    TUESDAY	  = 3,
    WEDNESDAY = 4,
    THURSDAY  = 5,
    FRIDAY    = 6,
    SATURDAY  = 7
} WeekDay;

typedef enum Month {
    JANUARY   = 1,
    FEBRUARY  = 2,
    MARCH	  = 3,
    APRIL	  = 4,
    MAY		  = 5,
    JUNE	  = 6,
    JULY	  = 7,
    AUGUST	  = 8,
    SEPTEMBER = 9,
    OCTOBER   = 10,
    NOVEMBER  = 11,
    DECEMBER  = 12
} Month;

typedef struct LocalDate {
    uint8_t day;
    Month month;
    uint16_t year;
    WeekDay weekDay;
} LocalDate;

typedef struct LocalTime {
    uint8_t hours;
    uint8_t minutes;
    uint8_t seconds;
    HourFormat hourFormat;
    AmPm amPm;
} LocalTime;

typedef struct LocalDateTime {
    LocalDate *date;
    LocalTime *time;
} LocalDateTime;


static inline const char *getWeekDayNameShort(WeekDay weekDay) {
    static const char *const WEEK_DAY_NAME_SHORT[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    return WEEK_DAY_NAME_SHORT[weekDay - 1];
}

static inline const char *getWeekDayNameLong(WeekDay weekDay) {
    static const char *const WEEK_DAY_NAME_LONG[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return WEEK_DAY_NAME_LONG[weekDay - 1];
}

static inline const char *getMonthNameShort(Month month) {
    static const char *const MONTH_NAME_SHORT[] = {"Jan", "Feb", "Mar", "Apr", "May", "Jun", "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};
    return MONTH_NAME_SHORT[month - 1];
}

static inline const char *getMonthNameLong(Month month) {
    static const char *const MONTH_NAME_LONG[] = {"January", "February", "March", "April", "May", "June", "July", "August", "September", "October", "November", "December"};
    return MONTH_NAME_LONG[month - 1];
}

static inline WeekDay getDayOfWeek(uint8_t day, uint8_t month, uint16_t year) {            // find day of a given date
    static const uint8_t time[] = {0, 3, 2, 5, 0, 3, 5, 1, 4, 6, 2, 4};        // predefined time constants
    year -= (month < 3);
    return ((year + (year / 4) - (year / 100) + (year / 400) + time[month - 1] + day) % 7) + 1;
}

static inline uint8_t getDaysInMonth(uint8_t month, uint16_t year) {
    if (year < 1528 || month > 12 || month < 1) {    // before this year the Gregorian Calendar was not defined
        return 0;
    } else if (month == 4 || month == 6 || month == 9 || month == 11) {
        return 30;
    } else if (month == 2) {
        return (((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0)) ? 29 : 28);
    } else {
        return 31;
    }
}


LocalDateTime *initDS1307(I2C_TypeDef *I2Cx);

void setTimeDS1307(uint8_t hours, uint8_t minutes, uint8_t seconds);
void setTime12hFormatDS1307(uint8_t hours, uint8_t minutes, uint8_t seconds, AmPm amPm);
void setDateDS1307(uint8_t day, Month month, uint16_t year);

void setHoursIn24hFormatDS1307(uint8_t hours);
void setHoursIn12hFormatDS1307(uint8_t hours, AmPm amPm);
void setMinutesDS1307(uint8_t minutes);
void setDayDS1307(uint8_t day);
void setMonthDS1307(uint8_t month);
void setYearDS1307(uint16_t year);
void setWeekDayDS1307(WeekDay weekDay);
void setWeekDayByDateDS1307(uint8_t day, uint8_t month, uint16_t year);

void getTimeDS1307(LocalTime *time);
void getDateDS1307(LocalDate *date);
void getDateTimeDS1307(LocalDateTime *dateTime);

const char *getWeekDayNameShort(WeekDay weekDay);
const char *getWeekDayNameLong(WeekDay weekDay);
WeekDay getDayOfWeek(uint8_t day, uint8_t month, uint16_t year);
uint8_t getDaysInMonth(uint8_t month, uint16_t year);

void deleteDS1307(LocalDateTime *dateTime);