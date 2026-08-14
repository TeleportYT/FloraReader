#include "WebServerManager.h"
#include <SD.h>

// Embedded HTML page stored in PROGMEM for instant delivery
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>🌸 FloraReader - iPhone WiFi Book Loader</title>
    <style>
        :root { --bg: #fffaf5; --card: #ffffff; --primary: #e88ca5; --text: #4a3e3d; }
        * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, sans-serif; }
        body { background: var(--bg); color: var(--text); display: flex; justify-content: center; padding: 16px; }
        .box { width: 100%; max-width: 440px; background: var(--card); border-radius: 20px; border: 2px solid #ffd8e4; padding: 20px; display: flex; flex-direction: column; gap: 16px; }
        h1 { color: var(--primary); text-align: center; font-size: 1.6rem; }
        .drop { border: 2px dashed var(--primary); border-radius: 14px; padding: 24px; text-align: center; background: #fff5f8; cursor: pointer; }
        .btn { background: var(--primary); color: white; border: none; padding: 10px 16px; border-radius: 10px; font-weight: 600; cursor: pointer; }
        ul { list-style: none; display: flex; flex-direction: column; gap: 8px; }
        li { display: flex; justify-content: space-between; align-items: center; padding: 10px; background: #fdf6f8; border-radius: 8px; font-size: 0.9rem; }
        .del { background: #ff6b6b; color: white; border: none; border-radius: 6px; padding: 4px 8px; font-size: 0.8rem; cursor: pointer; }
    </style>
</head>
<body>
    <div class="box">
        <h1>🌸 FloraReader</h1>
        <p style="text-align:center; font-size: 0.85rem; color:#888;">Upload books wirelessly from iPhone</p>
        <div class="drop" onclick="document.getElementById('f').click()">
            <div style="font-size:2rem;">📚🖼️</div>
            <div style="font-weight:600; margin-top:4px;">Tap to Select Book or Image (.txt / .md / .bmp)</div>
            <input type="file" id="f" style="display:none" accept=".txt,.md,.bmp" onchange="up(this.files[0])">
        </div>
        <div id="st" style="text-align:center; font-weight:500; color:var(--primary);"></div>
        <h3>🌸 SD Card Library & Screensavers</h3>
        <ul id="list"><li>Loading...</li></ul>
    </div>
    <script>
        function load() {
            fetch('/api/books').then(r=>r.json()).then(b=>{
                let l = document.getElementById('list'); l.innerHTML = '';
                if(!b.length) { l.innerHTML = '<li style="justify-content:center">No files yet 🌸</li>'; return; }
                b.forEach(x => {
                    let icon = (x.endsWith('.bmp') || x.endsWith('.BMP')) ? '🖼️' : '📖';
                    l.innerHTML += `<li><span>${icon} ${x}</span><button class="del" onclick="del('${x}')">Delete</button></li>`;
                });
            });
        }
        function up(file) {
            if(!file) return;
            let st = document.getElementById('st'); st.textContent = `Uploading ${file.name}...`;
            let fd = new FormData(); fd.append('file', file, file.name);
            fetch('/upload', { method: 'POST', body: fd }).then(r => {
                st.textContent = '✨ Upload Complete!';
                load();
            }).catch(() => st.textContent = '❌ Upload Failed');
        }
        function del(name) {
            if(confirm('Delete ' + name + '?')) {
                fetch('/api/delete?name=' + encodeURIComponent(name), { method: 'DELETE' }).then(()=>load());
            }
        }
        load();
    </script>
</body>
</html>
)rawliteral";

WebServerManager::WebServerManager(LibraryManager& libManager)
    : m_libManager(libManager), m_server(WEB_PORT), m_running(false), m_routesSetup(false) {
}

void WebServerManager::begin() {
    if (m_running) return;

    Serial.println("[WiFi] Starting Access Point mode...");
    WiFi.mode(WIFI_AP);
    delay(50);
    WiFi.softAPConfig(IPAddress(192, 168, 4, 1), IPAddress(192, 168, 4, 1), IPAddress(255, 255, 255, 0));
    WiFi.softAP(WIFI_AP_SSID, WIFI_AP_PASS);
    WiFi.setSleep(false);  // Disable WiFi modem sleep to keep AP alive
    
    IPAddress IP = WiFi.softAPIP();
    m_ipAddress = IP.toString();
    Serial.printf("[WiFi] AP IP address: %s\n", m_ipAddress.c_str());

    m_dnsServer.start(53, "*", IP);

    if (!m_routesSetup) {
        setupRoutes();
        m_routesSetup = true;
    }
    m_server.begin();
    m_running = true;
}

void WebServerManager::stop() {
    if (m_running) {
        m_running = false;
        m_dnsServer.stop();
        m_server.end();
        WiFi.mode(WIFI_OFF);
        Serial.println("[WiFi] Web Server and AP stopped safely.");
    }
}

void WebServerManager::loop() {
    if (m_running) {
        m_dnsServer.processNextRequest();
    }
}

void WebServerManager::setupRoutes() {
    m_server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });

    m_server.on("/hotspot-detect.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });
    m_server.on("/generate_204", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });
    m_server.on("/canonical.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });
    m_server.on("/nologin.html", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });
    m_server.on("/connecttest.txt", HTTP_GET, [](AsyncWebServerRequest *request) {
        request->send_P(200, "text/html", INDEX_HTML);
    });

    m_server.onNotFound([](AsyncWebServerRequest *request) {
        request->redirect("http://192.168.4.1/");
    });

    m_server.on("/api/books", HTTP_GET, [this](AsyncWebServerRequest *request) {
        JsonDocument doc;
        JsonArray array = doc.to<JsonArray>();
        
        const auto& books = m_libManager.getBookList();
        for (const auto& book : books) {
            array.add(book);
        }

        String jsonResponse;
        serializeJson(doc, jsonResponse);
        request->send(200, "application/json", jsonResponse);
    });

    m_server.on("/api/delete", HTTP_DELETE, [this](AsyncWebServerRequest *request) {
        if (request->hasParam("name")) {
            String filename = request->getParam("name")->value();
            bool deleted = m_libManager.deleteBookFile(filename.c_str());
            if (deleted) {
                request->send(200, "text/plain", "Deleted");
                return;
            }
        }
        request->send(400, "text/plain", "Error deleting file");
    });

    m_server.on("/upload", HTTP_POST, [](AsyncWebServerRequest *request) {
        request->send(200, "text/plain", "OK");
    }, [this](AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
        static File uploadFile;
        
        if (index == 0) {
            Serial.printf("[Upload] Starting upload: %s\n", filename.c_str());
            String fullPath = String(BOOKS_DIR) + "/" + filename;
            uploadFile = SD.open(fullPath, FILE_WRITE);
        }
        
        if (uploadFile) {
            uploadFile.write(data, len);
        }
        
        if (final) {
            if (uploadFile) uploadFile.close();
            Serial.printf("[Upload] Finished upload: %s (%d total bytes)\n", filename.c_str(), index + len);
            m_libManager.scanBooksDirectory();
        }
    });
}

