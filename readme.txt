miniservsmartsf.ino

Mini Smart Server using the SparkFun Driver Library

This Arduino WiFi server sketch has been tested using a SparkFun Blackboard Arduino board and a SparkFun WiFi Shield (ESP8266) with an external entenna. It was developed to be able to interpret simple client requests. The code connects to WiFi and initializes the server to wait for client requests. It prints TCP/IP connection data and the ESP8266 MAC address. It turns out that each ESP8266 device has a unique MAC. This is helpful since the MAC can be used in the WiFi router to associate the device with a fixed IP address. The client on the network can always address the server using this fixed IP address.

The tricky part for me was reading the request string from the ESP8266. The readData function accomplishes this, prints the result, and shows how parameters are passed to the server. This is the type of request that is handled: "http://192.168.1.101?opt=2". The result of readData can be parsed to decode the parameters, control the server functions, and return corresponding results. This sketch returns the text "Hello, Pi".

A Raspberry Pi was used as the client with the Requests library used to send the requests to the Arduino server.