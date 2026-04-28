#pragma once

// HTML5 Dashboard for Clack Reader V4
// Served at /dashboard - auto-refreshes from /json
// Features: salt tank visualization, capacity gauges, cycle stepper, flow rate,
//           leakage banner, refresh indicator

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
  --input-border:#475569;
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
.refresh-info{font-size:.7em;color:var(--text-dim)}

/* Leakage banner */
.leak-banner{display:none;max-width:1200px;margin:0 auto 16px;padding:14px 20px;
  background:linear-gradient(90deg,#991b1b,#dc2626);border-radius:var(--radius);
  text-align:center;font-weight:700;font-size:1.1em;color:#fff;
  box-shadow:0 0 20px rgba(248,113,113,.4);animation:leak-flash 1.5s infinite}
.leak-banner.show{display:block}
@keyframes leak-flash{0%,100%{opacity:1}50%{opacity:.7}}

/* Hero section */
.hero{display:flex;gap:16px;max-width:1200px;margin:0 auto 20px;flex-wrap:wrap;justify-content:center}
.hero-card{background:var(--card);border-radius:var(--radius);padding:20px;
  box-shadow:var(--shadow);display:flex;flex-direction:column;align-items:center}
.hero-title{font-size:.75em;text-transform:uppercase;letter-spacing:1px;
  color:var(--text-dim);margin-bottom:12px;display:flex;align-items:center;gap:6px}

/* Tank */
.tank-card{min-width:180px}
.tank-wrap{position:relative;width:100px;margin:0 auto}
.tank-cap{width:60px;height:10px;margin:0 auto;background:var(--card-hover);
  border:2px solid var(--accent);border-bottom:none;border-radius:6px 6px 0 0}
.tank-body{position:relative;width:100px;height:170px;border:2px solid var(--accent);
  border-radius:0 0 12px 12px;overflow:hidden;background:#0c1829}
.tank-fill{position:absolute;bottom:0;left:0;right:0;
  background:linear-gradient(to top,#1e3a8a,#2563eb,#60a5fa);transition:height .8s ease}
.tank-fill.low{background:linear-gradient(to top,#991b1b,#dc2626,#f87171)}
.tank-fill.mid{background:linear-gradient(to top,#854d0e,#ca8a04,#facc15)}
.tank-wave{position:absolute;top:-6px;left:-5%;width:110%;height:12px;opacity:.6;
  background:repeating-radial-gradient(ellipse at 50% 100%,rgba(255,255,255,.3) 0,rgba(255,255,255,0) 70%);
  background-size:24px 12px;animation:wave 3s linear infinite}
@keyframes wave{0%{transform:translateX(0)}100%{transform:translateX(24px)}}
.tank-pct{position:absolute;inset:0;display:flex;align-items:center;justify-content:center;
  font-size:1.8em;font-weight:700;text-shadow:0 2px 8px rgba(0,0,0,.6);z-index:2}
.tank-sub{text-align:center;margin-top:8px;font-size:.85em;color:var(--text-dim)}
.tank-alert{text-align:center;margin-top:4px;font-size:.75em;color:var(--red);font-weight:600}

/* Gauges */
.gauges-card{min-width:280px}
.gauges-row{display:flex;gap:20px;justify-content:center}
.gauge{text-align:center}
.gauge svg{width:110px;height:110px}
.gauge-track{fill:none;stroke:#334155;stroke-width:8}
.gauge-fill{fill:none;stroke-width:8;stroke-linecap:round;transition:stroke-dashoffset .8s ease,stroke .3s}
.gauge-pct{fill:var(--text);font-size:22px;font-weight:700;dominant-baseline:central;text-anchor:middle}
.gauge-label{font-size:.7em;color:var(--text-dim);margin-top:6px}
.gauge-sub{font-size:.75em;color:var(--text-dim);margin-top:2px}

/* Flow rate */
.flow-card{min-width:160px;justify-content:center}
.flow-val{font-size:2.2em;font-weight:700;line-height:1.1}
.flow-unit{font-size:.4em;font-weight:400;color:var(--text-dim);margin-left:4px}
.flow-status{margin-top:8px;font-size:.8em;display:flex;align-items:center;gap:6px}
.flow-dot{width:8px;height:8px;border-radius:50%;background:var(--text-dim)}
.flow-dot.active{background:var(--blue);box-shadow:0 0 8px var(--blue);animation:pulse-blue 1.5s infinite}
@keyframes pulse-blue{0%,100%{box-shadow:0 0 6px var(--blue)}50%{box-shadow:0 0 16px var(--blue)}}

/* Cycle section */
.cycle-section{max-width:1200px;margin:0 auto 20px;background:var(--card);
  border-radius:var(--radius);padding:20px;box-shadow:var(--shadow)}
.cycle-header{display:flex;justify-content:space-between;align-items:center;
  flex-wrap:wrap;gap:8px;margin-bottom:16px}
.cycle-header .ch-title{font-size:.75em;text-transform:uppercase;letter-spacing:1px;
  color:var(--text-dim);display:flex;align-items:center;gap:6px}
.cycle-mode{font-size:.75em;color:var(--accent);background:rgba(56,189,248,.1);
  padding:3px 10px;border-radius:12px}
.cycle-timer{font-size:.85em;font-weight:600;color:var(--accent)}
.stepper{display:flex;align-items:flex-start;position:relative;overflow-x:auto;padding:8px 0}
.step{flex:1;display:flex;flex-direction:column;align-items:center;position:relative;min-width:70px}
.step-dot{width:32px;height:32px;border-radius:50%;border:3px solid var(--input-border);
  background:var(--card);display:flex;align-items:center;justify-content:center;
  font-size:.7em;font-weight:700;color:var(--text-dim);z-index:2;transition:all .3s}
.step.done .step-dot{border-color:var(--green);background:var(--green);color:#0f172a}
.step.active .step-dot{border-color:var(--accent);background:var(--accent);color:#0f172a;
  box-shadow:0 0 12px var(--accent);animation:pulse 2s infinite}
.step-line{position:absolute;top:16px;left:50%;width:100%;height:3px;
  background:var(--input-border);z-index:1}
.step:last-child .step-line{display:none}
.step.done .step-line{background:var(--green)}
.step.active .step-line{background:linear-gradient(90deg,var(--accent),var(--input-border))}
.step-name{font-size:.65em;color:var(--text-dim);margin-top:6px;text-align:center;white-space:nowrap}
.step.active .step-name{color:var(--accent);font-weight:600}
.step.done .step-name{color:var(--green)}
.step-time{font-size:.6em;color:var(--text-dim);margin-top:2px;text-align:center}
.step.active .step-time{color:var(--accent)}
.cycle-idle{text-align:center;padding:12px 0;color:var(--text-dim);font-size:.9em}
.cycle-idle .idle-icon{font-size:1.5em;margin-bottom:4px}
.cycle-last{display:flex;flex-wrap:wrap;gap:8px;justify-content:center;margin-top:10px;font-size:.8em}
.cycle-last-item{background:var(--card-hover);padding:5px 10px;border-radius:8px;
  display:flex;gap:6px;align-items:center}
.cycle-last-item .lbl{color:var(--text-dim)}
.cycle-last-item .val{font-weight:600}
@keyframes pulse{0%,100%{box-shadow:0 0 8px var(--accent)}50%{box-shadow:0 0 20px var(--accent)}}

/* Cards grid */
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
  .hero{flex-direction:column;align-items:stretch}
  .hero-card{min-width:auto}
  .gauges-row{flex-wrap:wrap}
  .flow-val{font-size:1.6em}
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
  <span id="uptime-bar"></span>
  <span class="refresh-info" id="refreshInfo"></span>
</div>

<!-- Leakage Alert Banner -->
<div class="leak-banner" id="leakBanner">&#x1F6A8; WATER LEAKAGE DETECTED &#x1F6A8;</div>

<!-- Hero: Tank + Gauges + Flow -->
<div class="hero">
  <div class="hero-card tank-card">
    <div class="hero-title">&#x1F9C2; Salt Level</div>
    <div class="tank-wrap">
      <div class="tank-cap"></div>
      <div class="tank-body">
        <div class="tank-fill" id="tankFill" style="height:0%">
          <div class="tank-wave"></div>
        </div>
        <div class="tank-pct" id="tankPct">--%</div>
      </div>
    </div>
    <div class="tank-sub" id="tankSub">-- cm</div>
    <div class="tank-alert" id="tankAlert"></div>
  </div>

  <div class="hero-card gauges-card">
    <div class="hero-title">&#x1F4A7; Capacity Remaining</div>
    <div class="gauges-row">
      <div class="gauge">
        <svg viewBox="0 0 120 120">
          <circle class="gauge-track" cx="60" cy="60" r="50"/>
          <circle class="gauge-fill" id="gaugeLFill" cx="60" cy="60" r="50"
            stroke-dasharray="314.16" stroke-dashoffset="314.16" transform="rotate(-90 60 60)"/>
          <text class="gauge-pct" id="gaugeLPct" x="60" y="60">--%</text>
        </svg>
        <div class="gauge-label">Water (Liters)</div>
        <div class="gauge-sub" id="gaugeLSub">-- L left</div>
      </div>
      <div class="gauge">
        <svg viewBox="0 0 120 120">
          <circle class="gauge-track" cx="60" cy="60" r="50"/>
          <circle class="gauge-fill" id="gaugeTFill" cx="60" cy="60" r="50"
            stroke-dasharray="314.16" stroke-dashoffset="314.16" transform="rotate(-90 60 60)"/>
          <text class="gauge-pct" id="gaugeTPct" x="60" y="60">--%</text>
        </svg>
        <div class="gauge-label">Time</div>
        <div class="gauge-sub" id="gaugeTSub">-- to regen</div>
      </div>
    </div>
  </div>

  <div class="hero-card flow-card">
    <div class="hero-title">&#x1F30A; Flow Rate</div>
    <div class="flow-val"><span id="flowVal">--</span><span class="flow-unit">L/min</span></div>
    <div class="flow-status">
      <span class="flow-dot" id="flowDot"></span>
      <span id="flowStatus">No flow</span>
    </div>
  </div>
</div>

<!-- Cycle Progress -->
<div class="cycle-section">
  <div class="cycle-header">
    <div class="ch-title">&#x1F504; Cleaning Cycle</div>
    <div class="cycle-mode" id="cycleMode">--</div>
    <div class="cycle-timer" id="cycleTimer"></div>
  </div>
  <div id="cycleBody"></div>
</div>

<!-- Info Cards -->
<div class="grid" id="grid"></div>

<div class="links">
  <a href="/">ESPHome Web UI</a>
  <a href="/json">Raw JSON</a>
  <a href="/config">Configuration</a>
</div>

<script>
var REFRESH=5000;
var CIRC=314.16;
var lastRefresh=0;

var CYCLE_MODES={
  'Upflow - Post fill':['Brine','Backwash','Rinse','Fill'],
  'Upflow - Pre fill':['Fill','Service','Brine','Backwash','Rinse'],
  'Downflow - Post fill':['Backwash','Brine','Backwash2','Rinse','Fill'],
  'Downflow - Pre fill':['Fill','Service','Backwash','Brine','Backwash2','Rinse']
};

var STEP_TIME_KEYS={
  Brine:'brine',Backwash:'backwash',Backwash2:'backwash2',
  Rinse:'rinse',Fill:'fill',Service:'service'
};

var CARDS=[
  {id:'salt_pct',key:'salt_level_percent',label:'Salt Level',icon:'\u{1F9C2}',
   cls:'salt',fmt:function(v){return v!=null?Math.round(v)+'<span class="unit">%</span>':'--'},bar:true},
  {id:'salt_h',key:'salt_level_height',label:'Salt Height',icon:'\u{1F4CF}',
   cls:'salt',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">cm</span>':'--'}},
  {id:'salt_d',key:'salt_level_distance',label:'Sensor Distance',icon:'\u{1F4E1}',
   cls:'salt',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">cm</span>':'--'}},
  {id:'fill_salt',key:'fill_salt',label:'Fill Salt?',icon:'\u{26A0}',
   cls:'alert',fmt:function(v){return v||'--'}},
  {id:'flow',key:'water_flow_rate',label:'Flow Rate',icon:'\u{1F30A}',
   cls:'water',fmt:function(v){return v!=null?v.toFixed(2)+'<span class="unit">L/min</span>':'--'}},
  {id:'wm',key:'water_meter',label:'Water Meter',icon:'\u{1F6B0}',
   cls:'water',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">L</span>':'--'}},
  {id:'m3',key:'water_softener_m3_left',label:'Capacity Left',icon:'\u{1F4A7}',
   cls:'water',fmt:function(v){return v!=null?v.toFixed(2)+'<span class="unit">m\u00B3</span>':'--'}},
  {id:'ltr',key:'water_softener_ltr_left',label:'Liters Left',icon:'\u{1F4A7}',
   cls:'water',fmt:function(v){return v!=null?Math.round(v)+'<span class="unit">L</span>':'--'}},
  {id:'pct_l',key:'water_softener_percent_ltr_left',label:'Capacity % (Liters)',icon:'\u{1F4CA}',
   cls:'water',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">%</span>':'--'}},
  {id:'pct_t',key:'water_softener_percent_time_left',label:'Capacity % (Time)',icon:'\u{1F4CA}',
   cls:'water',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">%</span>':'--'}},
  {id:'regen',key:'regenerated_on',label:'Last Regeneration',icon:'\u{1F504}',
   cls:'water',fmt:function(v){return v||'Never'}},
  {id:'step',key:'cycle_step',label:'Cycle Step',icon:'\u{1F504}',
   cls:'water',fmt:function(v){return v||'Idle'}},
  {id:'ttg',key:'time_to_regen',label:'Time to Regen',icon:'\u{23F0}',
   cls:'water',fmt:function(v){return v||'Unknown'}},
  {id:'cap_used',key:'capacity_used',label:'Capacity Used (last)',icon:'\u{1F4C8}',
   cls:'water',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">L</span>':'--'}},
  {id:'op_time',key:'operation_time',label:'Operation Time (last)',icon:'\u{23F3}',
   cls:'water',fmt:function(v){return v||'--'}},
  {id:'ttrc',key:'time_to_resinclean',label:'Time to Resinclean',icon:'\u{1F9EA}',
   cls:'water',fmt:function(v){return v||'Unknown'}},
  {id:'rc_last',key:'resinclean_on',label:'Last Resinclean',icon:'\u{1F9EA}',
   cls:'water',fmt:function(v){return v||'Never'}},
  {id:'sf_last',key:'salt_fill_on',label:'Last Salt Fill',icon:'\u{1F9C2}',
   cls:'salt',fmt:function(v){return v||'Never'}},
  {id:'pwr',key:'power_clack',label:'Power Clack',icon:'\u{26A1}',
   cls:'power',fmt:function(v){return v!=null?v.toFixed(2)+'<span class="unit">W</span>':'--'}},
  {id:'pwr_esp',key:'power_esp',label:'Power ESP',icon:'\u{26A1}',
   cls:'power',fmt:function(v){return v!=null?v.toFixed(2)+'<span class="unit">W</span>':'--'}},
  {id:'pwr_chl',key:'power_chlorinator',label:'Power Chlorinator',icon:'\u{26A1}',
   cls:'power',fmt:function(v){return v!=null?v.toFixed(2)+'<span class="unit">W</span>':'--'}},
  {id:'volt',key:'voltage',label:'Voltage',icon:'\u{1F50B}',
   cls:'power',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">V</span>':'--'}},
  {id:'temp',key:'internal_temperature',label:'ESP Temperature',icon:'\u{1F321}',
   cls:'system',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">\u00b0C</span>':'--'}},
  {id:'wifi',key:'wifi_signal',label:'WiFi Signal',icon:'\u{1F4F6}',
   cls:'system',fmt:function(v){return v!=null?Math.round(v)+'<span class="unit">%</span>':'--'}},
  {id:'ver',key:'version',label:'ESPHome Version',icon:'\u{2139}',
   cls:'system',fmt:function(v){return v||'--'}},
  {id:'up',key:'uptime',label:'Uptime',icon:'\u{23F1}',
   cls:'system',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">days</span>':'--'}},
  {id:'chl',key:'chlorinator',label:'Chlorinator',icon:'\u{1F50C}',
   cls:'power',fmt:function(v){return v===true?'<span style="color:var(--green)">ON</span>':'<span style="color:var(--text-dim)">OFF</span>'}},
  {id:'mode',key:'function_mode',label:'Function Mode',icon:'\u{2699}',
   cls:'system',fmt:function(v){return v||'--'}},
  {id:'hard_d',key:'water_hardness___d',label:'Water Hardness \u00b0D',icon:'\u{1F4A7}',
   cls:'water',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">\u00b0D</span>':'--'}},
  {id:'hard_f',key:'water_hardness___f',label:'Water Hardness \u00b0F',icon:'\u{1F4A7}',
   cls:'water',fmt:function(v){return v!=null?v.toFixed(1)+'<span class="unit">\u00b0F</span>':'--'}},
  {id:'hard_cat',key:'water_hardness_class',label:'Hardness Class',icon:'\u{1F3F7}',
   cls:'water',fmt:function(v){return v||'--'}}
];

function gaugeColor(pct){
  if(pct>50)return'var(--green)';
  if(pct>25)return'var(--yellow)';
  return'var(--red)';
}
function barColor(pct){
  if(pct>50)return'var(--green)';
  if(pct>25)return'var(--yellow)';
  return'var(--red)';
}
function fmtUptime(s){
  if(s==null)return'';
  var d=Math.floor(s/86400),h=Math.floor(s%86400/3600),m=Math.floor(s%3600/60);
  return'Uptime: '+(d?d+'d ':'')+(h?h+'h ':'')+(m?m+'m':'0m');
}

function initCards(){
  var g=document.getElementById('grid');
  CARDS.forEach(function(c){
    var d=document.createElement('div');
    d.className='card '+c.cls;
    d.id='card_'+c.id;
    d.innerHTML='<div class="title"><span class="icon">'+c.icon+'</span> '+c.label+'</div>'+
      '<div class="value" id="val_'+c.id+'">--</div>'+
      (c.bar?'<div class="salt-bar"><div class="fill" id="bar_'+c.id+'" style="width:0%;background:var(--accent)"></div></div>':'');
    g.appendChild(d);
  });
}

function updateLeakage(d){
  var banner=document.getElementById('leakBanner');
  if(d.leakage_detected===true){
    banner.className='leak-banner show';
  }else{
    banner.className='leak-banner';
  }
}

function updateTank(d){
  var pct=d.salt_level_percent!=null?Math.max(0,Math.min(100,Math.round(d.salt_level_percent))):null;
  var h=d.salt_level_height;
  var fill=document.getElementById('tankFill');
  var pctEl=document.getElementById('tankPct');
  var sub=document.getElementById('tankSub');
  var alertEl=document.getElementById('tankAlert');
  if(pct!=null){
    fill.style.height=pct+'%';
    fill.className='tank-fill'+(pct<=25?' low':pct<=50?' mid':'');
    pctEl.textContent=pct+'%';
  }else{
    pctEl.textContent='--%';
  }
  sub.textContent=h!=null?h.toFixed(1)+' cm':'-- cm';
  var fs=d.fill_salt;
  if(fs&&(fs.toLowerCase()==='yes'||fs.toLowerCase()==='ja')){
    alertEl.textContent='\u26A0 Fill salt!';
  }else{
    alertEl.textContent='';
  }
}

function updateGauges(d){
  var lPct=d.water_softener_percent_ltr_left;
  var gLF=document.getElementById('gaugeLFill');
  var gLP=document.getElementById('gaugeLPct');
  var gLS=document.getElementById('gaugeLSub');
  if(lPct!=null){
    var p=Math.max(0,Math.min(100,lPct));
    gLF.style.strokeDashoffset=CIRC-(CIRC*p/100);
    gLF.style.stroke=gaugeColor(p);
    gLP.textContent=Math.round(p)+'%';
  }else{
    gLP.textContent='--%';
  }
  var lLeft=d.water_softener_ltr_left;
  gLS.textContent=lLeft!=null?Math.round(lLeft)+' L left':'-- L left';

  var tPct=d.water_softener_percent_time_left;
  var gTF=document.getElementById('gaugeTFill');
  var gTP=document.getElementById('gaugeTPct');
  var gTS=document.getElementById('gaugeTSub');
  if(tPct!=null){
    var p2=Math.max(0,Math.min(100,tPct));
    gTF.style.strokeDashoffset=CIRC-(CIRC*p2/100);
    gTF.style.stroke=gaugeColor(p2);
    gTP.textContent=Math.round(p2)+'%';
  }else{
    gTP.textContent='--%';
  }
  var ttg=d.time_to_regen;
  gTS.textContent=ttg||'-- to regen';
}

function updateFlow(d){
  var rate=d.water_flow_rate;
  var flowing=d.water_flowing;
  document.getElementById('flowVal').textContent=rate!=null?rate.toFixed(2):'--';
  var dot=document.getElementById('flowDot');
  var status=document.getElementById('flowStatus');
  if(flowing===true||(rate!=null&&rate>0)){
    dot.className='flow-dot active';
    status.textContent='Flowing';
    status.style.color='var(--blue)';
  }else{
    dot.className='flow-dot';
    status.textContent='No flow';
    status.style.color='var(--text-dim)';
  }
}

function updateCycle(d){
  var step=d.cycle_step||'Idle';
  var mode=d.regeneration_mode||'';
  var body=document.getElementById('cycleBody');
  var modeEl=document.getElementById('cycleMode');
  var timerEl=document.getElementById('cycleTimer');
  modeEl.textContent=mode||'--';

  if(step==='Idle'){
    timerEl.textContent='';
    var html='<div class="cycle-idle"><div class="idle-icon">\u2705</div>Idle \u2014 No active cycle</div>';
    var lastTimes=[];
    for(var name in STEP_TIME_KEYS){
      var t=d[STEP_TIME_KEYS[name]];
      if(t&&t!=='0s'&&t!=='0m 0s'&&t!=='0h 0m 0s')lastTimes.push({name:name,time:t});
    }
    if(lastTimes.length>0){
      html+='<div style="text-align:center;font-size:.7em;color:var(--text-dim);margin-top:4px">Last cycle step times:</div>';
      html+='<div class="cycle-last">';
      lastTimes.forEach(function(lt){
        html+='<div class="cycle-last-item"><span class="lbl">'+lt.name+'</span><span class="val">'+lt.time+'</span></div>';
      });
      if(d.run_time&&d.run_time!=='0s')
        html+='<div class="cycle-last-item"><span class="lbl">Total</span><span class="val">'+d.run_time+'</span></div>';
      html+='</div>';
    }
    body.innerHTML=html;
  }else{
    var steps=CYCLE_MODES[mode]||['Brine','Backwash','Rinse','Fill'];
    var activeIdx=steps.indexOf(step);
    var rt=d.cycle_runtime||'';
    var trt=d.cycle_total_runtime||'';
    timerEl.innerHTML=rt?'\u23F1 '+rt+(trt?' \u00b7 Total: '+trt:''):'';

    var html2='<div class="stepper">';
    steps.forEach(function(s,i){
      var cls='step';
      if(i<activeIdx)cls+=' done';
      else if(i===activeIdx)cls+=' active';
      var timeKey=STEP_TIME_KEYS[s];
      var timeVal=(i<activeIdx&&d[timeKey])?d[timeKey]:'';
      html2+='<div class="'+cls+'">'+
        '<div class="step-line"></div>'+
        '<div class="step-dot">'+(i+1)+'</div>'+
        '<div class="step-name">'+s+'</div>'+
        '<div class="step-time">'+(i===activeIdx?rt:timeVal)+'</div>'+
        '</div>';
    });
    html2+='</div>';
    body.innerHTML=html2;
  }
}

function updateRefreshInfo(){
  var el=document.getElementById('refreshInfo');
  if(!lastRefresh){el.textContent='';return;}
  var ago=Math.round((Date.now()-lastRefresh)/1000);
  el.textContent='Updated '+ago+'s ago';
}

async function refresh(){
  try{
    var r=await fetch('/json',{cache:'no-store'});
    if(!r.ok)throw new Error(r.status);
    var d=await r.json();
    lastRefresh=Date.now();
    document.getElementById('dot').className='dot online';
    document.getElementById('conn').textContent='Online';
    document.getElementById('ts').textContent=d.timestamp?d.timestamp.replace('T',' '):'';
    document.getElementById('uptime-bar').textContent=fmtUptime(d.uptime_seconds);

    updateLeakage(d);
    updateTank(d);
    updateGauges(d);
    updateFlow(d);
    updateCycle(d);

    CARDS.forEach(function(c){
      var el=document.getElementById('val_'+c.id);
      if(!el)return;
      var v=d[c.key];
      el.innerHTML=c.fmt(v,d);
      if(c.bar){
        var bar=document.getElementById('bar_'+c.id);
        var pct=Math.max(0,Math.min(100,v||0));
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
setInterval(updateRefreshInfo,1000);
</script>
</body>
</html>
)rawhtml";
