#include "esp32.h"
#include <Arduino.h>
#include "vm.h"

#ifdef ESP32
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WebServer.h>
#include <ESPmDNS.h>

#pragma region PWM
extern "C" void pwmStart(struct PwmConfig* config) {
    auto err = ledcSetup(config->channel, config->freq, config->resolution);
    Serial.print("ledcSetup=");
    Serial.println(err);
    ledcAttachPin(config->pin, config->channel);
    ledcWrite(config->channel, config->duty);
}

extern "C" void pwmSetup(struct PwmConfig* config) {
    ledcWrite(config->channel, config->duty);
}

extern "C" void pwmStop(struct PwmConfig* config) {
    ledcWrite(config->channel, 0);
}
#pragma endregion

WebServer server(80);
VM *espState = nullptr;
PwmConfig espPwmConfig;

const char HTML_MAIN[] PROGMEM = R"(
<!DOCTYPE html><html><head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        html,body {height:100%;margin:0;background-color:#f9f9f9;}
        .box {display:flex;flex-flow:column;padding:10pt;margin:10pt;}
        .sec {display:flex;width:100%;font-family:Arial, Helvetica, sans-serif;font-size:30pt;padding:7pt;align-items:center;gap:7pt;color:#444;}
        .ttl {display:flex;width:32pt;height:32pt;font-size:12pt;border:2px solid #444;border-radius:7px;justify-content:center;align-items:center;box-shadow: 2px 2px 2px 2px #eed;}
        .slider {width:100%;background:#ddd;accent-color:#888;}
    </style>
    <script>
    </script>
</head>
<body>
    <div class="box">
        <div class="sec">
            <div class="ttl">PWM</div><span id="valuePwm">127</span>
        </div>
        <input type="range" min="0" max="8192" value="4096" class="slider" id="sliderPwm">
        <div class="sec">
            <div class="ttl">V</div><span id="valueV">0.00</span> V
        </div>
        <div class="sec">
            <div class="ttl">A</div><span id="valueA">0.00</span> A
        </div>
    </div>
    <script>
        const slider = document.getElementById("sliderPwm");
        const output = document.getElementById("valuePwm");
        slider.addEventListener("input", function() {
            output.textContent = Math.floor(this.value/32);
        });
    </script>
</body></html>
)";

void handle_OnConnect()
{
    server.send(200, "text/html", HTML_MAIN);
}

void handle_NotFound()
{
    server.send(404, "text/plain", "Not found");
}

void handle_PwmOff()
{
    for (uint8_t i = 0; i < server.args(); i++)
    {
        if (server.argName(i).equals("channel"))
        {
            espPwmConfig.channel = atoi(server.arg(i).c_str());
            vmOnPwmEnd(&espPwmConfig);
            break;
        }
    }
    server.send(200, "text/plain", "OK");
}

void handle_PwmSet()
{
    int8_t channel = -1;
    int32_t duty = -1;
    for (uint8_t i = 0; i < server.args(); i++)
    {
        if (server.argName(i).equals("channel"))
            channel = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("duty"))
            duty = atoi(server.arg(i).c_str());
    }
    if (channel > -1 && duty > -1) {
        espPwmConfig.channel = channel;
        espPwmConfig.duty = duty;
        vmOnPwmUpdate(&espPwmConfig);
        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missed argument: channel,duty");
    }
}

void handle_PwmOn()
{
    int8_t channel = -1;
    int32_t freq = -1;
    int8_t resolution = -1;
    int8_t pin = -1;
    int16_t duty = 0;
    for (uint8_t i = 0; i < server.args(); i++)
    {
        if (server.argName(i).equals("channel"))
            channel = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("freq"))
            freq = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("res"))
            resolution = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("pin"))
            pin = atoi(server.arg(i).c_str());
        else if (server.argName(i).equals("duty"))
            duty = atoi(server.arg(i).c_str());
    }
    if (channel > -1 && freq > -1 && resolution > -1 && pin > -1) {
        Serial.print("Enable PWM on ");
        Serial.print(channel);
        Serial.print(" freq=");
        Serial.print(freq);
        Serial.print(" duty=");
        Serial.print(duty);
        Serial.print(" res=");
        Serial.println(resolution);

        espPwmConfig.channel = channel;
        espPwmConfig.freq = freq;
        espPwmConfig.resolution = resolution;
        espPwmConfig.pin = pin;
        espPwmConfig.duty = duty;

        vmOnPwmStart(&espPwmConfig);
        server.send(200, "text/plain", "OK");
    } else {
        server.send(400, "text/plain", "Missed argument: channel,freq,res,pin");
    }
}


void esp32Init(struct VM *state)
{
    Serial.begin(9600);
    espState = state;
    WiFiManager wm;
    bool res = wm.autoConnect("ESPPower", "E$PP0w4r"); // password protected ap
    if (!res)
    {
        Serial.println("Failed to connect");
        // ESP.restart();
    }
    else
    {
        // if you get here you have connected to the WiFi
        Serial.println("connected...yeey :)");
    }

    if (!MDNS.begin("ESPPower"))
    {
        Serial.println("Error starting mDNS");
    }
    MDNS.addService("esppower", "tcp", 80);

    server.on("/", handle_OnConnect);
    server.on("/pwmon", handle_PwmOn);
    server.on("/pwmset", handle_PwmSet);
    server.on("/pwmoff", handle_PwmOff);
    server.onNotFound(handle_NotFound);
    server.begin();
}

void esp32Loop(struct VM *state)
{
    server.handleClient();
}
#endif