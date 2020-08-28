# WeatherStation

The device displays on the screen the current temperature in celcium, atmospheric pressure in kPa and some data from the weather forecasts. 
To do this, it uses the BMP280 sensor, and also request weather reports METAR(actual value) and TAF (forecast for 5 hours) via HTTP requests via server  https://avwx.rest/.

Upon request from the smartphone app Serial Bluetooth Terminal, the device sends information on temperature, pressure and forecast to the smartphone. 

Bluetooth commands for smartphone app:
 - bmp280 all   - sends temperature, pressure and altitude. 
 - bmp280 temp  - sends temperature. 
 - bmp280 press - sends pressure. 
 - bmp280 alt   - sends altitude. 
 - metar/Metar/METAR - sends data from the METAR report: temperature, dewpoint, pressure, wind direction, wind speed, visibility.
 - taf/Taf/TAF - sends data from the TAF report: time of issue, wind direction, wind speed, visibility.
 
The device works using FreeRTOS. Four threads work:
 - thread_lcd to display actual information on the screen. 
 - thread_sensors for periodic reading of sensor data.
 - thread_btooth for receiving, processing and sending data to a smartphone via bluetooth.
 - thread_reports for weather reports via Internet connection.
 
The Sensors thread sends the read data to the Queue(for the Display thread).
The LCD thread receives the actual data from the Queue and draws it to the screen,
in addition, writes data to a structure that is globally accessible (for the Bluetooth stream). Writing to the structure is wrapped in a mutex. 
When receiving a command from a smartphone, the Bluetooth thread response using data from the global structure. Reading the structure is wrapped in a mutex.
The Reports stream periodically requests data from the weather reporting service for the UKKK airport (Zhulyany), receives data in JSON format. 
The received JSON parses using the cJSON. The extracted data is sent to the Queue (for the Display stream).

# Hardware 

 - Nucleo F446RE devboard from STM.
 - MCUfriend shield LCD.
 - ESP8266 wifi module.
 - BMP280 pressure and temperature sensor.
 - Bluetooth module HC-06
 
# Using

Software:
1. Find file internet.h from Core/Inc.
2. Enter your server token (can get it via  https://avwx.rest/) -  #define SERVER_TOKEN. 
3. Enter name of your WiFi network #define WIFI_SSID and password #define WIFI_PASS.	
4. Compiler the project and flash the Nucleo F446RE using STM32CubeIDE. 
	
Hardware:
1. Place the screen LCD on the Ardruino's connetor of the Nucleo F446RE.
2. Connect ESP8266 WiFi module to PC10(UART4_TX), PC11(UART4_RX), power and ground.
3. Connect BMP280 pressure and temperature sensor to PB9(I2C1_SDA), PB8(I2C1_SCL), power and ground.
4. Connect Bluetooth module to PC12(UART5_TX), PD2(UART5_RX), power and ground.
5. Use console application, for example putty or YAT, for debug mode. Application UART settings 115200-8-N-1.

# Example

Demo video https://www.youtube.com/watch?v=OsyIRgUQyG8&t=13s 


