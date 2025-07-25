#define WIN32_LEAN_AND_MEAN   // avoid including old winsock.h
#include <winsock2.h>         // must come before windows.h
#include <ws2tcpip.h>
#include <Windows.h>          // include after winsock2.h
#include <thread>
#include <map>
#include <string>
#include "httplib.h"


// Track the current state of each cheat
std::map<std::string, bool> cheatStates = {
    {"fly", false},
    {"xray", false},
    {"killaura", false},
    {"speed", false},
    {"nofall", false}
};

// Embedded HTML (you can edit this as needed)
const char* html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8" />
<meta name="viewport" content="width=device-width, initial-scale=1" />
<title>McClient Control Panel - Vape Style</title>
<style>
  /* Reset */
  * {
    box-sizing: border-box;
  }
  body {
    margin: 0;
    font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif;
    background-color: #121017;
    color: #f2b5d4;
    display: flex;
    height: 100vh;
    overflow: hidden;
  }
  /* Sidebar */
  .sidebar {
    background: #1f1c2f;
    width: 220px;
    padding-top: 2rem;
    display: flex;
    flex-direction: column;
  }
  .sidebar h2 {
    color: #ff8ecf;
    font-weight: 700;
    text-align: center;
    margin-bottom: 1.5rem;
  }
  .sidebar button {
    background: none;
    border: none;
    color: #f2b5d4;
    padding: 1rem 1.5rem;
    text-align: left;
    font-size: 1.1rem;
    cursor: pointer;
    transition: background 0.3s;
    border-left: 4px solid transparent;
  }
  .sidebar button.active,
  .sidebar button:hover {
    background: #2f2a4c;
    border-left: 4px solid #ff8ecf;
    color: #ff8ecf;
  }

  /* Main content */
  .main {
    flex-grow: 1;
    padding: 2rem;
    background: #1e1e2f;
    overflow-y: auto;
  }
  .main h1 {
    margin-top: 0;
    margin-bottom: 1.5rem;
    color: #ff8ecf;
  }
  .category {
    display: none;
  }
  .category.active {
    display: block;
  }
  .toggle-list {
    display: flex;
    flex-wrap: wrap;
    gap: 1.5rem;
  }
  .toggle {
    background: #2a2a3d;
    border-radius: 12px;
    padding: 1rem 1.2rem;
    min-width: 130px;
    display: flex;
    justify-content: space-between;
    align-items: center;
    box-shadow: 0 0 10px rgba(255, 142, 207, 0.3);
    cursor: pointer;
    user-select: none;
  }
  .toggle span {
    font-size: 1.1rem;
  }
  /* Custom checkbox toggle */
  input[type="checkbox"] {
    appearance: none;
    width: 36px;
    height: 20px;
    background: #44415f;
    border-radius: 20px;
    position: relative;
    cursor: pointer;
    transition: background-color 0.3s;
  }
  input[type="checkbox"]::before {
    content: "";
    position: absolute;
    top: 2px;
    left: 2px;
    width: 16px;
    height: 16px;
    background: #ff8ecf;
    border-radius: 50%;
    transition: transform 0.3s;
  }
  input[type="checkbox"]:checked {
    background: #ff8ecf;
  }
  input[type="checkbox"]:checked::before {
    transform: translateX(16px);
    background: #fff0fb;
  }
</style>
</head>
<body>
  <nav class="sidebar">
    <h2>McClient</h2>
    <button class="active" data-target="combat">Combat</button>
    <button data-target="movement">Movement</button>
    <button data-target="render">Render</button>
    <button data-target="player">Player</button>
    <button data-target="misc">Misc</button>
  </nav>

  <main class="main">
    <h1>Control Panel</h1>

    <!-- Combat -->
    <section id="combat" class="category active">
      <div class="toggle-list">
        <label class="toggle" for="killaura"><span>KillAura</span><input type="checkbox" id="killaura" onchange="updateFlag('killaura')" /></label>
        <label class="toggle" for="criticals"><span>Criticals</span><input type="checkbox" id="criticals" onchange="updateFlag('criticals')" /></label>
        <label class="toggle" for="autoBlock"><span>AutoBlock</span><input type="checkbox" id="autoBlock" onchange="updateFlag('autoBlock')" /></label>
        <label class="toggle" for="velocity"><span>Velocity</span><input type="checkbox" id="velocity" onchange="updateFlag('velocity')" /></label>
        <label class="toggle" for="triggerbot"><span>TriggerBot</span><input type="checkbox" id="triggerbot" onchange="updateFlag('triggerbot')" /></label>
      </div>
    </section>

    <!-- Movement -->
    <section id="movement" class="category">
      <div class="toggle-list">
        <label class="toggle" for="fly"><span>Fly</span><input type="checkbox" id="fly" onchange="updateFlag('fly')" /></label>
        <label class="toggle" for="speed"><span>Speed</span><input type="checkbox" id="speed" onchange="updateFlag('speed')" /></label>
        <label class="toggle" for="noFall"><span>NoFall</span><input type="checkbox" id="noFall" onchange="updateFlag('noFall')" /></label>
        <label class="toggle" for="step"><span>Step</span><input type="checkbox" id="step" onchange="updateFlag('step')" /></label>
        <label class="toggle" for="longJump"><span>LongJump</span><input type="checkbox" id="longJump" onchange="updateFlag('longJump')" /></label>
      </div>
    </section>

    <!-- Render -->
    <section id="render" class="category">
      <div class="toggle-list">
        <label class="toggle" for="xray"><span>X-Ray</span><input type="checkbox" id="xray" onchange="updateFlag('xray')" /></label>
        <label class="toggle" for="chestESP"><span>ChestESP</span><input type="checkbox" id="chestESP" onchange="updateFlag('chestESP')" /></label>
        <label class="toggle" for="tracers"><span>Tracers</span><input type="checkbox" id="tracers" onchange="updateFlag('tracers')" /></label>
        <label class="toggle" for="fullbright"><span>Fullbright</span><input type="checkbox" id="fullbright" onchange="updateFlag('fullbright')" /></label>
        <label class="toggle" for="nametags"><span>Nametags</span><input type="checkbox" id="nametags" onchange="updateFlag('nametags')" /></label>
      </div>
    </section>

    <!-- Player -->
    <section id="player" class="category">
      <div class="toggle-list">
        <label class="toggle" for="autoEat"><span>AutoEat</span><input type="checkbox" id="autoEat" onchange="updateFlag('autoEat')" /></label>
        <label class="toggle" for="noPush"><span>NoPush</span><input type="checkbox" id="noPush" onchange="updateFlag('noPush')" /></label>
        <label class="toggle" for="antiKB"><span>AntiKB</span><input type="checkbox" id="antiKB" onchange="updateFlag('antiKB')" /></label>
        <label class="toggle" for="autoRespawn"><span>AutoRespawn</span><input type="checkbox" id="autoRespawn" onchange="updateFlag('autoRespawn')" /></label>
        <label class="toggle" for="scaffold"><span>Scaffold</span><input type="checkbox" id="scaffold" onchange="updateFlag('scaffold')" /></label>
      </div>
    </section>

    <!-- Misc -->
    <section id="misc" class="category">
      <div class="toggle-list">
        <label class="toggle" for="autoFish"><span>AutoFish</span><input type="checkbox" id="autoFish" onchange="updateFlag('autoFish')" /></label>
        <label class="toggle" for="autoTool"><span>AutoTool</span><input type="checkbox" id="autoTool" onchange="updateFlag('autoTool')" /></label>
        <label class="toggle" for="fastPlace"><span>FastPlace</span><input type="checkbox" id="fastPlace" onchange="updateFlag('fastPlace')" /></label>
        <label class="toggle" for="blink"><span>Blink</span><input type="checkbox" id="blink" onchange="updateFlag('blink')" /></label>
        <label class="toggle" for="timer"><span>Timer</span><input type="checkbox" id="timer" onchange="updateFlag('timer')" /></label>
      </div>
    </section>

  </main>

<script>
  // Sidebar nav logic
  const buttons = document.querySelectorAll('.sidebar button');
  const categories = document.querySelectorAll('.category');

  buttons.forEach(button => {
    button.addEventListener('click', () => {
      buttons.forEach(btn => btn.classList.remove('active'));
      button.classList.add('active');
      const target = button.getAttribute('data-target');
      categories.forEach(cat => {
        cat.classList.toggle('active', cat.id === target);
      });
    });
  });

  async function updateFlag(flag) {
    const value = document.getElementById(flag).checked;
    try {
      await fetch('/api/toggle', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ [flag]: value })
      });
    } catch (e) {
      console.error('Failed to update flag', flag, e);
    }
  }

  async function syncToggles() {
    try {
      const res = await fetch('/api/state');
      const state = await res.json();
      for (const key in state) {
        const box = document.getElementById(key);
        if (box) box.checked = state[key];
      }
    } catch (e) {
      console.error('Failed to sync toggles', e);
    }
  }

  window.onload = syncToggles;
</script>
</body>
</html>
)rawliteral";


void StartServer() {
    httplib::Server svr;

    svr.Get("/", [](const httplib::Request&, httplib::Response& res) {
        res.set_content(html, "text/html");
        });

    svr.Get("/api/state", [](const httplib::Request&, httplib::Response& res) {
        std::string json = "{";
        bool first = true;
        for (const auto& [key, val] : cheatStates) {
            if (!first) json += ",";
            json += "\"" + key + "\":" + (val ? "true" : "false");
            first = false;
        }
        json += "}";
        res.set_content(json, "application/json");
        });

    svr.Post("/api/toggle", [](const httplib::Request& req, httplib::Response& res) {
        for (auto& [key, val] : cheatStates) {
            if (req.body.find("\"" + key + "\":true") != std::string::npos)
                val = true;
            else if (req.body.find("\"" + key + "\":false") != std::string::npos)
                val = false;
        }
        res.set_content("OK", "text/plain");
        });

    svr.listen("127.0.0.1", 1337);
}

DWORD WINAPI MainThread(LPVOID) {
    std::thread serverThread(StartServer);
    serverThread.detach();

    while (true) {
        // TODO: use `cheatStates["fly"]`, etc. to toggle cheat logic
        Sleep(1000);
    }
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID) {
    if (ul_reason_for_call == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(hModule);
        CreateThread(nullptr, 0, MainThread, nullptr, 0, nullptr);
    }
    return TRUE;
}
