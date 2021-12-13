# DS1307
*STM32* Low Layer(LL) library. DS1307 serial real-time clock (RTC) is a low power, full binary-coded decimal (BCD) clock/calendar
plus 56 bytes of NV SRAM.

<img src="https://github.com/ximtech/DS1307/blob/main/example/ds1307.PNG" alt="image" width="300"/>

### Features
- Power Supply from 3.3V to 5V DC
- Completely Manages All Timekeeping FunctionsReal-Time Clock Counts:
  - Seconds
  - Minutes
  - Hours
  - Date of the Month
  - Month
  - Day of the Week
  - Year with Leap-Year Compensation Valid Up to 2100
- 56-Byte, Battery-Backed, General-Purpose RAM with Unlimited Writes
- I2C Serial Interface
- Low Power Operation Extends Battery Backup Run Time
- Consumes Less than 500nA in Battery-Backup Mode with Oscillator Running
- Automatic Power-Fail Detect and Switch Circuitry

### Add as CPM project dependency

How to add CPM to the project, check the [link](https://github.com/cpm-cmake/CPM.cmake)
```cmake
CPMAddPackage(
        NAME DS1307
        GITHUB_REPOSITORY ximtech/DS1307
        GIT_TAG origin/main)
```

### Project configuration

1. Start project with STM32CubeMX:
    * [I2C configuration](https://github.com/ximtech/DS1307/blob/main/example/config.PNG)
2. Select: Project Manager -> Advanced Settings -> I2C -> LL
3. Generate Code
4. Add sources to project:

```cmake
add_subdirectory(${STM32_CORE_SOURCE_DIR}/I2C/Polling)  # add I2C dependency

include_directories(${includes} 
        ${DS1307_DIRECTORY})   # source directories

file(GLOB_RECURSE SOURCES ${sources} 
        ${DS1307_SOURCES})    # source files
```

3. Then Build -> Clean -> Rebuild Project

### Wiring

- <img src="https://github.com/ximtech/DS1307/blob/main/example/wiring.PNG" alt="image" width="300"/>

## Usage

- Usage example: [link](https://github.com/ximtech/DS1307/blob/main/example/example.c)