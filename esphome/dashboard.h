#pragma once

// HTML5 Dashboard for Clack Reader V4
// Served at /dashboard - auto-refreshes from /json

static const char DASHBOARD_HTML[] PROGMEM = R"rawhtml(
<!DOCTYPE html>
<html lang="en">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width,initial-scale=1">
<title>Clack Reader</title>
<style>
*{margin:0;padding:0;box-sizing:border-box}
:root{
  --bg:#0f172a;--card:#1e293b;--card-hover:#273548;
  --text:#e2e8f0;--text-dim:#94a3b8;--accent:#38bdf8;
  --green:#4ade80;--yellow:#facc15;--red:#f87171;--blue:#60a5fa;
  --radius:12px;--shadow:0 4px 24px rgba(0,0,0,.3);
}
body{font-family:-apple-system,BlinkMacSystemFont,'Segoe UI',Roboto,sans-serif;
  background:var(--bg);color:var(--text);min-height:100vh;padding:16px}
.header{text-align:center;padding:20px 0 10px;margin-bottom:20px}
.header h1{font-size:1.6em;font-weight:700;color:var(--accent);letter-spacing:-.5px}
.header .sub{font-size:.85em;color:var(--text-dim);margin-top:4px}
.status-bar{display:flex;justify-content:center;gap:16px;flex-wrap:wrap;
  margin-bottom:24px;font-size:.8em;color:var(--text-dim)}
.status-bar .dot{width:8px;height:8px;border-radius:50%;display:inline-block;
  margin-right:4px;vertical-align:middle}
.dot.online{background:var(--green);box-shadow:0 0 6px var(--green)}
.dot.offline{background:var(--red);box-shadow:0 0 6px var(--red)}
.grid{display:grid;grid-template-columns:repeat(auto-fill,minmax(280px,1fr));
  gap:16px;max-width:1200px;margin:0 auto}
.card{background:var(--card);border-radius:var(--radius);padding:20px;
  box-shadow:var(--shadow);transition:background .2s,transform .15s}
.card:hover{background:var(--card-hover);transform:translateY(-2px)}
.card .title{font-size:.75em;text-transform:uppercase;letter-spacing:1px;
  color:var(--text-dim);margin-bottom:10px;display:flex;align-items:center;gap:6px}
.card .title .icon{font-size:1.2em}
.card .value{font-size:2em;font-weight:700;line-height:1.1}
.card .unit{font-size:.45em;font-weight:400;color:var(--text-dim);margin-left:4px}
.card .sub-value{font-size:.8em;color:var(--text-dim);margin-top:6px}
.card.salt{border-left:3px solid var(--accent)}
.card.water{border-left:3px solid var(--blue)}
.card.power{border-left:3px solid var(--yellow)}
.card.system{border-left:3px solid var(--green)}
.card.alert{border-left:3px solid var(--red)}
.salt-bar{height:8px;border-radius:4px;background:#334155;margin-top:12px;overflow:hidden}
.salt-bar .fill{height:100%;border-radius:4px;transition:width .6s ease}
.links{text-align:center;margin-top:24px;padding:12px 0;font-size:.8em}
.links a{color:var(--accent);text-decoration:none;margin:0 12px}
.links a:hover{text-decoration:underline}
@media(max-width:600px){
  .card .value{font-size:1.5em}
  .grid{grid-template-columns:1fr}
}
</style>
</head>
<body>
<div class="header">
  <h1>&#x1F4A7; Clack Reader</h1>
  <div class="sub">Water Softener Dashboard</div>
</div>
<div class="status-bar">
  <span><span class="dot" id="dot"></span> <span id="conn">Connecting...</span></span>
  <span id="ts"></span>
  <span id="uptime"></span>
</div>
<div class="grid" id="grid"></div>
<div class="links">
  <a href="/">ESPHome Web UI</a>
  <a href="/json">Raw JSON</a>
  <a href="/config">Configuration</a>
</div>
<script>
const REFRESH=5000;
const CARDS=[
  {id:'salt_pct',key:'salt_level_percent',label:'Salt Level',icon:'\u{1F9C2}',
   cls:'salt',fmt:v=>v!=null?Math.round(v)+'<span class="unit">%</span>':'--',bar:true},
  {id:'salt_h',key:'salt_level_height',label:'Salt Height',icon:'\u{1F4CF}',
   cls:'salt',fmt:(v,d)=>v!=null?v.toFixed(1)+'<span class="unit">cm</span>':'--'},
  {id:'salt_d',key:'salt_level_distance',label:'Sensor Distance',icon:'\u{1F4E1}',
   cls:'salt',fmt:v=>v!=null?v.toFixed(1)+'<span class="unit">cm</span>':'--'},
  {id:'fill',key:'fill_salt',label:'Fill Salt?',icon:'\u{26A0}',
   cls:'alert',fmt:v=>v||'--'},
  {id:'wm',key:'water_meter',label:'Water Meter',icon:'\u{1F6B0}',
   cls:'water',fmt:v=>v!=null?v.toFixed(1)+'<span class="unit">L</span>':'--'},
  {id:'m3',key:'water_softener_m3_left',label:'Capacity Left',icon:'\u{1F4A7}',
   cls:'water',fmt:v=>v!=null?v.toFixed(2)+'<span class="unit">m\u00B3</span>':'--'},
  {id:'ltr',key:'water_softener_ltr_left',label:'Liters Left',icon:'\u{1F4A7}',
   cls:'water',fmt:v=>v!=null?Math.round(v)+'<span class="unit">L</span>':'--'},
  {id:'regen',key:'regenerated_on',label:'Last Regeneration',icon:'\u{1F504}',
   cls:'water',fmt:v=>v||'Never'},
  {id:'step',key:'cycle_step',label:'Cycle Step',icon:'\u{1F504}',
   cls:'water',fmt:v=>v||'Idle'},
  {id:'ttg',key:'time_to_regen',label:'Time to Regen',icon:'\u{23F0}',
   cls:'water',fmt:v=>v||'Unknown'},
  {id:'pwr',key:'power_clack',label:'Power',icon:'\u{26A1}',
   cls:'power',fmt:v=>v!=null?v.toFixed(2)+'<span class="unit">W</span>':'--'},
  {id:'volt',key:'voltage',label:'Voltage',icon:'\u{1F50B}',
   cls:'power',fmt:v=>v!=null?v.toFixed(1)+'<span class="unit">V</span>':'--'},
  {id:'wifi',key:'wifi_signal',label:'WiFi Signal',icon:'\u{1F4F6}',
   cls:'system',fmt:v=>v!=null?Math.round(v)+'<span class="unit">%</span>':'--'},
  {id:'ver',key:'version',label:'ESPHome Version',icon:'\u{2139}',
   cls:'system',fmt:v=>v||'--'},
  {id:'up',key:'uptime',label:'Uptime',icon:'\u{23F1}',
   cls:'system',fmt:v=>v!=null?v.toFixed(1)+'<span class="unit">days</span>':'--'},
  {id:'chl',key:'chlorinator',label:'Chlorinator',icon:'\u{1F50C}',
   cls:'power',fmt:v=>v===true?'<span style="color:var(--green)">ON</span>':'<span style="color:var(--text-dim)">OFF</span>'},
  {id:'mode',key:'function_mode',label:'Function Mode',icon:'\u{2699}',
   cls:'system',fmt:v=>v||'--'},
  {id:'hard_d',key:'water_hardness_d',label:'Water Hardness \u00b0D',icon:'\u{1F4A7}',
   cls:'water',fmt:v=>v!=null?v.toFixed(1)+'<span class="unit">\u00b0D</span>':'--'},
  {id:'hard_f',key:'water_hardness_f',label:'Water Hardness \u00b0F',icon:'\u{1F4A7}',
   cls:'water',fmt:v=>v!=null?v.toFixed(1)+'<span class="unit">\u00b0F</span>':'--'},
  {id:'hard_cat',key:'water_hardness_class',label:'Hardness Class',icon:'\u{1F3F7}',
   cls:'water',fmt:v=>v||'--'},
];

function initCards(){
  const g=document.getElementById('grid');
  CARDS.forEach(c=>{
    const d=document.createElement('div');
    d.className='card '+c.cls;
    d.id='card_'+c.id;
    d.innerHTML=`<div class="title"><span class="icon">${c.icon}</span> ${c.label}</div>
      <div class="value" id="val_${c.id}">--</div>
      ${c.bar?'<div class="salt-bar"><div class="fill" id="bar_'+c.id+'" style="width:0%;background:var(--accent)"></div></div>':''}`;
    g.appendChild(d);
  });
}

function barColor(pct){
  if(pct>50)return'var(--green)';
  if(pct>25)return'var(--yellow)';
  return'var(--red)';
}

function fmtUptime(s){
  if(s==null)return'';
  const d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60);
  return'Uptime: '+(d?d+'d ':'')+(h?h+'h ':'')+(m?m+'m':'0m');
}

async function refresh(){
  try{
    const r=await fetch('/json',{cache:'no-store'});
    if(!r.ok)throw new Error(r.status);
    const d=await r.json();
    document.getElementById('dot').className='dot online';
    document.getElementById('conn').textContent='Online';
    document.getElementById('ts').textContent=d.timestamp?d.timestamp.replace('T',' '):'';
    document.getElementById('uptime').textContent=fmtUptime(d.uptime_seconds);
    CARDS.forEach(c=>{
      const el=document.getElementById('val_'+c.id);
      if(!el)return;
      const v=d[c.key];
      el.innerHTML=c.fmt(v,d);
      if(c.bar){
        const bar=document.getElementById('bar_'+c.id);
        const pct=Math.max(0,Math.min(100,v||0));
        bar.style.width=pct+'%';
        bar.style.background=barColor(pct);
      }
    });
  }catch(e){
    document.getElementById('dot').className='dot offline';
    document.getElementById('conn').textContent='Offline';
  }
}

initCards();
refresh();
setInterval(refresh,REFRESH);
</script>
</body>
</html>
)rawhtml";
