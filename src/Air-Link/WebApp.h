#ifndef WEBAPP_H
#define WEBAPP_H

#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "config.h"
#include "HardwareInfo.h"
#include "diagnostics.h"
#include "web_serial.h"

extern ESP8266WebServer server;

// Pagina HTML con JavaScript integrato e chiamate asincrone AJAX
const char PAGE_INDEX[] PROGMEM = R"=====(
<!DOCTYPE html>
<html lang="it">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <meta name="theme-color" content="#1976d2">
    <title>Air-Link Weather Station</title>
    <link rel="stylesheet" href="https://cdnjs.cloudflare.com/ajax/libs/font-awesome/6.5.0/css/all.min.css">
    <link href="https://fonts.googleapis.com/css2?family=Material+Symbols+Outlined" rel="stylesheet">
    <link href="https://fonts.googleapis.com/css2?family=Inter:wght@300;400;500;600;700;800&family=JetBrains+Mono:wght@400;500;600;700&display=swap" rel="stylesheet">
   <!-- Collegamento all'icona classica e iOS Apple -->
  <link rel="icon" type="image/png" sizes="192x192" href="/icon-192.png">
  <link rel="apple-touch-icon" href="/icon-192.png">
  
  <!-- Manifest per l'installazione PWA -->
  <link rel="manifest" href="/manifest.json">
  
  <!-- Colore barra di stato dello smartphone -->
  <meta name="theme-color" content="#1976d2">

  <!-- Registrazione Service Worker per installazione PWA -->
  <script>
    if ('serviceWorker' in navigator) {
      window.addEventListener('load', function() {
        navigator.serviceWorker.register('/sw.js').then(function(reg) {
          console.log('Service Worker registrato con successo:', reg.scope);
        }).catch(function(err) {
          console.log('Errore registrazione Service Worker:', err);
        });
      });
    }
  </script>
    
    <style>
        :root {
            --bg-app: #edf2f7; --bg-card: #ffffff; --bg-box: #f8fafc; --bg-box-hover: #f1f5f9;
            --bg-sidebar: #1f2937; --text-primary: #1e293b; --text-secondary: #475569;
            --text-muted: #94a3b8; --text-sidebar: #ffffff; --text-sidebar-muted: #94a3b8;
            --text-sidebar-hover: rgba(255, 255, 255, .1); --border-color: #e2e8f0;
            --border-dashed: #cbd5e1; --appbar-bg: #ffffff; --shadow: 0 4px 6px -1px rgba(0, 0, 0, .05);
            --shadow-card: 0 10px 25px -5px rgba(0, 0, 0, .05); --accent: #1976d2;
            --accent-light: rgba(25, 118, 210, 0.1); --box-border: 2px dashed var(--border-dashed);
            --status-online: #10b981; --status-offline: #ef4444; --bg-terminal: #0f172a; --text-terminal: #38bdf8;
        }
        body.dark-theme {
            --bg-app: #0f172a; --bg-card: #1e293b; --bg-box: #0f172a; --bg-box-hover: #1e293b;
            --bg-sidebar: #020617; --text-primary: #f8fafc; --text-secondary: #94a3b8;
            --text-muted: #64748b; --text-sidebar: #f1f5f9; --text-sidebar-muted: #64748b;
            --text-sidebar-hover: rgba(255, 255, 255, .08); --border-color: #334155;
            --border-dashed: #475569; --appbar-bg: #1e293b; --bg-terminal: #020617; --text-terminal: #4ade80;
        }
        * { margin: 0; padding: 0; box-sizing: border-box; font-family: 'Inter', sans-serif; transition: background-color 0.25s, border-color 0.25s, color 0.25s; }
        body { background: var(--bg-app); height: 100vh; overflow: hidden; color: var(--text-primary); }
        .container { display: flex; height: 100vh; position: relative; }
        .sidebar { width: 260px; background: var(--bg-sidebar); color: var(--text-sidebar); padding: 25px 20px; display: flex; flex-direction: column; justify-content: space-between; height: 100%; z-index: 100; transition: transform .3s ease; }
        .sidebar h2 { margin-bottom: 30px; font-size: 24px; font-weight: 800; display: flex; align-items: center; gap: 12px; border-bottom: 1px solid rgba(255, 255, 255, 0.1); padding-bottom: 20px; }
        .sidebar h2 i { color: var(--accent); }
        .sidebar ul { list-style: none; flex-grow: 1; }
        .sidebar li { padding: 14px 18px; border-radius: 12px; cursor: pointer; margin-bottom: 8px; display: flex; align-items: center; gap: 14px; font-weight: 500; font-size: 14px; color: rgba(255, 255, 255, 0.85); }
        .sidebar li:hover { background: var(--text-sidebar-hover); color: #fff; }
        .sidebar li.active { background: var(--accent); color: #fff; font-weight: 600; }
        .sidebar-footer { background: rgba(255, 255, 255, 0.04); border: 1px solid rgba(255, 255, 255, 0.08); border-radius: 16px; padding: 14px 16px; margin-top: auto; }
        .sidebar-footer-title { font-size: 11px; text-transform: uppercase; font-weight: 700; color: var(--text-sidebar-muted); margin-bottom: 8px; font-family: 'JetBrains Mono', monospace; display: flex; align-items: center; gap: 6px; }
        .sidebar-footer-info { font-size: 12px; display: flex; flex-direction: column; gap: 6px; color: rgba(255, 255, 255, 0.9); }
        .sidebar-footer-info div { display: flex; justify-content: space-between; }
        .sidebar-footer-info span { font-family: 'JetBrains Mono', monospace; font-size: 11px; }
        .content { flex: 1; display: flex; flex-direction: column; height: 100%; overflow: hidden; }
        .appbar { height: 70px; background: var(--appbar-bg); display: flex; align-items: center; justify-content: space-between; padding: 0 25px; box-shadow: var(--shadow); border-bottom: 1px solid var(--border-color); z-index: 10; }
        .left-title { display: flex; align-items: center; gap: 12px; font-size: 24px; font-weight: 800; }
        .left-title i { color: var(--accent); }
        .menu-btn { display: none; font-size: 28px; cursor: pointer; margin-right: 15px; color: var(--text-primary); }
        .status-indicator { display: inline-flex; align-items: center; gap: 6px; background: rgba(16, 185, 129, 0.1); padding: 5px 12px; border-radius: 20px; margin-left: 15px; border: 1px solid rgba(16, 185, 129, 0.2); font-size: 11px; font-weight: 700; color: var(--status-online); }
        .status-indicator.offline { background: rgba(239, 68, 68, 0.1); border-color: rgba(239, 68, 68, 0.2); color: var(--status-offline); }
        .status-dot { width: 8px; height: 8px; border-radius: 50%; background-color: var(--status-online); }
        .status-indicator.offline .status-dot { background-color: var(--status-offline); }
        .datetime { display: flex; align-items: center; gap: 8px; background: var(--bg-box); color: var(--text-secondary); border: 1px solid var(--border-color); padding: 8px 16px; border-radius: 30px; font-size: 13px; font-weight: 600; font-family: 'JetBrains Mono', monospace; }
        .page { padding: 25px; flex: 1; overflow: auto; display: flex; flex-direction: column; gap: 20px; }
        .dashboard { background: var(--bg-card); border-radius: 25px; padding: 30px; flex: 1; box-shadow: var(--shadow-card); border: 1px solid var(--border-color); display: grid; gap: 20px; grid-template-columns: repeat(2, 1fr); grid-template-rows: 220px 220px 180px; }
        .box { background: var(--bg-box); border-radius: 20px; border: var(--box-border); padding: 22px; display: flex; flex-direction: column; justify-content: space-between; transition: all 0.25s ease; }
        .box:hover { transform: translateY(-2px); background: var(--bg-box-hover); border-color: var(--accent); }
        .box-header { display: flex; justify-content: space-between; align-items: center; }
        .box-title { font-size: 12px; text-transform: uppercase; font-weight: 700; color: var(--text-secondary); font-family: 'JetBrains Mono', monospace; }
        .box-icon { font-size: 22px; color: var(--accent); background: var(--accent-light); padding: 8px; border-radius: 12px; width: 42px; height: 42px; display: flex; align-items: center; justify-content: center; }
        .box-value-container { margin: 15px 0; }
        .box-value { font-size: 36px; font-weight: 800; color: var(--text-primary); font-family: 'JetBrains Mono', monospace; }
        .box-footer { border-top: 1px solid var(--border-color); padding-top: 12px; display: flex; justify-content: space-between; font-size: 12px; font-family: 'JetBrains Mono', monospace; color: var(--text-secondary); }
        .box-footer-min { color: #3b82f6; font-weight: 500; }
        .box-footer-max { color: #ef4444; font-weight: 500; }
        .sensor-badge { display: inline-block; padding: 4px 10px; border-radius: 20px; font-size: 11px; font-weight: 700; text-transform: uppercase; font-family: 'JetBrains Mono', monospace; }
        .badge-normal { background: rgba(16, 185, 129, 0.12); color: #10b981; }
        .badge-warning { background: rgba(245, 158, 11, 0.12); color: #f59e0b; }
        .badge-danger { background: rgba(239, 68, 68, 0.12); color: #ef4444; }
        .badge-info { background: rgba(59, 130, 246, 0.12); color: #3b82f6; }
        .sidebar-overlay { display: none; position: fixed; inset: 0; background: rgba(0, 0, 0, 0.4); backdrop-filter: blur(4px); z-index: 99; opacity: 0; transition: opacity 0.3s ease; pointer-events: none; }
        .sidebar-overlay.active { opacity: 1; pointer-events: auto; }
        .theme-selector { display: flex; align-items: center; gap: 2px; background: var(--bg-box); border: 1px solid var(--border-color); padding: 4px; border-radius: 12px; }
        .theme-btn { background: transparent; border: none; color: var(--text-secondary); cursor: pointer; padding: 6px 10px; border-radius: 8px; font-size: 13px; display: flex; align-items: center; justify-content: center; font-weight: 600; }
        .theme-btn.active { background: var(--bg-card); color: var(--accent); box-shadow: var(--shadow); }
        .cruscotto-top-bar { display: flex; justify-content: space-between; align-items: center; margin-bottom: 15px; padding: 0 5px; }
        .cruscotto-title-section h3 { font-size: 18px; font-weight: 700; }
        .cruscotto-buttons { display: flex; gap: 10px; }
        .btn-action { background: var(--bg-box); border: 1px solid var(--border-color); color: var(--text-primary); padding: 8px 14px; border-radius: 12px; font-size: 12px; font-weight: 600; cursor: pointer; display: flex; align-items: center; gap: 6px; }
        .btn-action:hover { background: var(--bg-box-hover); border-color: var(--accent); }
        .page-content-wrapper { display: none; flex-direction: column; flex: 1; height: 100%; }
        .page-content-wrapper.active-view { display: flex; }
        .logs-layout { display: grid; grid-template-columns: 1.4fr 0.6fr; gap: 20px; flex: 1; min-height: 0; }
        .log-card { background: var(--bg-card); border-radius: 25px; padding: 25px; box-shadow: var(--shadow-card); border: 1px solid var(--border-color); display: flex; flex-direction: column; gap: 15px; height: 100%; }
        .log-card h4 { font-size: 15px; font-weight: 700; display: flex; align-items: center; gap: 8px; color: var(--text-primary); border-bottom: 1px solid var(--border-color); padding-bottom: 12px; }
        .terminal-display { background: var(--bg-terminal); color: var(--text-terminal); flex: 1; border-radius: 16px; padding: 16px; font-family: 'JetBrains Mono', monospace; font-size: 13px; line-height: 1.5; overflow-y: auto; border: 1px solid var(--border-color); }
        .events-list { flex: 1; overflow-y: auto; display: flex; flex-direction: column; gap: 10px; }
        .event-item { display: flex; flex-direction: column; gap: 4px; padding: 12px; background: var(--bg-box); border-left: 4px solid var(--accent); border-radius: 0 12px 12px 0; font-size: 13px; }
        .event-item.info { border-left-color: #3b82f6; }
        .event-item.success { border-left-color: #10b981; }
        .event-item.warning { border-left-color: #f59e0b; }
        .event-time { font-family: 'JetBrains Mono', monospace; font-size: 11px; color: var(--text-muted); }
        .event-msg { font-weight: 500; color: var(--text-secondary); }
        .hardware-container { width: 100%; max-width: 1500px; }
        .hardware-grid { display: grid; grid-template-columns: 2fr 1fr; gap: 25px; }
        .hardware-card { padding: 25px; display: flex; flex-direction: column; }
        .card-title { display: flex; align-items: center; gap: 12px; padding-bottom: 15px; margin-bottom: 20px; border-bottom: 1px solid var(--border-color); font-size: 20px; font-weight: 700; }
        .hardware-info { display: grid; grid-template-columns: repeat(auto-fit, minmax(220px, 1fr)); gap: 18px; }
        .hw-item { display: flex; align-items: center; gap: 15px; padding: 18px; background: var(--bg-box); border-radius: 16px; border: 1px solid var(--border-color); transition: .25s; }
        .hw-item:hover { transform: translateY(-3px); box-shadow: 0 10px 20px rgba(0, 0, 0, .08); }
        .hw-item i { width: 40px; font-size: 28px; color: var(--accent); text-align: center; }
        .hw-item small { display: block; color: var(--text-muted); margin-bottom: 4px; font-size: 12px; }
        .hw-item strong { font-size: 15px; color: var(--text-primary); font-family: 'JetBrains Mono', monospace; }
        .sensor-list { display: flex; flex-direction: column; gap: 14px; }
        .sensor { display: flex; justify-content: space-between; align-items: center; padding: 15px 18px; background: var(--bg-box); border: 1px solid var(--border-color); border-radius: 14px; }
        .diag-button { height: 50px; border: none; border-radius: 14px; background: var(--accent); color: #fff; font-size: 15px; font-weight: 600; cursor: pointer; transition: .25s; display: flex; align-items: center; justify-content: center; gap: 10px; margin-top: 15px; }
        .diag-button:hover { transform: translateY(-2px); opacity: .92; }
        @media(max-width:1100px) { .hardware-grid { grid-template-columns: 1fr; } }
        @media(max-width:900px) {
            .sidebar { position: fixed; left: -260px; top: 0; height: 100%; z-index: 100; }
            .sidebar.active { left: 0; }
            .sidebar-overlay { display: block; }
            .menu-btn { display: block; }
            .dashboard { grid-template-columns: 1fr; grid-template-rows: auto; padding: 20px; }
            .box { height: auto; min-height: 185px; }
            .appbar { padding: 0 15px; }
            .status-text { display: none; }
            .datetime { padding: 6px 10px; font-size: 11px; }
            .logs-layout { grid-template-columns: 1fr; grid-template-rows: 400px 300px; }
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="sidebar-overlay" id="sidebarOverlay" onclick="toggleMenu()"></div>
        <div class="sidebar" id="sidebar">
            <div>
                <h2><i class="fa-solid fa-cloud-sun"></i> Air-Link</h2>
                <ul>
                    <li class="active" data-page="dashboard" onclick="switchNav(this)">
                        <span class="material-symbols-outlined">space_dashboard</span> Dashboard
                    </li>
                    <li data-page="logs" onclick="switchNav(this)">
                        <span class="material-symbols-outlined">code</span> Log e Eventi
                    </li>
                    <li data-page="database" onclick="switchNav(this)">
                        <span class="material-symbols-outlined">database</span> Database
                    </li>
                    <li data-page="hardware" onclick="switchNav(this)">
                        <span class="material-symbols-outlined">memory</span> Hardware Monitor
                    </li>
                </ul>
            </div>
            <div class="sidebar-footer">
                <div class="sidebar-footer-title">
                    <span class="material-symbols-outlined" style="font-size:12px;">info</span> Dispositivo
                </div>
                <div class="sidebar-footer-info">
                    <div><span>Firmware:</span><span>v2.0.0</span></div>
                    <div><span>IP Locale:</span><span id="footer-ip">---</span></div>
                    <div><span>Segnale:</span><span id="footer-rssi" style="color:#10b981; font-weight: bold;">---</span></div>
                </div>
            </div>
        </div>

        <div class="content">
            <div class="appbar">
                <div style="display:flex; align-items:center;">
                    <span class="material-symbols-outlined menu-btn" onclick="toggleMenu()">menu</span>
                    <div class="left-title"><i class="fa-solid fa-cloud-sun"></i> Air-Link</div>
                    <div class="status-indicator" id="statusIndicator">
                        <span class="status-dot"></span>
                        <span class="status-text" id="statusText">ONLINE</span>
                    </div>
                </div>
                <div style="display:flex; align-items:center; gap:12px;">
                    <div class="theme-selector">
                        <button class="theme-btn" id="theme-btn-light" onclick="setTheme('light')"><span class="material-symbols-outlined" style="font-size:18px;">light_mode</span></button>
                        <button class="theme-btn" id="theme-btn-dark" onclick="setTheme('dark')"><span class="material-symbols-outlined" style="font-size:18px;">dark_mode</span></button>
                    </div>
                    <div class="datetime">
                        <span class="material-symbols-outlined" style="font-size:16px;">calendar_today</span>
                        <span id="datetime-display">--/--/---- --:--:--</span>
                    </div>
                </div>
            </div>

            <div class="page">
                <div style="flex: 1; display: flex; flex-direction: column;">
                    <div class="cruscotto-top-bar">
                        <div class="cruscotto-title-section"><h3 id="panel-title">Dashboard</h3></div>
                        <div class="cruscotto-buttons">
                            <button class="btn-action" onclick="fetchData(); fetchHw();">
                                <span class="material-symbols-outlined" style="font-size:16px;">refresh</span> Aggiorna Dati
                            </button>
                        </div>
                    </div>

                    <!-- DASHBOARD VIEW -->
                    <div id="view-dashboard" class="page-content-wrapper active-view">
                        <div class="dashboard">
                            <!-- TEMP -->
                            <div class="box">
                                <div style="display: flex; justify-content: space-between; align-items: stretch; width: 100%; height: 100%;">
                                    <div style="display: flex; flex-direction: column; justify-content: space-between; flex: 1;">
                                        <div>
                                            <div class="box-icon" style="background: rgba(239, 68, 68, 0.12); color: #ef4444; margin-bottom: 12px;"><i class="fa-solid fa-temperature-three-quarters"></i></div>
                                            <span class="box-title">Temperatura</span>
                                        </div>
                                        <div class="box-footer" style="border: none; padding-top: 0; margin-top: 15px; display: flex; flex-direction: column; gap: 4px;">
                                            <span class="box-footer-min">Min: <span id="min-temp">--</span></span>
                                            <span class="box-footer-max">Max: <span id="max-temp">--</span></span>
                                        </div>
                                    </div>
                                    <div style="display: flex; align-items: center; justify-content: center; width: 120px; min-width: 120px;">
                                        <svg width="110" height="110" viewBox="0 0 100 100">
                                            <circle cx="50" cy="50" r="38" fill="none" stroke="var(--border-color)" stroke-width="7" stroke-dasharray="179 238.7" stroke-linecap="round" transform="rotate(135 50 50)" />
                                            <circle id="bar-temp" cx="50" cy="50" r="38" fill="none" stroke="#ef4444" stroke-width="8" stroke-dasharray="179 238.7" stroke-dashoffset="179" stroke-linecap="round" transform="rotate(135 50 50)" style="transition: stroke-dashoffset 0.6s ease;" />
                                            <text x="50" y="55" text-anchor="middle" font-family="'JetBrains Mono', monospace" font-size="12" font-weight="800" fill="var(--text-primary)" id="val-temp">-- °C</text>
                                        </svg>
                                    </div>
                                </div>
                            </div>

                            <!-- HUM -->
                            <div class="box">
                                <div style="display: flex; justify-content: space-between; align-items: stretch; width: 100%; height: 100%;">
                                    <div style="display: flex; flex-direction: column; justify-content: space-between; flex: 1;">
                                        <div>
                                            <div class="box-icon" style="background: rgba(59, 130, 246, 0.12); color: #3b82f6; margin-bottom: 12px;"><i class="fa-solid fa-droplet"></i></div>
                                            <span class="box-title">Umidità Relativa</span>
                                        </div>
                                        <div class="box-footer" style="border: none; padding-top: 0; margin-top: 15px; display: flex; flex-direction: column; gap: 4px;">
                                            <span class="box-footer-min">Min: <span id="min-hum">--</span></span>
                                            <span class="box-footer-max">Max: <span id="max-hum">--</span></span>
                                        </div>
                                    </div>
                                    <div style="display: flex; align-items: center; justify-content: center; width: 120px; min-width: 120px;">
                                        <svg width="110" height="110" viewBox="0 0 100 100">
                                            <circle cx="50" cy="50" r="38" fill="none" stroke="var(--border-color)" stroke-width="7" stroke-dasharray="179 238.7" stroke-linecap="round" transform="rotate(135 50 50)" />
                                            <circle id="bar-hum" cx="50" cy="50" r="38" fill="none" stroke="#3b82f6" stroke-width="8" stroke-dasharray="179 238.7" stroke-dashoffset="179" stroke-linecap="round" transform="rotate(135 50 50)" style="transition: stroke-dashoffset 0.6s ease;" />
                                            <text x="50" y="55" text-anchor="middle" font-family="'JetBrains Mono', monospace" font-size="12" font-weight="800" fill="var(--text-primary)" id="val-hum">-- %</text>
                                        </svg>
                                    </div>
                                </div>
                            </div>

                            <!-- PRES -->
                            <div class="box">
                                <div style="display: flex; justify-content: space-between; align-items: stretch; width: 100%; height: 100%;">
                                    <div style="display: flex; flex-direction: column; justify-content: space-between; flex: 1;">
                                        <div>
                                            <div class="box-icon" style="background: rgba(139, 92, 246, 0.12); color: #8b5cf6; margin-bottom: 12px;"><i class="fa-solid fa-gauge"></i></div>
                                            <span class="box-title">Pressione</span>
                                        </div>
                                        <div class="box-footer" style="border: none; padding-top: 0; margin-top: 15px; display: flex; flex-direction: column; gap: 4px;">
                                            <span class="box-footer-min">Min: <span id="min-pres">--</span></span>
                                            <span class="box-footer-max">Max: <span id="max-pres">--</span></span>
                                        </div>
                                    </div>
                                    <div style="display: flex; align-items: center; justify-content: center; width: 120px; min-width: 120px;">
                                        <svg width="110" height="110" viewBox="0 0 100 100">
                                            <circle cx="50" cy="50" r="38" fill="none" stroke="var(--border-color)" stroke-width="7" stroke-dasharray="179 238.7" stroke-linecap="round" transform="rotate(135 50 50)" />
                                            <circle id="bar-pres" cx="50" cy="50" r="38" fill="none" stroke="#8b5cf6" stroke-width="8" stroke-dasharray="179 238.7" stroke-dashoffset="179" stroke-linecap="round" transform="rotate(135 50 50)" style="transition: stroke-dashoffset 0.6s ease;" />
                                            <text x="50" y="55" text-anchor="middle" font-family="'JetBrains Mono', monospace" font-size="10" font-weight="800" fill="var(--text-primary)" id="val-pres">-- hPa</text>
                                        </svg>
                                    </div>
                                </div>
                            </div>

                            <!-- CO2 -->
                            <div class="box">
                                <div style="display: flex; justify-content: space-between; align-items: stretch; width: 100%; height: 100%;">
                                    <div style="display: flex; flex-direction: column; justify-content: space-between; flex: 1;">
                                        <div>
                                            <div class="box-icon" style="background: rgba(16, 185, 129, 0.12); color: #10b981; margin-bottom: 12px;"><i class="fa-solid fa-leaf"></i></div>
                                            <span class="box-title" style="margin-bottom: 6px; display: block;">CO₂ / Gas</span>
                                            <span class="sensor-badge badge-normal" id="badge-gas">Ottima</span>
                                        </div>
                                        <div class="box-footer" style="border: none; padding-top: 0; margin-top: 15px; display: flex; flex-direction: column; gap: 4px;">
                                            <span class="box-footer-min">Min: <span id="min-gas">--</span></span>
                                            <span class="box-footer-max">Max: <span id="max-gas">--</span></span>
                                        </div>
                                    </div>
                                    <div style="display: flex; align-items: center; justify-content: center; width: 120px; min-width: 120px;">
                                        <svg width="110" height="110" viewBox="0 0 100 100">
                                            <circle cx="50" cy="50" r="38" fill="none" stroke="var(--border-color)" stroke-width="7" stroke-dasharray="179 238.7" stroke-linecap="round" transform="rotate(135 50 50)" />
                                            <circle id="bar-gas" cx="50" cy="50" r="38" fill="none" stroke="#10b981" stroke-width="8" stroke-dasharray="179 238.7" stroke-dashoffset="179" stroke-linecap="round" transform="rotate(135 50 50)" style="transition: stroke-dashoffset 0.6s ease;" />
                                            <text x="50" y="55" text-anchor="middle" font-family="'JetBrains Mono', monospace" font-size="11" font-weight="800" fill="var(--text-primary)" id="val-gas">-- PPM</text>
                                        </svg>
                                    </div>
                                </div>
                            </div>

                            <!-- RAIN -->
                            <div class="box">
                                <div class="box-header">
                                    <span class="box-title">Sensore Pioggia</span>
                                    <div class="box-icon" id="icon-rain-box"><i class="fa-solid fa-sun" id="icon-rain"></i></div>
                                </div>
                                <div class="box-value-container">
                                    <div style="display:flex; justify-content:space-between; align-items:center;">
                                        <div class="box-value" id="val-rain" style="font-size:28px;">Asciutto</div>
                                        <span class="sensor-badge badge-normal" id="badge-rain">No Pioggia</span>
                                    </div>
                                </div>
                                <div class="box-footer">
                                    <span>Stato Evento:</span>
                                    <span id="status-rain" style="font-weight:700;">Nessuna pioggia</span>
                                </div>
                            </div>

                            <!-- LDR -->
                            <div class="box">
                                <div class="box-header">
                                    <span class="box-title">Luminosità</span>
                                    <div class="box-icon" id="icon-ldr-box"><i class="fa-solid fa-sun" id="icon-ldr"></i></div>
                                </div>
                                <div class="box-value-container">
                                    <div style="display:flex; justify-content:space-between; align-items:center;">
                                        <div class="box-value" id="val-ldr" style="font-size:26px;">Giorno</div>
                                        <span class="sensor-badge badge-normal" id="badge-ldr">Luminoso</span>
                                    </div>
                                </div>
                                <div class="box-footer">
                                    <span>Stato Ambientale:</span>
                                    <span id="status-ldr" style="font-weight:700;">Luce solare</span>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- LOGS VIEW -->
                    <div id="view-logs" class="page-content-wrapper">
                        <div class="logs-layout">
                            <div class="log-card">
                                <h4><span class="material-symbols-outlined" style="color:var(--accent);">terminal</span> Monitor Seriale Hardware</h4>
                                <div class="terminal-display" id="serialMonitor">
                                    [INFO] Air-Link Weather Station v2.0.0 Online<br>
                                    [INFO] Inizializzazione completata con successo.<br>
                                    [SUCCESS] Connessione WiFi e Blynk Cloud attiva.
                                </div>
                            </div>
                            <div class="log-card">
                                <h4><span class="material-symbols-outlined" style="color:#10b981;">notifications_active</span> Eventi di Sistema</h4>
                                <div class="events-list" id="eventsList">
                                    <div class="event-item success"><span class="event-time">--:--:--</span><span class="event-msg">Sistema Operativo</span></div>
                                </div>
                            </div>
                        </div>
                    </div>

                    <!-- DATABASE VIEW -->
                    <div id="view-database" class="page-content-wrapper">
                        <div class="log-card" style="flex: 1; height: 100%; padding: 15px;">
                            <h4><span class="material-symbols-outlined" style="color:#f59e0b;">table_chart</span> Foglio Google Real-time</h4>
                            <div style="flex: 1; width: 100%; height: 100%; margin-top: 10px;">
                             <!-- su src scrivere il link pubblico del file Google Sheets -->
                                <iframe src="   " style="width: 100%; height: 100%; border: 1px solid var(--border-color); border-radius: 16px; background: #fff;" allowfullscreen></iframe>
                            </div>
                        </div>
                    </div>

                    <!-- HARDWARE VIEW -->
                    <div id="view-hardware" class="page-content-wrapper">
                        <div class="hardware-container">
                            <div class="hardware-grid">
                                <div class="box hardware-card">
                                    <div class="card-title">
                                        <i class="fa-solid fa-microchip"></i> Hardware ESP8266
                                    </div>
                                    <div class="hardware-info">
                                        <div class="hw-item"><i class="fa-solid fa-gauge-high"></i><div><small>CPU</small><strong id="cpuFreq">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-memory"></i><div><small>RAM Libera</small><strong id="freeRam">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-hard-drive"></i><div><small>Flash Size</small><strong id="flashSize">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-bolt"></i><div><small>Flash Speed</small><strong id="flashSpeed">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-wifi"></i><div><small>RSSI Wi-Fi</small><strong id="rssi">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-clock"></i><div><small>Uptime</small><strong id="uptime">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-network-wired"></i><div><small>IP Locale</small><strong id="ip">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-route"></i><div><small>Gateway</small><strong id="gateway">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-signal"></i><div><small>Canale Wi-Fi</small><strong id="channel">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-gear"></i><div><small>SDK Version</small><strong id="sdk">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-fingerprint"></i><div><small>Chip ID</small><strong id="chipid">--</strong></div></div>
                                        <div class="hw-item"><i class="fa-solid fa-rotate-right"></i><div><small>Ultimo Reset</small><strong id="resetReason">--</strong></div></div>
                                    </div>
                                </div>

                                <div class="box hardware-card">
                                    <div class="card-title">
                                        <i class="fa-solid fa-stethoscope"></i> Diagnostica Sensori
                                    </div>
                                    <div class="sensor-list">
                                        <div class="sensor"><span>DHT22</span><div class="status online" id="diag-dht"><i class="fa-solid fa-circle"></i> OK</div></div>
                                        <div class="sensor"><span>BME280</span><div class="status online" id="diag-bme"><i class="fa-solid fa-circle"></i> OK</div></div>
                                        <div class="sensor"><span>MQ135</span><div class="status online" id="diag-mq"><i class="fa-solid fa-circle"></i> OK</div></div>
                                        <div class="sensor"><span>Fotoresistore</span><div class="status online" id="diag-ldr"><i class="fa-solid fa-circle"></i> OK</div></div>
                                        <div class="sensor"><span>Sensore Pioggia</span><div class="status online" id="diag-rain"><i class="fa-solid fa-circle"></i> OK</div></div>
                                    </div>
                                    <button class="diag-button" onclick="runDiagnostics()">
                                        <i class="fa-solid fa-heart-pulse"></i> Avvia Diagnostica Completa
                                    </button>
                                </div>
                            </div>
                        </div>
                    </div>

                </div>
            </div>
        </div>
    </div>

    <script>
        function toggleMenu() {
            document.getElementById("sidebar").classList.toggle("active");
            document.getElementById("sidebarOverlay").classList.toggle("active");
        }

        function switchNav(element) {
            document.querySelectorAll(".sidebar li").forEach(item => item.classList.remove("active"));
            element.classList.add("active");
            const targetPage = element.getAttribute("data-page");
            document.getElementById("panel-title").textContent = element.textContent.trim();
            document.querySelectorAll(".page-content-wrapper").forEach(page => page.classList.remove("active-view"));
            const targetView = document.getElementById("view-" + targetPage);
            if (targetView) targetView.classList.add("active-view");
            if (window.innerWidth <= 900) toggleMenu();
        }

        function updateClock() {
            const now = new Date();
            document.getElementById("datetime-display").textContent = now.toLocaleString("it-IT");
        }
        setInterval(updateClock, 1000);
        updateClock();

        let themeSetting = localStorage.getItem("airlink-theme-pref") || "light";
        function setTheme(theme) {
            themeSetting = theme;
            localStorage.setItem("airlink-theme-pref", theme);
            document.getElementById("theme-btn-light").classList.toggle("active", theme === 'light');
            document.getElementById("theme-btn-dark").classList.toggle("active", theme === 'dark');
            document.body.classList.toggle("dark-theme", theme === 'dark');
        }
        setTheme(themeSetting);

        // Fetch Dati Sensori
        function fetchData() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var data = JSON.parse(this.responseText);
                    
                    // Temp
                    document.getElementById("val-temp").innerHTML = data.temp + " °C";
                    document.getElementById("min-temp").innerHTML = data.tempMin + " °C";
                    document.getElementById("max-temp").innerHTML = data.tempMax + " °C";
                    var offsetTemp = 179 - Math.min(Math.max((data.temp + 40) / 120, 0), 1) * 179;
                    document.getElementById("bar-temp").style.strokeDashoffset = offsetTemp;

                    // Hum
                    document.getElementById("val-hum").innerHTML = data.hum + " %";
                    document.getElementById("min-hum").innerHTML = data.humMin + " %";
                    document.getElementById("max-hum").innerHTML = data.humMax + " %";
                    var offsetHum = 179 - Math.min(Math.max(data.hum / 100, 0), 1) * 179;
                    document.getElementById("bar-hum").style.strokeDashoffset = offsetHum;

                    // Pressione (1 decimale)
                    document.getElementById("val-pres").innerHTML = Number(data.pres).toFixed(1) + " hPa";
                    document.getElementById("min-pres").innerHTML = Number(data.presMin).toFixed(1) + " hPa";
                    document.getElementById("max-pres").innerHTML = Number(data.presMax).toFixed(1) + " hPa";

                    var offsetPres = 179 - Math.min(Math.max((data.pres - 300) / 800, 0), 1) * 179;
                    document.getElementById("bar-pres").style.strokeDashoffset = offsetPres;


                   // CO2 (Numero intero)
                   document.getElementById("val-gas").innerHTML = Math.round(data.co2) + " PPM";
                   document.getElementById("min-gas").innerHTML = Math.round(data.co2Min) + " PPM";
                   document.getElementById("max-gas").innerHTML = Math.round(data.co2Max) + " PPM";

                   var offsetGas = 179 - Math.min(Math.max(data.co2 / 5000, 0), 1) * 179;
                   document.getElementById("bar-gas").style.strokeDashoffset = offsetGas;

                    // Rain & LDR
                    updateRainSensor(data.meteo);
                    // Nella funzione di fetch che riceve il JSON:
                    updateLdrSensor(data.luce);

                    // Status Online
                    document.getElementById("statusIndicator").classList.remove("offline");
                    document.getElementById("statusText").innerHTML = "ONLINE";
                }
            };
            xhttp.open("GET", "/api/data", true);
            xhttp.send();
        }

            // Aggiornamento del Monitor Seriale
      async function fetchSerialLogs() {
          try {
              const response = await fetch('/api/serial');
              if (!response.ok) return;
              const data = await response.json();
              
              const serialContainer = document.getElementById('serialMonitor');
              if (!serialContainer) return;
      
              if (!data.logs || data.logs.length === 0) {
                  serialContainer.innerHTML = '<span style="color:#666;">In attesa di messaggi...</span>';
                  return;
              }
      
              // Converte l'array di stringhe in righe formattate con colori
              serialContainer.innerHTML = data.logs.map(line => {
                  let color = '#d4d4d4'; // Bianco/Grigio standard
                  if (line.includes('❌') || line.includes('Errore') || line.includes('ERROR')) color = '#ef4444';
                  if (line.includes('✅') || line.includes('OK') || line.includes('SUCCESS')) color = '#10b981';
                  if (line.includes('⚠️') || line.includes('WARN')) color = '#f59e0b';
                  if (line.includes('🔍') || line.includes('🔍') || line.includes('INFO')) color = '#3b82f6';
      
                  return `<div style="color: ${color}; white-space: pre-wrap; font-family: monospace;">${line}</div>`;
              }).join('');
      
              // Auto-scroll automatico verso il basso
              serialContainer.scrollTop = serialContainer.scrollHeight;
          } catch (err) {
              console.error("Errore recupero Monitor Seriale:", err);
          }
      }
      
      // Aggiornamento degli Eventi di Sistema
      async function fetchSystemEvents() {
          try {
              const response = await fetch('/api/events');
              if (!response.ok) return;
              const data = await response.json();
              
              const eventsContainer = document.getElementById('eventsList');
              if (!eventsContainer) return;
      
              if (!data.events || data.events.length === 0) {
                  eventsContainer.innerHTML = '<div class="event-item"><span class="event-msg">Nessun evento registrato.</span></div>';
                  return;
              }
      
              // Mostra gli eventi più recenti in alto
              eventsContainer.innerHTML = data.events.map(ev => {
                  let statusClass = 'info';
                  if (ev.type === 'OK') statusClass = 'success';
                  if (ev.type === 'ERROR') statusClass = 'error';
      
                  return `
                      <div class="event-item ${statusClass}">
                          <span class="event-time">${ev.time}</span>
                          <span class="event-msg">${ev.msg}</span>
                      </div>
                  `;
              }).reverse().join('');
          } catch (err) {
              console.error("Errore recupero Eventi:", err);
          }
      }
      
      // Avvio del polling aggiornamento automatico ogni 2.5 secondi
      setInterval(() => {
          fetchSerialLogs();
          fetchSystemEvents();
      }, 2500);
      
      // Caricamento immediato all'apertura della pagina
      document.addEventListener("DOMContentLoaded", () => {
          fetchSerialLogs();
          fetchSystemEvents();
      });


        // Fetch Hardware metriche
        function fetchHw() {
            var xhttp = new XMLHttpRequest();
            xhttp.onreadystatechange = function () {
                if (this.readyState == 4 && this.status == 200) {
                    var hw = JSON.parse(this.responseText);
                    document.getElementById("cpuFreq").innerHTML = hw.cpuFreq;
                    document.getElementById("freeRam").innerHTML = hw.freeHeap;
                    document.getElementById("flashSize").innerHTML = hw.flashSize;
                    document.getElementById("flashSpeed").innerHTML = hw.flashSpeed;
                    document.getElementById("rssi").innerHTML = hw.rssi;
                    document.getElementById("uptime").innerHTML = hw.uptime;
                    document.getElementById("ip").innerHTML = hw.ip;
                    document.getElementById("gateway").innerHTML = hw.gateway;
                    document.getElementById("channel").innerHTML = hw.channel;
                    document.getElementById("sdk").innerHTML = hw.sdk;
                    document.getElementById("chipid").innerHTML = hw.chipId;
                    document.getElementById("resetReason").innerHTML = hw.resetReason;

                    document.getElementById("footer-ip").innerHTML = hw.ip;
                    document.getElementById("footer-rssi").innerHTML = hw.rssi;
                }
            };
            xhttp.open("GET", "/api/hw", true);
            xhttp.send();
        }

        function updateRainSensor(rainData) {
                const iconBox = document.getElementById("icon-rain-box");
                const icon = document.getElementById("icon-rain");
                const val = document.getElementById("val-rain");
                const badge = document.getElementById("badge-rain");
                const status = document.getElementById("status-rain");
            
                // Controlla se la stringa contiene "Pioggia" e NON "No Pioggia"
                const isRaining = (typeof rainData === "string") 
                    ? (rainData.includes("Pioggia") && !rainData.includes("No Pioggia"))
                    : Boolean(rainData);
            
                if (isRaining) {
                    iconBox.style.background = "rgba(59, 130, 246, 0.12)"; 
                    iconBox.style.color = "#3b82f6";
                    icon.className = "fa-solid fa-cloud-showers-heavy"; 
                    val.textContent = "Pioggia";
                    badge.textContent = "In Corso"; 
                    badge.className = "sensor-badge badge-danger";
                    status.textContent = "Precipitazione rilevata!";
                } else {
                    iconBox.style.background = "rgba(245, 158, 11, 0.12)"; 
                    iconBox.style.color = "#f59e0b";
                    icon.className = "fa-solid fa-sun"; 
                    val.textContent = "Asciutto";
                    badge.textContent = "No Pioggia"; 
                    badge.className = "sensor-badge badge-normal";
                    status.textContent = "Nessuna pioggia rilevata";
                }
            }

        function updateLdrSensor(luceData) {
            const iconBox = document.getElementById("icon-ldr-box");
            const icon = document.getElementById("icon-ldr");
            const val = document.getElementById("val-ldr");
            const badge = document.getElementById("badge-ldr");
            const status = document.getElementById("status-ldr");
        
            // Riconosce sia la stringa "Giorno" che il booleano true
            const isDaylight = (typeof luceData === "string") 
                ? luceData.includes("Giorno") 
                : Boolean(luceData);
        
            if (isDaylight) {
                iconBox.style.background = "rgba(234, 179, 8, 0.12)"; 
                iconBox.style.color = "#eab308";
                icon.className = "fa-solid fa-sun"; 
                val.textContent = "Luce Giorno";
                badge.textContent = "Luminoso"; 
                badge.className = "sensor-badge badge-normal";
                status.textContent = "Giorno (Sole)";
            } else {
                iconBox.style.background = "rgba(99, 102, 241, 0.12)"; 
                iconBox.style.color = "#6366f1";
                icon.className = "fa-solid fa-moon"; 
                val.textContent = "Buio Notte";
                badge.textContent = "Oscuro"; 
                badge.className = "sensor-badge badge-info";
                status.textContent = "Notte (Buio)";
            }
        }
        // funzione diagnostica
   function runDiagnostics() {
    const btn = document.querySelector(".diag-button");

    // Mappa degli elementi HTML
    const sensors = {
        dht22: document.getElementById("diag-dht"),
        bme280: document.getElementById("diag-bme"),
        mq135: document.getElementById("diag-mq"),
        ldr: document.getElementById("diag-ldr"),
        rain: document.getElementById("diag-rain")
    };

    // Helper per aggiornare la classe e il contenuto di ciascun badge
    function updateSensorStatus(element, isOk) {
        if (!element) return;
        if (isOk) {
            element.className = "status online";
            element.innerHTML = '<i class="fa-solid fa-circle"></i> OK';
        } else {
            element.className = "status offline";
            element.innerHTML = '<i class="fa-solid fa-circle-xmark"></i> ERRORE';
        }
    }

    // Feedback di caricamento sul pulsante
    btn.disabled = true;
    const originalText = btn.innerHTML;
    btn.innerHTML = '<i class="fa-solid fa-spinner fa-spin"></i> Analisi in corso...';

    fetch('/api/diagnostics')
        .then(response => {
            if (!response.ok) throw new Error("Errore risposta HTTP");
            return response.json();
        })
        .then(data => {
            // Aggiorna lo stato dei singoli sensori usando il JSON
            updateSensorStatus(sensors.dht22, data.dht22 === "ok");
            updateSensorStatus(sensors.bme280, data.bme280 === "ok");
            updateSensorStatus(sensors.mq135, data.mq135 === "ok");
            updateSensorStatus(sensors.ldr, data.ldr === "ok");
            updateSensorStatus(sensors.rain, data.rain === "ok");
        })
        .catch(err => {
            console.error("Errore diagnostica:", err);
            // In caso di mancata risposta della scheda imposta tutti a offline
            Object.values(sensors).forEach(el => updateSensorStatus(el, false));
        })
        .finally(() => {
            // Ripristina il pulsante
            btn.disabled = false;
            btn.innerHTML = originalText;
        });
}

       setInterval(fetchData, 30000);
        setInterval(fetchHw, 5000);
        fetchData();
        fetchHw();
        setInterval(fetchSerialLogs, 5000);
        setInterval(fetchSystemEvents, 10000);


   
  if ('serviceWorker' in navigator) {
    window.addEventListener('load', () => {
      navigator.serviceWorker.register('/sw.js')
        .then(reg => console.log('PWA Service Worker registrato:', reg))
        .catch(err => console.log('Errore SW:', err));
    });
  }
  
  </script>

    
</body>
</html>
)=====";

// Handler per servire la pagina principale
inline void handleRootPage() {
  server.send(200, "text/html", PAGE_INDEX);
}

// Handler Endpoint JSON dei Sensori
inline void handleApiData() {
  // Allocazione fissa dello stack
  StaticJsonDocument<384> doc;

  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  float pres = bme.readPressure() / 100.0F;
  float co2 = gasSensor.getCorrectedPPM(isnan(temp) ? 20.0 : temp, isnan(hum) ? 50.0 : hum);

  int pioggia = digitalRead(RAIN_SENSOR);
  int sole = digitalRead(FOTORESISTORE);

  doc[F("temp")] = isnan(temp) ? 0.0 : temp;
  doc[F("tempMin")] = tempMin;
  doc[F("tempMax")] = tempMax;
  doc[F("hum")] = isnan(hum) ? 0.0 : hum;
  doc[F("humMin")] = umidMin;
  doc[F("humMax")] = umidMax;
  doc[F("pres")] = isnan(pres) ? 0.0 : pres;
  doc[F("presMin")] = pressioneMin;
  doc[F("presMax")] = pressioneMax;
  doc[F("co2")] = isnan(co2) ? 0.0 : co2;
  
  doc[F("meteo")] = (pioggia == LOW ? F("🌧 Pioggia") : F("☀ No Pioggia"));
  doc[F("luce")] = (sole == LOW ? F("🌞 Giorno") : F("🌙 Notte"));

  // Serializza prima il JSON in una stringa temporanea per misurarne la lunghezza
  String jsonBuffer;
  serializeJson(doc, jsonBuffer);

  // Invia gli header HTTP corretti con Content-Length
  server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
  server.send(200, F("application/json"), jsonBuffer);
}

// Handler Endpoint JSON Hardware
inline void handleApiHw() {
  server.send(200, "application/json", getHardwareJson());
}

// Handler Endpoint Diagnostica
inline void handleApiDiagnostics() {
  StaticJsonDocument<256> doc;
  
  RisultatoDiagnostica res = eseguiControlloSensori();

  doc[F("dht22")] = res.dhtOk ? F("ok") : F("error");
  doc[F("bme280")] = res.bmeOk ? F("ok") : F("error");
  doc[F("mq135")] = res.mqOk ? F("ok") : F("error");
  doc[F("rain")] = res.rainOk ? F("ok") : F("error");
  doc[F("ldr")] = res.ldrOk ? F("ok") : F("error");
  doc[F("status")] = (res.erroriTotali == 0) ? F("ok") : F("error");
  doc[F("errorsCount")] = res.erroriTotali;

  String jsonBuffer;
  serializeJson(doc, jsonBuffer);
  server.sendHeader(F("Access-Control-Allow-Origin"), F("*"));
  server.send(200, F("application/json"), jsonBuffer);
}

// Handler 404
inline void handleNotFound() {
  server.send(404, "text/plain", "404 Not Found");
}

// Inizializzazione delle rotte
inline void initWebApp() {
  server.on("/", handleRootPage);
  server.on("/api/data", handleApiData);
  server.on("/api/hw", handleApiHw);
  server.on("/api/diagnostics", handleApiDiagnostics);
  server.onNotFound(handleNotFound);
  server.begin();
  
  // Stampa completa dell'indirizzo IP
  Serial.print("[WebApp] Server avviato con successo su http://");
  Serial.println(WiFi.localIP());
  logWebSerial("[WebApp] Server avviato su http://" + WiFi.localIP().toString());
}

void handleManifest() {
  static const char manifest_json[] PROGMEM = R"json({
  "name": "Air-Link Weather Station",
  "short_name": "Air-Link",
  "start_url": "/",
  "display": "standalone",
  "background_color": "#0f172a",
  "theme_color": "#1976d2",
  "icons": [
    {
      "src": "/icon-192.png",
      "sizes": "192x192",
      "type": "image/png",
      "purpose": "any maskable"
    }
  ]
})json";

  server.send_P(200, "application/json", manifest_json);
}
#endif // WEBAPP_H
