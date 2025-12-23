class EspRepo {
    constructor() {
        this.wsDataCb = null;
        this.ws = new WebSocket(`ws://${window.location.host}/ws`);
        this.ws.onopen = () => {console.log("WebSocket connected");};
        this.ws.onmessage = (event) => {
            // console.log("WebSocket message received:", event.data);
            if (this.wsDataCb != null) { this.wsDataCb(JSON.parse(event.data)); }
        };
        this.ws.onclose = () => console.log("WebSocket disconnected");
    }
    sendPwmValue(channel,duty) {
        const xhr = new XMLHttpRequest();
        xhr.open('GET','./pwmset?channel='+channel+'&duty=' + duty,true);
        xhr.send();
    }
    setWsCallback(callback) { this.wsDataCb = callback; }
    getState() {
        return new Promise((resolve, reject) => {
        const xhr = new XMLHttpRequest();
        xhr.open('GET','./pwmget',true);
        xhr.onreadystatechange = function () {
            if (xhr.readyState===4) {
                if (xhr.status>=200 && xhr.status<300) {
                    resolve(xhr.responseText);
                } else {
                    reject(new Error(`Request failed with status ${xhr.status}`));
                }
            }
        };
        xhr.onerror = () => reject(new Error('Network error'));
        xhr.send();
    });
    }
}

class State{ 
    constructor(pwmValue, pwmEnabled, volts, current) {
        this.pwmValue = pwmValue;
        this.pwmEnabled = pwmEnabled;
        this.volts = volts;
        this.current = current;
    }
    copyPwmValue(value) { return new State(value, this.pwmEnabled, this.volts, this.current); }
    copyPwmEnabled(value) { return new State(this.pwmValue, value, this.volts, this.current); }
    copyVoltsCurrent(volts, current) { return new State(this.pwmValue, this.pwmEnabled, volts, current); }
}

class ViewModel {
    constructor(espRepo, updateUi) {
        this.repeatTimeMs = 100;
        this.delayTimeMs = 1200;
        this.sendPwmTimeMs = 500;
        this.state = new State(255, false);
        this.espRepo = espRepo;
        this.updateUi = updateUi;
        this.espRepo.getState().then(response => {
            const data=JSON.parse(response);
            this.state = new State(data.duty, data.work == 1);
            this.updateUi(this.state);
        })
        .catch(error => { console.error('Request error:', error); })
        .finally(() => { });
        this.espRepo.setWsCallback(data => {
            this.state = this.state.copyVoltsCurrent(data.voltage, data.current);
            this.updateUi(this.state);
        })
    }

    onPwmClick() {
        this.state = this.state.copyPwmEnabled(!this.state.pwmEnabled);
        const xhr = new XMLHttpRequest();
        if (this.state.pwmEnabled)
            xhr.open('GET','./pwmon?channel=0&freq=20000&res=8&pin=1&duty=' + 
                this.state.pwmValue, true);
        else 
            xhr.open('GET','./pwmoff?channel=0', true);

        xhr.onreadystatechange = () => {
            if (xhr.readyState===4) {
                if (xhr.status>=200 && xhr.status<300) {
                    //resolve(xhr.responseText);
                    this.updateUi(this.state);
                } else {
                    reject(new Error(`Request failed with status ${xhr.status}`));
                }
            }
        };
        xhr.onerror = () => reject(new Error('Network error'));
        xhr.send();
    }

    onSetPwm(value, final) {
        if (value < 0) value = 0;
        if (value > 255) value = 255;
        if (value == this.state.pwmValue && !final) return;
        this.state = this.state.copyPwmValue(value);
        const channel = 0;
        
        if (this.state.pwmEnabled) {
            if (final) {
                if (this.debounceTimer) {
                    clearInterval(this.debounceTimer);
                    this.debounceTimer = null;
                }
                this.espRepo.sendPwmValue(channel, this.state.pwmValue);
            } else {
                // Debounce non-final values
                if (this.debounceTimer == null) {
                    this.debounceTimer = setInterval(() => {
                        this.espRepo.sendPwmValue(channel, this.state.pwmValue);
                    }, this.sendPwmTimeMs);
                }
            }
        }
        
        this.updateUi(this.state);
    }

    onStartCalibration() {
        const xhr = new XMLHttpRequest();
        xhr.open('GET','./calibration',true);
        xhr.onreadystatechange = () => {
            if (xhr.readyState===4) {
                if (xhr.status>=200 && xhr.status<300) {
                    this.updateUi(this.state);
                } else {
                    console.error(`Request failed with status ${xhr.status}`);
                }
            }
        };
        xhr.onerror = () => console.error('Network error');
        xhr.send();
    }

    onReadPartitions() {
        const xhr = new XMLHttpRequest();
        xhr.open('GET','./api/read_partitions',true);
        xhr.onreadystatechange = () => {
            if (xhr.readyState===4) {
                if (xhr.status>=200 && xhr.status<300) {
                    console.log('Partitions:', xhr.responseText);
                } else {
                    console.error(`Request failed with status ${xhr.status}`);
                }
            }
        };
        xhr.onerror = () => console.error('Network error');
        xhr.send();
    }

    cleanPwmChangeTimers() {
        if (this.delayTimer != null) {
            clearTimeout(this.delayTimer);
            this.delayTimer = null;
        }
        if (this.repeatInterval != null) {
            clearInterval(this.repeatInterval);
            this.repeatInterval = null;
        }
    }

    onPwmValuePlusDown() {
        this.onSetPwm(this.state.pwmValue + 1, false);
        this.cleanPwmChangeTimers();
        this.delayTimer = setTimeout(() => {
            this.repeatInterval = setInterval(() => {
                this.onSetPwm(this.state.pwmValue + 1, false);
            }, this.repeatTimeMs);            
        }, this.delayTimeMs);
    }

    onPwmValuePlusUp() {
        this.cleanPwmChangeTimers();
        this.onSetPwm(this.state.pwmValue, true);
    }

    onPwmValueMinusDown() {
        this.onSetPwm(this.state.pwmValue - 1, false);
        this.cleanPwmChangeTimers();
        this.delayTimer = setTimeout(() => {
            this.repeatInterval = setInterval(() => {
                this.onSetPwm(this.state.pwmValue - 1, false);
            }, this.repeatTimeMs);            
        }, this.delayTimeMs);
    }

    onPwmValueMinusUp() {
        this.cleanPwmChangeTimers();
        this.onSetPwm(this.state.pwmValue, true);
    }
}
