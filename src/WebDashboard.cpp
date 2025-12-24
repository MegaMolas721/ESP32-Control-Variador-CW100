#include "WebDashboard.h"

WebDashboard::WebDashboard(VFDController& vfd, IOManager& io, WiFiManager& wifi) : 
    _vfd(vfd),
    _io(io),
    _wifi(wifi),
    _server(80),
    _running(false) {
}

void WebDashboard::begin(uint16_t port) {
    Serial.println("\n=== INICIALIZANDO WEB DASHBOARD ===");
    Serial.printf("Puerto: %d\n", port);
    
    setupRoutes();
    _server.begin();
    _running = true;
    
    Serial.println("Web Dashboard iniciado");
    Serial.printf("Accede desde: http://%s\n", _wifi.getLocalIP().c_str());
}

bool WebDashboard::isRunning() {
    return _running;
}

void WebDashboard::setupRoutes() {
    // Página principal
    _server.on("/", HTTP_GET, [this](AsyncWebServerRequest *request){
        this->handleRoot(request);
    });
    
    // API: Obtener info WiFi
    _server.on("/api/wifi", HTTP_GET, [this](AsyncWebServerRequest *request){
        this->handleWiFiInfo(request);
    });
    
    // API: Desconectar WiFi
    _server.on("/api/wifi/disconnect", HTTP_POST, [this](AsyncWebServerRequest *request){
        this->handleWiFiDisconnect(request);
    });
    
    // API: Obtener estado variador
    _server.on("/api/status", HTTP_GET, [this](AsyncWebServerRequest *request){
        this->handleStatus(request);
    });
    
    // API: Enviar comando (start/stop/reset)
    _server.on("/api/command", HTTP_POST, [this](AsyncWebServerRequest *request){
        this->handleCommand(request);
    });
    
    // API: Configurar frecuencia
    _server.on("/api/frequency", HTTP_POST, [this](AsyncWebServerRequest *request){
        this->handleSetFrequency(request);
    });
    
    // API: Configurar velocidad por porcentaje (0-100%)
    _server.on("/api/speed", HTTP_POST, [this](AsyncWebServerRequest *request){
        if (request->hasParam("percent", true)) {
            float percent = request->getParam("percent", true)->value().toFloat();
            // El porcentaje se envía directamente al registro 1000H como percent × 100
            // El variador calcula la frecuencia según su configuración interna
            float freq = (percent / 100.0f) * 60.0f;  // Para mostrar al usuario
            _vfd.setFrequency(freq);  // Internamente convierte a porcentaje
            request->send(200, "application/json", "{\"status\":\"ok\",\"percent\":" + String(percent, 2) + ",\"frequency\":" + String(freq, 2) + "}");
        } else {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Falta parámetro percent\"}");
        }
    });
}

void WebDashboard::handleRoot(AsyncWebServerRequest *request) {
    request->send(200, "text/html", generateHTML());
}

void WebDashboard::handleWiFiInfo(AsyncWebServerRequest *request) {
    request->send(200, "application/json", generateWiFiInfoJSON());
}

void WebDashboard::handleWiFiDisconnect(AsyncWebServerRequest *request) {
    _wifi.disconnect();
    request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"WiFi desconectado. Para reconectar, reinicia el ESP32.\"}");
}

void WebDashboard::handleStatus(AsyncWebServerRequest *request) {
    request->send(200, "application/json", generateStatusJSON());
}

void WebDashboard::handleCommand(AsyncWebServerRequest *request) {
    if (request->hasParam("cmd", true)) {
        String cmd = request->getParam("cmd", true)->value();
        
        if (cmd == "start") {
            bool reverse = false;
            if (request->hasParam("reverse", true)) {
                String v = request->getParam("reverse", true)->value();
                if (v == "1" || v.equalsIgnoreCase("true")) reverse = true;
            }
            if (reverse) {
                _vfd.startInverse();
                request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Motor arrancado (inverso)\"}");
            } else {
                _vfd.start();
                request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Motor arrancado\"}");
            }
        }
        else if (cmd == "stop") {
            bool freeStop = false;
            if (request->hasParam("free", true)) {
                String v = request->getParam("free", true)->value();
                if (v == "1" || v.equalsIgnoreCase("true")) freeStop = true;
            }
            if (freeStop) {
                _vfd.freeStop();
                request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Motor detenido (free stop)\"}");
            } else {
                _vfd.stop();
                request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Motor detenido\"}");
            }
        }
        else if (cmd == "reset") {
            _vfd.resetFault();
            request->send(200, "application/json", "{\"status\":\"ok\",\"message\":\"Falla reseteada\"}");
        }
        else {
            request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Comando desconocido\"}");
        }
    } else {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Falta par\u00e1metro cmd\"}");
    }
}

void WebDashboard::handleSetFrequency(AsyncWebServerRequest *request) {
    if (request->hasParam("freq", true)) {
        float freq = request->getParam("freq", true)->value().toFloat();
        
        if (freq >= 0 && freq <= 400) {
            _vfd.setFrequency(freq);
            request->send(200, "application/json", 
                "{\"status\":\"ok\",\"message\":\"Frecuencia configurada\",\"frequency\":" + String(freq) + "}");
        } else {
            request->send(400, "application/json", 
                "{\"status\":\"error\",\"message\":\"Frecuencia fuera de rango (0-400 Hz)\"}");
        }
    } else {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Falta par\u00e1metro freq\"}");
    }
}

String WebDashboard::generateWiFiInfoJSON() {
    WiFiConnectionInfo info = _wifi.getConnectionInfo();
    
    String json = "{";
    json += "\"ssid\":\"" + info.ssid + "\",";
    json += "\"ip\":\"" + info.localIP + "\",";
    json += "\"gateway\":\"" + info.gateway + "\",";
    json += "\"subnet\":\"" + info.subnet + "\",";
    json += "\"dns\":\"" + info.dns + "\",";
    json += "\"rssi\":" + String(info.rssi) + ",";
    json += "\"connected\":" + String(info.isConnected ? "true" : "false") + ",";
    json += "\"connectionType\":\"" + String(info.usesDHCP ? "DHCP" : "Estática") + "\"";
    json += "}";
    
    return json;
}

String WebDashboard::generateStatusJSON() {
    const VFDData& data = _vfd.getData();
    uint16_t effectiveSetpoint = _vfd.getEffectiveSetpoint();
    
    String json = "{";
    json += "\"frequency\":" + String(data.frequencyActual / 100.0, 2) + ",";
    json += "\"current\":" + String(data.currentOut / 100.0, 2) + ",";
    json += "\"voltage\":" + String(data.voltageOut) + ",";
    json += "\"status\":" + String(data.status) + ",";
    json += "\"statusText\":\"" + _vfd.getStatusText() + "\",";
    json += "\"speedPercent\":" + String(data.speedPercent, 2) + ",";
    json += "\"setpoint\":" + String(effectiveSetpoint) + ",";
    json += "\"faultCode\":" + String(data.faultCode) + ",";
    json += "\"faultDescription\":\"" + _vfd.getFaultDescription(data.faultCode) + "\",";
    json += "\"lastCommand\":" + String(data.lastCommand) + ",";  // Comando 2000H (1=RUN, 6=STOP)
    json += "\"isRunning\":" + String(data.isRunning ? "true" : "false") + ",";
    json += "\"hasFault\":" + String(data.hasFault ? "true" : "false");
    json += "}";
    
    return json;
}

String WebDashboard::generateHTML() {
    String html = R"rawliteral(
<!DOCTYPE html>
<html lang="es">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Control Variador CW100</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            padding: 20px;
            min-height: 100vh;
        }
        .container {
            max-width: 1200px;
            margin: 0 auto;
            background: white;
            border-radius: 15px;
            padding: 30px;
            box-shadow: 0 20px 60px rgba(0,0,0,0.3);
        }
        h1 {
            text-align: center;
            color: #333;
            margin-bottom: 30px;
            font-size: 2.5em;
        }
        /* Tabs */
        .tabs {
            display: flex;
            gap: 10px;
            margin-bottom: 20px;
            border-bottom: 2px solid #667eea;
        }
        .tab {
            padding: 15px 30px;
            font-size: 1.1em;
            font-weight: bold;
            background: #f0f0f0;
            border: none;
            border-radius: 10px 10px 0 0;
            cursor: pointer;
            transition: all 0.3s ease;
        }
        .tab:hover {
            background: #e0e0e0;
        }
        .tab.active {
            background: #667eea;
            color: white;
        }
        .tab-content {
            display: none;
        }
        .tab-content.active {
            display: block;
        }
        .dashboard {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(300px, 1fr));
            gap: 20px;
            margin-bottom: 30px;
        }
        .card {
            background: #f8f9fa;
            padding: 20px;
            border-radius: 10px;
            box-shadow: 0 2px 10px rgba(0,0,0,0.1);
        }
        .card-title {
            font-size: 1.2em;
            color: #666;
            margin-bottom: 15px;
            font-weight: bold;
        }
        .value-display {
            font-size: 3em;
            font-weight: bold;
            color: #667eea;
            text-align: center;
            margin: 20px 0;
        }
        .unit {
            font-size: 0.5em;
            color: #999;
            margin-left: 5px;
        }
        .speed-control {
            display: flex;
            align-items: center;
            gap: 15px;
            padding: 10px 0;
        }
        .speed-slider {
            flex: 1;
            height: 40px;
            -webkit-appearance: none;
            appearance: none;
            background: linear-gradient(90deg, #e0e0e0 0%, #4CAF50 100%);
            border-radius: 20px;
            outline: none;
        }
        .speed-slider::-webkit-slider-thumb {
            -webkit-appearance: none;
            appearance: none;
            width: 50px;
            height: 50px;
            background: #2196F3;
            border: 4px solid white;
            border-radius: 50%;
            cursor: pointer;
            box-shadow: 0 2px 8px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
        }
        .speed-slider::-webkit-slider-thumb:hover {
            background: #1976D2;
            transform: scale(1.1);
        }
        .speed-slider::-moz-range-thumb {
            width: 50px;
            height: 50px;
            background: #2196F3;
            border: 4px solid white;
            border-radius: 50%;
            cursor: pointer;
            box-shadow: 0 2px 8px rgba(0,0,0,0.3);
            transition: all 0.3s ease;
        }
        .speed-slider::-moz-range-thumb:hover {
            background: #1976D2;
            transform: scale(1.1);
        }
        .speed-value {
            min-width: 60px;
            font-size: 1.5em;
            font-weight: bold;
            color: #2196F3;
            text-align: center;
        }
        .status-box {
            padding: 15px;
            border-radius: 8px;
            text-align: center;
            font-size: 1.5em;
            font-weight: bold;
            margin: 10px 0;
        }
        .status-forward { background: #4CAF50; color: white; }
        .status-reverse { background: #FF9800; color: white; }
        .status-stop { background: #9E9E9E; color: white; }
        .controls {
            display: flex;
            gap: 20px;
            justify-content: center;
            margin: 30px 0;
            position: relative;
            height: 160px;
            align-items: center;
        }
        .btn {
            padding: 20px 40px;
            font-size: 1.5em;
            border: none;
            border-radius: 50%;
            cursor: pointer;
            transition: all 0.3s ease;
            width: 120px;
            height: 120px;
            display: flex;
            flex-direction: column;
            align-items: center;
            justify-content: center;
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
        }
        /* Botones cuadrados autoenclavables (toggles) al borde */
        .btn-toggle {
            width: 120px;
            height: 120px;
            padding: 0;
            border: none;
            border-radius: 8px;
            background: #e0e0e0;
            cursor: pointer;
            display: flex;
            align-items: center;
            justify-content: center;
            font-size: 0.95em;
            color: #333;
            box-shadow: 0 5px 15px rgba(0,0,0,0.2);
            position: absolute;
            top: 50%;
            transform: translateY(-50%);
            z-index: 5;
        }
        .btn-toggle:hover { transform: translateY(-50%) scale(1.03); }
        .btn-toggle.active { background: #4CAF50; color: white; }
        #btnReverse.active { background: #FF9800; color: white; }
        .btn-toggle.left { left: 24px; }
        .btn-toggle.right { right: 24px; }
        .btn:hover {
            transform: scale(1.1);
            box-shadow: 0 8px 25px rgba(0,0,0,0.3);
        }
        .btn-start {
            background: #4CAF50;
            color: white;
        }
        .btn-stop {
            background: #f44336;
            color: white;
        }
        .btn-reset {
            background: #2196F3;
            color: white;
            border-radius: 10px;
            width: auto;
            height: auto;
            padding: 15px 30px;
        }
        .freq-control {
            display: flex;
            gap: 10px;
            align-items: center;
            margin: 20px 0;
        }
        .freq-input {
            flex: 1;
            padding: 15px;
            font-size: 1.2em;
            border: 2px solid #ddd;
            border-radius: 8px;
        }
        .btn-set {
            padding: 15px 30px;
            font-size: 1.2em;
            background: #667eea;
            color: white;
            border: none;
            border-radius: 8px;
            cursor: pointer;
        }
        .btn-set:hover {
            background: #5568d3;
        }
        .fault-box {
            padding: 15px;
            border-radius: 8px;
            text-align: center;
            font-size: 1.8em;
            font-weight: bold;
            background: #f44336;
            color: white;
        }
        .no-fault {
            background: #4CAF50;
        }
        @keyframes pulse {
            0%, 100% { opacity: 1; }
            50% { opacity: 0.5; }
        }
        .running-indicator {
            animation: pulse 1.5s infinite;
        }
    </style>
</head>
<body>
    <div class="container">
        <h1>🎛️ Control Variador CW100</h1>
        
        <!-- Tabs -->
        <div class="tabs">
            <button class="tab active" onclick="switchTab('wifi')">📡 DATOS WIFI</button>
            <button class="tab" onclick="switchTab('variador')">⚙️ CONTROL VARIADOR</button>
        </div>
        
        <!-- Tab Content: WiFi -->
        <div id="tab-wifi" class="tab-content active">
            <div class="dashboard">
                <div class="card">
                    <div class="card-title">SSID</div>
                    <div class="value-display" style="font-size: 2em;" id="wifi-ssid">---</div>
                </div>
                <div class="card">
                    <div class="card-title">IP</div>
                    <div class="value-display" style="font-size: 2em;" id="wifi-ip">---</div>
                </div>
                <div class="card">
                    <div class="card-title">GATEWAY</div>
                    <div class="value-display" style="font-size: 2em;" id="wifi-gateway">---</div>
                </div>
                <div class="card">
                    <div class="card-title">RSSI (Señal)</div>
                    <div class="value-display" id="wifi-rssi">-- <span class="unit">dBm</span></div>
                </div>
                <div class="card">
                    <div class="card-title">Tipo de Conexión</div>
                    <div class="value-display" style="font-size: 2em;" id="wifi-type">---</div>
                </div>
                <div class="card">
                    <div class="card-title">Servidor DNS</div>
                    <div class="value-display" style="font-size: 2em;" id="wifi-dns">---</div>
                </div>
            </div>
            
            <div style="text-align: center; margin-top: 30px;">
                <button class="btn-reset" onclick="disconnectWiFi()" style="font-size: 1.5em; padding: 20px 50px;">
                    📶 DESCONECTAR WIFI
                </button>
                <p style="margin-top: 15px; color: #666;">Para reconectar, reinicia el ESP32</p>
            </div>
        </div>
        
        <!-- Tab Content: Variador -->
        <div id="tab-variador" class="tab-content">
        
        <div class="dashboard">
            <!-- Porcentaje de Velocidad -->
            <div class="card">
                <div class="card-title">Porcentaje de Velocidad Nominal</div>
                <div class="speed-control">
                    <input type="range" id="speedSlider" class="speed-slider" 
                           min="0" max="100" value="0" step="1"
                           oninput="updateSpeedDisplay(this.value)"
                           onchange="setSpeed(this.value)">
                    <span id="speedValue" class="speed-value">0%</span>
                </div>
            </div>
            
            <!-- Frecuencia Actual -->
            <div class="card">
                <div class="card-title">FRECUENCIA ACTUAL</div>
                <div class="value-display" id="frequency">0.00<span class="unit">Hz</span></div>
            </div>
            
            <!-- Corriente Actual -->
            <div class="card">
                <div class="card-title">Corriente Actual</div>
                <div class="value-display" id="current">0.00<span class="unit">A</span></div>
            </div>
        </div>
        
        <!-- Estado del Variador -->
        <div class="card">
            <div class="card-title">STATUS VARIADOR</div>
            <div class="status-box status-stop" id="statusBox">
                <span id="statusText">3 - STOP</span>
            </div>
        </div>
        
        <!-- Botones de Control con toggles cuadrados al borde -->
        <div class="controls" style="align-items: center;">
            <button id="btnFree" class="btn-toggle left" title="Motor libre (free stop)">
                <div style="text-align:center; font-weight:bold;">MOTOR<br>LIBRE</div>
            </button>

            <button class="btn btn-start" onclick="sendCommand('start')">
                ▶<br>RUN
            </button>

            <button class="btn btn-stop" onclick="sendCommand('stop')">
                ⏹<br>STOP
            </button>

            <button id="btnReverse" class="btn-toggle right" title="Cambio de giro (RUN inverso)">
                <div style="text-align:center; font-weight:bold;">CAMBIO<br>GIRO</div>
            </button>
        </div>
        
        <!-- Visualización de Comando de Ejecución 2000H -->
        <div class="card">
            <div class="card-title">Visualización de comando de ejecución 2000H</div>
            <div class="status-box" style="background: #2196F3; color: white;" id="commandBox">
                <span id="commandValue">6</span>
            </div>
            <div style="margin-top: 10px; font-size: 0.9em; color: #666; text-align: center;">
                1 = RUN &nbsp;&nbsp; 2 = RUN (inverso) <br> 5 = FREE STOP &nbsp;&nbsp; 6 = STOP
            </div>
        </div>
        
        <!-- Código de Falla -->
        <div class="card">
            <div class="card-title">FALLA</div>
            <div class="fault-box no-fault" id="faultBox">0</div>
            <button class="btn-reset" onclick="sendCommand('reset')" style="margin-top: 15px; width: 100%;">
                🔄 RESET FALLA
            </button>
        </div>
        
        </div> <!-- Fin tab-variador -->
        
    </div> <!-- Fin container -->

    <script>
        // Función para cambiar entre tabs
        function switchTab(tabName) {
            // Ocultar todos los tabs
            document.querySelectorAll('.tab-content').forEach(tab => {
                tab.classList.remove('active');
            });
            document.querySelectorAll('.tab').forEach(btn => {
                btn.classList.remove('active');
            });
            
            // Mostrar el tab seleccionado
            if (tabName === 'wifi') {
                document.getElementById('tab-wifi').classList.add('active');
                document.querySelectorAll('.tab')[0].classList.add('active');
                updateWiFiInfo();
            } else if (tabName === 'variador') {
                document.getElementById('tab-variador').classList.add('active');
                document.querySelectorAll('.tab')[1].classList.add('active');
                updateStatus();
            }
        }
        
        // Actualizar información WiFi
        function updateWiFiInfo() {
            fetch('/api/wifi')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('wifi-ssid').textContent = data.ssid;
                    document.getElementById('wifi-ip').textContent = data.ip;
                    document.getElementById('wifi-gateway').textContent = data.gateway;
                    document.getElementById('wifi-dns').textContent = data.dns;
                    document.getElementById('wifi-type').textContent = data.connectionType;
                    document.getElementById('wifi-rssi').innerHTML = 
                        data.rssi + ' <span class="unit">dBm</span>';
                })
                .catch(error => console.error('Error:', error));
        }
        
        // Desconectar WiFi
        function disconnectWiFi() {
            if (confirm('¿Desconectar WiFi? Deberás reiniciar el ESP32 para reconectar.')) {
                fetch('/api/wifi/disconnect', { method: 'POST' })
                    .then(response => response.json())
                    .then(data => {
                        alert(data.message);
                        setTimeout(() => updateWiFiInfo(), 1000);
                    })
                    .catch(error => console.error('Error:', error));
            }
        }
        
        // Actualizar estado variador
        function updateStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    // Actualizar frecuencia
                    document.getElementById('frequency').innerHTML = 
                        data.frequency.toFixed(2) + '<span class="unit">Hz</span>';
                    
                    // Actualizar corriente
                    document.getElementById('current').innerHTML = 
                        data.current.toFixed(2) + '<span class="unit">A</span>';
                    
                    // Actualizar slider de velocidad (solo si el usuario no lo está moviendo)
                    const speedSlider = document.getElementById('speedSlider');
                    const speedValue = document.getElementById('speedValue');
                    if (document.activeElement !== speedSlider) {
                        // Preferir mostrar el setpoint real (registro 1000H) cuando esté disponible,
                        // en lugar de inferir porcentaje desde la frecuencia actual que puede ser 0.
                        if (typeof data.setpoint !== 'undefined' && data.setpoint !== null) {
                            // `setpoint` viene como valor del registro (porcentaje * 100), por ejemplo 6200 => 62%
                            const spPercent = Math.round(data.setpoint / 100);
                            speedSlider.value = spPercent;
                            speedValue.textContent = spPercent + '%';
                        } else {
                            speedSlider.value = Math.round(data.speedPercent);
                            speedValue.textContent = Math.round(data.speedPercent) + '%';
                        }
                    }
                    
                    // Actualizar estado
                    const statusBox = document.getElementById('statusBox');
                    const statusText = document.getElementById('statusText');
                    statusBox.className = 'status-box';
                    
                    if (data.status == 1) {
                        statusBox.classList.add('status-forward', 'running-indicator');
                        statusText.textContent = '1 - Adelante';
                    } else if (data.status == 2) {
                        statusBox.classList.add('status-reverse', 'running-indicator');
                        statusText.textContent = '2 - Atrás';
                    } else {
                        statusBox.classList.add('status-stop');
                        statusText.textContent = '3 - Stop';
                    }
                    
                    // Actualizar comando de ejecución 2000H
                    const commandValue = document.getElementById('commandValue');
                    commandValue.textContent = data.lastCommand;
                    
                    // Actualizar falla (código + descripción en español)
                    const faultBox = document.getElementById('faultBox');
                    const desc = data.faultDescription ? data.faultDescription : '';
                    if (data.faultCode == 0) {
                        faultBox.textContent = data.faultCode + ' - ' + desc;
                        faultBox.className = 'fault-box no-fault';
                    } else {
                        faultBox.textContent = data.faultCode + ' - ' + desc;
                        faultBox.className = 'fault-box';
                    }
                })
                .catch(error => console.error('Error:', error));
        }
        
        function sendCommand(cmd) {
            // Construir el cuerpo con flags según toggles
            let body = 'cmd=' + encodeURIComponent(cmd);
            if (cmd === 'start') {
                const rev = (document.getElementById('btnReverse') && document.getElementById('btnReverse').classList.contains('active')) ? '1' : '0';
                body += '&reverse=' + rev;
            }
            if (cmd === 'stop') {
                const free = (document.getElementById('btnFree') && document.getElementById('btnFree').classList.contains('active')) ? '1' : '0';
                body += '&free=' + free;
            }

            fetch('/api/command', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: body
            })
            .then(response => response.json())
            .then(data => {
                console.log(data.message);
                setTimeout(updateStatus, 500);
            })
            .catch(error => console.error('Error:', error));
        }
        
        // Inicializar botones toggle (autoenclavables)
        const btnFree = document.getElementById('btnFree');
        const btnReverse = document.getElementById('btnReverse');
        if (btnFree) {
            btnFree.addEventListener('click', function(e){ e.preventDefault(); this.classList.toggle('active'); });
        }
        if (btnReverse) {
            btnReverse.addEventListener('click', function(e){ e.preventDefault(); this.classList.toggle('active'); });
        }

        function setFrequency() {
            const freq = document.getElementById('freqInput').value;
            fetch('/api/frequency', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'freq=' + freq
            })
            .then(response => response.json())
            .then(data => {
                alert(data.message);
                setTimeout(updateStatus, 500);
            })
            .catch(error => console.error('Error:', error));
        }
        
        // Actualizar display del slider en tiempo real
        function updateSpeedDisplay(value) {
            document.getElementById('speedValue').textContent = value + '%';
        }
        
        // Enviar nueva velocidad cuando el usuario suelta el slider
        function setSpeed(percent) {
            fetch('/api/speed', {
                method: 'POST',
                headers: {'Content-Type': 'application/x-www-form-urlencoded'},
                body: 'percent=' + percent
            })
            .then(response => response.json())
            .then(data => {
                console.log('Velocidad establecida: ' + percent + '% (' + data.frequency + ' Hz)');
                setTimeout(updateStatus, 500);
            })
            .catch(error => console.error('Error:', error));
        }
        
        // Actualizar cada 1 segundo según el tab activo
        setInterval(() => {
            if (document.getElementById('tab-wifi').classList.contains('active')) {
                updateWiFiInfo();
            } else {
                updateStatus();
            }
        }, 1000);
        
        // Inicializar mostrando WiFi info
        updateWiFiInfo();
    </script>
</body>
</html>
)rawliteral";
    
    return html;
}
