#include "DS1307.h"

#define DS1307_SELF_ADDRESS     0xA0    // DS1307 slave address
#define DS1307_WRITE_ADDRESS    0xD0    // DS1307 slave write address
#define DS1307_READ_ADDRESS     0xD1    // make LSB bit high of slave address for read

#define DS1307_TIME_START_ADDRESS 0
#define DS1307_DATE_START_ADDRESS 3

#define DS1307_MINUTES_ADDRESS  1
#define DS1307_HOURS_ADDRESS    2
#define DS1307_WEEK_DAY_ADDRESS 3
#define DS1307_DAY_ADDRESS      4
#define DS1307_MONTH_ADDRESS    5
#define DS1307_YEAR_ADDRESS     6

#define DS1307_HOUR_FORMAT_BIT 6
#define DS1307_AM_PM_BIT       5

static I2C_Polling i2c = {0};

static bool isInitialDateTimeInvalid(LocalDateTime *dateTimeInstance);
static inline uint8_t parseReceivedData(uint8_t dataBits);
static inline uint8_t parseOutputData(uint8_t dataBits);
static inline uint8_t parseYear(uint16_t year);
static inline HourFormat getHourFormat(uint8_t hours);
static inline AmPm getHourAmPm(uint8_t hours);
static inline uint8_t removeUnusedBitsInReceivedHours(uint8_t hours);
static inline uint8_t set24HourFormat(uint8_t hours);
static inline uint8_t set12HourFormat(uint8_t hours, AmPm amPm);


LocalDateTime *initDS1307(I2C_TypeDef *I2Cx) {
    i2c = initI2C(I2Cx, I2C_ADDRESSING_MODE_7BIT, DS1307_I2C_TIMEOUT);

    if (isDeviceReady(&i2c, DS1307_SELF_ADDRESS) != I2C_OK) {    // Check if we have the correct connection with our RTC
        return NULL;
    }
    LocalDate *dateInstance = malloc(sizeof(LocalDate));
    LocalTime *timeInstance = malloc(sizeof(LocalTime));
    LocalDateTime *dateTimeInstance = malloc(sizeof(LocalDateTime));

    if (dateInstance == NULL || timeInstance == NULL || dateTimeInstance == NULL) {
        return NULL;
    }
    dateInstance->day = 1;
    dateInstance->month = JANUARY;
    dateInstance->year = 2000;
    dateInstance->weekDay = SUNDAY;

    timeInstance->hours = 0;
    timeInstance->minutes = 0;
    timeInstance->seconds = 0;
    timeInstance->hourFormat = HOUR_FORMAT_24;
    timeInstance->amPm = AM;

    dateTimeInstance->date = dateInstance;
    dateTimeInstance->time = timeInstance;

    getDateTimeDS1307(dateTimeInstance);// First RTC start can contain invalid date time values. So the clock can't be started, need set initial time
    if (isInitialDateTimeInvalid(dateTimeInstance)) {
        setTimeDS1307(0, 0, 0); // set initial date and time
        setDateDS1307(FIRST_DAY_IN_MONTH, JANUARY, MIN_YEAR);
    }

    return dateTimeInstance;
}

void setTimeDS1307(uint8_t hours, uint8_t minutes, uint8_t seconds) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {    // start I2C communication with RTC
        transmitByteAsMasterI2C(&i2c, DS1307_TIME_START_ADDRESS);                       // write 0 address for second
        transmitByteAsMasterI2C(&i2c, parseOutputData(seconds));                        // write second on 00 location
        transmitByteAsMasterI2C(&i2c, parseOutputData(minutes));                        // write minutes on 01(auto increment) location
        transmitByteAsMasterI2C(&i2c, set24HourFormat(hours));                          // write hours on 02 location
        stopAsMasterI2C(&i2c);                                                          // stop I2C communication
    }
}

void setTime12hFormatDS1307(uint8_t hours, uint8_t minutes, uint8_t seconds, AmPm amPm) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {    // start I2C communication with RTC
        transmitByteAsMasterI2C(&i2c, DS1307_TIME_START_ADDRESS);                       // write 0 address for second
        transmitByteAsMasterI2C(&i2c, parseOutputData(seconds));                        // write second on 00 location
        transmitByteAsMasterI2C(&i2c, parseOutputData(minutes));                        // write minutes on 01(auto increment) location
        transmitByteAsMasterI2C(&i2c, set12HourFormat(hours, amPm));                   // write hours on 02 location
        stopAsMasterI2C(&i2c);                                                        // stop I2C communication
    }
}

void setDateDS1307(uint8_t day, Month month, uint16_t year) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {    // start I2C communication with RTC
        transmitByteAsMasterI2C(&i2c, DS1307_DATE_START_ADDRESS);            // write 3 address for day
        transmitByteAsMasterI2C(&i2c, getDayOfWeek(day, month, year));        // write week day on 03 location
        transmitByteAsMasterI2C(&i2c, parseOutputData(day));                // write date on 04 location
        transmitByteAsMasterI2C(&i2c, parseOutputData(month));                // write month on 05 location
        transmitByteAsMasterI2C(&i2c, parseYear(year));                        // write year on 06 location
        stopAsMasterI2C(&i2c);
    }
}

void setHoursIn24hFormatDS1307(uint8_t hours) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {
        transmitByteAsMasterI2C(&i2c, DS1307_HOURS_ADDRESS);
        transmitByteAsMasterI2C(&i2c, set24HourFormat(hours));              // write hours on 02 location
        stopAsMasterI2C(&i2c);                                              // stop I2C communication
    }
}

void setHoursIn12hFormatDS1307(uint8_t hours, AmPm amPm) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {
        transmitByteAsMasterI2C(&i2c, DS1307_HOURS_ADDRESS);
        transmitByteAsMasterI2C(&i2c, set12HourFormat(hours, amPm));       // write hours on 02 location
        stopAsMasterI2C(&i2c);                                               // stop I2C communication
    }
}

void setMinutesDS1307(uint8_t minutes) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {
        transmitByteAsMasterI2C(&i2c, DS1307_MINUTES_ADDRESS);
        transmitByteAsMasterI2C(&i2c, parseOutputData(minutes));         // write minutes on 01(auto increment) location
        stopAsMasterI2C(&i2c);                                             // stop I2C communication
    }
}

void setDayDS1307(uint8_t day) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {
        transmitByteAsMasterI2C(&i2c, DS1307_DAY_ADDRESS);
        transmitByteAsMasterI2C(&i2c, parseOutputData(day));            // write date on 04 location
        stopAsMasterI2C(&i2c);                                            // stop I2C communication
    }
}

void setMonthDS1307(uint8_t month) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {
        transmitByteAsMasterI2C(&i2c, DS1307_MONTH_ADDRESS);
        transmitByteAsMasterI2C(&i2c, parseOutputData(month));        // write moth on 05 location
        stopAsMasterI2C(&i2c);                                         // stop I2C communication
    }
}

void setYearDS1307(uint16_t year) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {
        transmitByteAsMasterI2C(&i2c, DS1307_YEAR_ADDRESS);
        transmitByteAsMasterI2C(&i2c, parseYear(year));            // write year on 06 location
        stopAsMasterI2C(&i2c);                                       // stop I2C communication
    }
}

void setWeekDayDS1307(WeekDay weekDay) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) {
        transmitByteAsMasterI2C(&i2c, DS1307_WEEK_DAY_ADDRESS);
        transmitByteAsMasterI2C(&i2c, weekDay);                       // write year on 06 location
        stopAsMasterI2C(&i2c);                                          // stop I2C communication
    }
}

void setWeekDayByDateDS1307(uint8_t day, uint8_t month, uint16_t year) {
    setWeekDayDS1307(getDayOfWeek(day, month, year));
}

void getTimeDS1307(LocalTime *time) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) { // start I2C communication with RTC and check connection status
        transmitByteAsMasterI2C(&i2c, DS1307_TIME_START_ADDRESS);                        // write 0 address for second
        startAsMasterI2C(&i2c, DS1307_READ_ADDRESS, I2C_READ_FROM_SLAVE);               // start I2C communication with RTC

        uint8_t seconds = 0;
        receiveByteAsMasterI2C(&i2c, &seconds);         // read seconds
        uint8_t minutes = 0;
        receiveByteAsMasterI2C(&i2c, &minutes);            // read minutes
        uint8_t hours = 0;
        receiveByteAsMasterWithNackI2C(&i2c, &hours);   // read hour with Nack, stop I2C communication

        time->seconds = parseReceivedData(seconds);
        time->minutes = parseReceivedData(minutes);
        time->hourFormat = getHourFormat(hours);
        time->amPm = getHourAmPm(hours);
        time->hours = parseReceivedData(removeUnusedBitsInReceivedHours(hours));// convert received hours to decimal value
    }
}

void getDateDS1307(LocalDate *date) {
    if (startAsMasterI2C(&i2c, DS1307_WRITE_ADDRESS, I2C_WRITE_TO_SLAVE) == I2C_OK) { // start I2C communication with RTC and check connection status
        transmitByteAsMasterI2C(&i2c, DS1307_DATE_START_ADDRESS);                        // write 3 address for day
        startAsMasterI2C(&i2c, DS1307_READ_ADDRESS, I2C_READ_FROM_SLAVE);               // start I2C communication with RTC

        uint8_t weekDay = 0;
        receiveByteAsMasterI2C(&i2c, &weekDay);     // read week day
        uint8_t day = 0;
        receiveByteAsMasterI2C(&i2c, &day);         // read day
        uint8_t month = 0;
        receiveByteAsMasterI2C(&i2c, &month);       // read month
        uint8_t year = 0;                           // year is stored in short format
        receiveByteAsMasterWithNackI2C(&i2c, &year);// read the year with Nack, stop I2C communication

        date->weekDay = weekDay;
        date->day = parseReceivedData(day);
        date->month = parseReceivedData(month);
        date->year = MIN_YEAR + parseReceivedData(year);
    }
}

void getDateTimeDS1307(LocalDateTime *dateTime) {
    getDateDS1307(dateTime->date);
    getTimeDS1307(dateTime->time);
}

void deleteDS1307(LocalDateTime *dateTime) {
    if (dateTime != NULL) {
        free(dateTime->date);
        free(dateTime->time);
        free(dateTime);
    }
}

static bool isInitialDateTimeInvalid(LocalDateTime *dateTimeInstance) {
    bool isInitialSecondsInvalid = dateTimeInstance->time->seconds > (SECONDS_IN_MINUTE - 1);
    bool isInitialDayInvalid = dateTimeInstance->date->day < 1;
    bool isInitialMonthInvalid = dateTimeInstance->date->month < 1;
    return isInitialSecondsInvalid || isInitialDayInvalid || isInitialMonthInvalid;
}

static inline uint8_t parseReceivedData(uint8_t dataBits) {
    return (((dataBits & 0xF0) >> 4) * 10) + (dataBits & 0x0F);        // parse incoming bits for correct decimal representation. Remove highest bit with mask(0b01110000) to prevent incorrect data store
}

static inline uint8_t parseOutputData(uint8_t dataBits) {
    return ((dataBits / 10) << 4) | (dataBits % 10);    // parse hours and place bits, Example: 23 -> parsed to 2 and 3, 2 is placed at Bit5 and Bit4, lower part is placed from 0-3 bits
}

static inline uint8_t parseYear(uint16_t year) {
    return parseOutputData(year % 100);                // get only last two year digits
}

static inline HourFormat getHourFormat(uint8_t hours) {
    return READ_BIT(hours, DS1307_HOUR_FORMAT_BIT) == SET ? HOUR_FORMAT_12 : HOUR_FORMAT_24;
}

static inline AmPm getHourAmPm(uint8_t hours) {
    return READ_BIT(hours, DS1307_AM_PM_BIT) == SET ? PM : AM;
}

static inline uint8_t removeUnusedBitsInReceivedHours(uint8_t hours) {
    if (getHourFormat(hours) == HOUR_FORMAT_12) {
        hours &= ~(1 << DS1307_AM_PM_BIT);                // clear AM/PM bit if hours format is 12h for 24h this bit represents 2
    }
    hours &= ~(1 << DS1307_HOUR_FORMAT_BIT);        // clear hours format bit
    return hours;
}

static inline uint8_t set24HourFormat(uint8_t hours) {
    hours = hours & 0x1F;    // clean unused bits for 24h format
    return parseOutputData(hours);
}

static inline uint8_t set12HourFormat(uint8_t hours, AmPm amPm) {
    hours = hours & 0x0F;                        // clean unused bits for 12h format
    uint8_t resultValue = parseOutputData(hours);
    resultValue |= (1 << DS1307_HOUR_FORMAT_BIT);

    if (amPm == PM) {
        resultValue |= (1 << DS1307_AM_PM_BIT);
    } else {
        resultValue &= ~(1 << DS1307_AM_PM_BIT);
    }
    return resultValue;
}