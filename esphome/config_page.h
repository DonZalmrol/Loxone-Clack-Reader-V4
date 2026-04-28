#pragma once

// Configuration page for Clack Reader V4
// Served at /config - allows setting all entity values and WiFi credentials

static const char CONFIG_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Clack V4 - Configuration</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
:root{
  --bg:#0f172a;--card:#1e293b;--card-hover:#273548;
  --text:#e2e8f0;--text-dim:#94a3b8;--accent:#38bdf8;
  --green:#4ade80;--yellow:#facc15;--red:#f87171;--blue:#60a5fa;
  --input-bg:#334155;--input-border:#475569;
  --radius:12px;--shadow:0 4px 24px rgba(0,0,0,.3);
}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  background:var(--bg);color:var(--text);min-height:100vh;padding:16px}
.header{text-align:center;padding:20px 0 10px;margin-bottom:20px}
.header h1{font-size:1.6em;font-weight:700;color:var(--accent);letter-spacing:-.5px}
.header .sub{font-size:.85em;color:var(--text-dim);margin-top:4px}
.container{max-width:800px;margin:0 auto}
.section{margin-bottom:24px}
.section h2{font-size:1.1em;color:var(--accent);margin-bottom:12px;padding-bottom:8px;
  border-bottom:1px solid var(--input-border);display:flex;align-items:center;gap:8px}
.card{background:var(--card);border-radius:var(--radius);padding:16px 20px;
  box-shadow:var(--shadow);margin-bottom:10px;transition:background .2s}
.card:hover{background:var(--card-hover)}
.card .label{font-size:.8em;text-transform:uppercase;letter-spacing:.5px;
  color:var(--text-dim);margin-bottom:8px}
.card .row{display:flex;align-items:center;gap:12px}
.card .row input[type=range]{flex:1;accent-color:var(--accent);height:6px}
.card .row .val{min-width:70px;text-align:right;font-weight:600;font-size:1.1em}
.card .row .unit{color:var(--text-dim);font-size:.85em;margin-left:2px}
input[type=number],input[type=text],input[type=password]{
  background:var(--input-bg);border:1px solid var(--input-border);border-radius:8px;
  color:var(--text);padding:8px 12px;font-size:.95em;width:100%;outline:none;
  transition:border-color .2s}
input:focus{border-color:var(--accent)}
select{background:var(--input-bg);border:1px solid var(--input-border);border-radius:8px;
  color:var(--text);padding:8px 12px;font-size:.95em;width:100%;outline:none;
  appearance:none;cursor:pointer}
.toggle-wrap{display:flex;align-items:center;justify-content:space-between}
.toggle{position:relative;width:48px;height:26px;cursor:pointer}
.toggle input{display:none}
.toggle .slider{position:absolute;inset:0;background:var(--input-border);border-radius:13px;
  transition:.25s}
.toggle .slider:before{content:'';position:absolute;width:20px;height:20px;left:3px;
  bottom:3px;background:var(--text);border-radius:50%;transition:.25s}
.toggle input:checked+.slider{background:var(--accent)}
.toggle input:checked+.slider:before{transform:translateX(22px)}
.toggle-label{font-weight:600;font-size:1em}
.btn{background:var(--accent);color:#0f172a;border:none;border-radius:8px;
  padding:10px 24px;font-size:.95em;font-weight:600;cursor:pointer;
  transition:opacity .2s,transform .1s;display:inline-flex;align-items:center;gap:6px}
.btn:hover{opacity:.9;transform:translateY(-1px)}
.btn:active{transform:translateY(0)}
.btn.danger{background:var(--red)}
.btn.secondary{background:var(--input-border);color:var(--text)}
.btn-row{display:flex;gap:10px;margin-top:12px;flex-wrap:wrap}
.wifi-info{display:grid;grid-template-columns:auto 1fr;gap:6px 16px;font-size:.9em;margin-bottom:16px}
.wifi-info .lbl{color:var(--text-dim)}
.wifi-info .val{font-weight:600}
.wifi-form{display:grid;gap:10px}
.form-row{display:grid;gap:4px}
.form-row label{font-size:.8em;color:var(--text-dim);text-transform:uppercase;letter-spacing:.5px}
.toast{position:fixed;bottom:20px;right:20px;background:var(--green);color:#0f172a;
  padding:12px 20px;border-radius:8px;font-weight:600;font-size:.9em;
  transform:translateY(100px);opacity:0;transition:all .3s;z-index:1000}
.toast.show{transform:translateY(0);opacity:1}
.toast.error{background:var(--red);color:#fff}
.links{text-align:center;margin-top:24px;padding:12px 0;font-size:.8em}
.links a{color:var(--accent);text-decoration:none;margin:0 12px}
.links a:hover{text-decoration:underline}
.password-wrap{position:relative}
.password-wrap input{padding-right:40px}
.password-wrap .eye{position:absolute;right:10px;top:50%;transform:translateY(-50%);
  cursor:pointer;font-size:1.1em;color:var(--text-dim);user-select:none}
</style>
</head>
<body>
<div class="header">
  <h1>&#x2699; Configuration</h1>
  <div class="sub">Clack Reader V4 Settings</div>
</div>
<div class="container">
  <div class="section" id="numbers-section">
    <h2>&#x1F4CF; Sensor &amp; Capacity Settings</h2>
    <div id="numbers"></div>
  </div>
  <div class="section" id="selects-section">
    <h2>&#x1F527; Mode Settings</h2>
    <div id="selects"></div>
  </div>
  <div class="section" id="switches-section">
    <h2>&#x1F50C; Controls</h2>
    <div id="switches"></div>
  </div>
  <div class="section">
    <h2>&#x1F4F6; WiFi Configuration</h2>
    <div class="card">
      <div class="label">Current Connection</div>
      <div class="wifi-info" id="wifi-info">
        <span class="lbl">SSID:</span><span class="val" id="wifi-ssid">--</span>
        <span class="lbl">IP Address:</span><span class="val" id="wifi-ip">--</span>
        <span class="lbl">Gateway:</span><span class="val" id="wifi-gw">--</span>
        <span class="lbl">DNS:</span><span class="val" id="wifi-dns">--</span>
        <span class="lbl">Signal:</span><span class="val" id="wifi-rssi">--</span>
        <span class="lbl">MAC:</span><span class="val" id="wifi-mac">--</span>
      </div>
    </div>
    <div class="card">
      <div class="label">Change WiFi Network</div>
      <div class="wifi-form">
        <div class="form-row">
          <label>SSID (Network Name)</label>
          <input type="text" id="new-ssid" placeholder="Enter WiFi network name" autocomplete="off">
        </div>
        <div class="form-row">
          <label>Password</label>
          <div class="password-wrap">
            <input type="password" id="new-pass" placeholder="Enter WiFi password" autocomplete="off">
            <span class="eye" onclick="togglePass()">&#x1F441;</span>
          </div>
        </div>
        <div class="btn-row">
          <button class="btn" onclick="saveWifi()">&#x1F4BE; Save &amp; Reconnect</button>
          <button class="btn secondary" onclick="resetWifi()">&#x21A9; Reset to Default</button>
        </div>
        <div style="font-size:.75em;color:var(--text-dim);margin-top:4px">
          Device will restart and connect to the new network. If connection fails, it will fall back to AP mode (SSID: clack).
        </div>
      </div>
    </div>
  </div>
  <div class="section">
    <h2>&#x1F6E0; System</h2>
    <div class="card">
      <div class="btn-row">
        <button class="btn danger" onclick="restartDevice()">&#x1F504; Restart Device</button>
      </div>
    </div>
  </div>
</div>
<div class="links">
  <a href="/">ESPHome Web UI</a>
  <a href="/dashboard">Dashboard</a>
  <a href="/json">Raw JSON</a>
</div>
<div class="toast" id="toast"></div>
<script>
function toast(msg,error){
  const t=document.getElementById('toast');
  t.textContent=msg;
  t.className='toast'+(error?' error':'')+' show';
  setTimeout(()=>t.className='toast',3000);
}

function togglePass(){
  const p=document.getElementById('new-pass');
  p.type=p.type==='password'?'text':'password';
}

function renderNumber(n){
  const d=document.createElement('div');
  d.className='card';
  const step=n.step||0.1;
  d.innerHTML=`<div class="label">${esc(n.name)}</div>
    <div class="row">
      <input type="range" min="${n.min}" max="${n.max}" step="${step}"
        value="${n.state}" id="rng_${n.id}"
        oninput="document.getElementById('val_${n.id}').textContent=this.value">
      <span class="val" id="val_${n.id}">${n.state!=null?n.state:'--'}</span>
      <span class="unit">${esc(n.unit||'')}</span>
    </div>
    <div style="display:flex;gap:8px;margin-top:8px;align-items:center">
      <input type="number" min="${n.min}" max="${n.max}" step="${step}"
        value="${n.state}" id="num_${n.id}"
        style="width:120px"
        oninput="document.getElementById('rng_${n.id}').value=this.value;document.getElementById('val_${n.id}').textContent=this.value">
      <button class="btn" onclick="setNumber('${n.id}')">Set</button>
    </div>`;
  document.getElementById('numbers').appendChild(d);
  // Sync slider and input
  document.getElementById('rng_'+n.id).addEventListener('input',function(){
    document.getElementById('num_'+n.id).value=this.value;
  });
}

function renderSelect(s){
  const d=document.createElement('div');
  d.className='card';
  let opts=s.options.map(o=>`<option value="${esc(o)}"${o===s.state?' selected':''}>${esc(o)}</option>`).join('');
  d.innerHTML=`<div class="label">${esc(s.name)}</div>
    <div style="display:flex;gap:8px;align-items:center">
      <select id="sel_${s.id}" style="flex:1">${opts}</select>
      <button class="btn" onclick="setSelect('${s.id}')">Set</button>
    </div>`;
  document.getElementById('selects').appendChild(d);
}

function renderSwitch(s){
  const d=document.createElement('div');
  d.className='card';
  d.innerHTML=`<div class="toggle-wrap">
    <span class="toggle-label">${esc(s.name)}</span>
    <label class="toggle">
      <input type="checkbox" id="sw_${s.id}" ${s.state?'checked':''}
        onchange="setSwitch('${s.id}',this.checked)">
      <span class="slider"></span>
    </label>
  </div>`;
  document.getElementById('switches').appendChild(d);
}

function esc(s){return s?s.replace(/&/g,'&amp;').replace(/</g,'&lt;').replace(/>/g,'&gt;').replace(/"/g,'&quot;'):''}

async function setNumber(id){
  const v=document.getElementById('num_'+id).value;
  try{
    const r=await fetch('/api/set?domain=number&id='+encodeURIComponent(id)+'&value='+encodeURIComponent(v),{method:'POST'});
    if(r.ok)toast('Updated '+id);
    else toast('Failed: '+await r.text(),true);
  }catch(e){toast('Error: '+e.message,true)}
}

async function setSelect(id){
  const v=document.getElementById('sel_'+id).value;
  try{
    const r=await fetch('/api/set?domain=select&id='+encodeURIComponent(id)+'&option='+encodeURIComponent(v),{method:'POST'});
    if(r.ok)toast('Updated '+id);
    else toast('Failed: '+await r.text(),true);
  }catch(e){toast('Error: '+e.message,true)}
}

async function setSwitch(id,on){
  try{
    const r=await fetch('/api/set?domain=switch&id='+encodeURIComponent(id)+'&state='+(on?'on':'off'),{method:'POST'});
    if(r.ok)toast((on?'Enabled':'Disabled')+' '+id);
    else toast('Failed: '+await r.text(),true);
  }catch(e){toast('Error: '+e.message,true)}
}

async function saveWifi(){
  const ssid=document.getElementById('new-ssid').value.trim();
  const pass=document.getElementById('new-pass').value;
  if(!ssid){toast('Please enter an SSID',true);return}
  if(!confirm('Save WiFi credentials and restart?\n\nSSID: '+ssid+'\n\nThe device will restart and try to connect to the new network.'))return;
  try{
    const r=await fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({ssid:ssid,password:pass})});
    if(r.ok){toast('WiFi saved! Restarting...');
      setTimeout(()=>{document.getElementById('wifi-ssid').textContent='Restarting...'},1000);
    }else toast('Failed: '+await r.text(),true);
  }catch(e){toast('Error: '+e.message,true)}
}

async function resetWifi(){
  if(!confirm('Reset WiFi to firmware defaults and restart?'))return;
  try{
    const r=await fetch('/api/wifi',{method:'POST',headers:{'Content-Type':'application/json'},
      body:JSON.stringify({reset:true})});
    if(r.ok)toast('WiFi reset! Restarting...');
    else toast('Failed: '+await r.text(),true);
  }catch(e){toast('Error: '+e.message,true)}
}

async function restartDevice(){
  if(!confirm('Restart the device?'))return;
  try{
    await fetch('/api/restart',{method:'POST'});
    toast('Restarting...');
  }catch(e){toast('Restarting...')}
}

async function loadEntities(){
  try{
    const r=await fetch('/api/entities',{cache:'no-store'});
    const d=await r.json();
    if(d.numbers)d.numbers.forEach(renderNumber);
    if(d.selects)d.selects.forEach(renderSelect);
    if(d.switches)d.switches.forEach(renderSwitch);
    if(!d.numbers||!d.numbers.length)document.getElementById('numbers-section').style.display='none';
    if(!d.selects||!d.selects.length)document.getElementById('selects-section').style.display='none';
    if(!d.switches||!d.switches.length)document.getElementById('switches-section').style.display='none';
  }catch(e){toast('Failed to load entities',true)}
}

async function loadWifi(){
  try{
    const r=await fetch('/api/wifi',{cache:'no-store'});
    const d=await r.json();
    document.getElementById('wifi-ssid').textContent=d.ssid||'--';
    document.getElementById('wifi-ip').textContent=d.ip||'--';
    document.getElementById('wifi-gw').textContent=d.gateway||'--';
    document.getElementById('wifi-dns').textContent=d.dns||'--';
    document.getElementById('wifi-rssi').textContent=d.rssi!=null?d.rssi+' dBm':'--';
    document.getElementById('wifi-mac').textContent=d.mac||'--';
  }catch(e){}
}

loadEntities();
loadWifi();
</script>
</body>
</html>
)rawhtml";
