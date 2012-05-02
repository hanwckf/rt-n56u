/*
	Tomato GUI
	Copyright (C) 2006-2009 Jonathan Zarate
	http://www.polarcloud.com/tomato/

	For use with Tomato Firmware only.
	No part of this file may be used without permission.
*/

var tabs = [];
var rx_max, rx_avg;
var tx_max, tx_avg;
var xx_max = 0;
var ifname;
var htmReady = 0;
var svgReady = 0;
var updating = 0;
var scaleMode = 0;
var scaleLast = -1;
var drawMode = 0;
var drawLast = -1;
var drawColor = 0;
var drawColorRX = 0;	//Viz add 2010.09
var drawColorTX = 0;	//Viz add 2010.09
var avgMode = 0;
var avgLast = -1;
var colorX = 0;
var colors = [
	['Green &amp; Blue', '#118811', '#6495ed'], ['Blue &amp; Orange', '#003EBA', '#FF9000'],
	['Blue &amp; Red', '#003EDD', '#CC4040'], ['Blue', '#22f', '#225'], ['Gray', '#000', '#999'],
	['Red &amp; Black', '#d00', '#000']];
// Viz add colorRX�BcoloTX 2010.09
// 0Orange 1Blue 2Black 3Red 4Gray  5Green
var colorRX = [ '#FF9000', '#003EBA', '#000000',  '#dd0000', '#999999',  '#118811'];
var colorTX = ['#FF9000', '#003EBA', '#000000',  '#dd0000', '#999999',  '#118811'];

function xpsb(byt)
{
/* REMOVE-BEGIN
	kbit/s = 1000 bits/s
	125 = 1000 / 8
	((B * 8) / 1000)
REMOVE-END */
	return (byt / 1024).toFixed(2) + ' <small>KB/s</small>';
}

function showCTab()
{
	showTab('speed-tab-' + ifname);
}

function showSelectedOption(prefix, prev, now)
{
	var e;

	elem.removeClass(prefix + prev, 'selected');	// safe if prev doesn't exist
	if ((e = E(prefix + now)) != null) {
		elem.addClass(e, 'selected');
		e.blur();
	}
}

function showDraw()
{
	if (drawLast == drawMode) return;
	showSelectedOption('draw', drawLast, drawMode);
	drawLast = drawMode;
}

function switchDraw(n)
{
	if ((!svgReady) || (updating)) return;
	drawMode = n;
	showDraw();
	showCTab();
	cookie.set(cprefix + 'draw', drawMode);
}

/*			//Banned by Viz 2010.09           vvvvvvvvvv
function showColor()
{	
	E('drawcolor').innerHTML = colors[drawColor][0] + ' &raquo;';	//>>
	E('rx-name').style.borderBottom = '2px dashed ' + colors[drawColor][1 + colorX];
	E('tx-name').style.borderBottom = '2px dashed ' + colors[drawColor][1 + (colorX ^ 1)];
	
}

function switchColor(rev)
{
	if ((!svgReady) || (updating)) return;
	
	drawColor = rev;
	showColor();
	showCTab();
	cookie.set(cprefix + 'color', drawColor + ',' + colorX);
}
*/				//Banned by Viz 2010.09   ^^^^^^^^


// Viz add 2010.09  vvvvvvvvvv
function showColor()
{	
	//E('drawcolor').innerHTML = colors[drawColor][0] + ' &raquo;';	//>>
	E('rx-name').style.borderBottom = '2px dashed ' + colorRX[drawColorRX];
	E('tx-name').style.borderBottom = '2px dashed ' + colorTX[drawColorTX];
	E('rx-sel').style.background =colorRX[drawColorRX];
	E('tx-sel').style.background =colorTX[drawColorTX];	

}

function switchColorRX(rev)
{
	if ((!svgReady) || (updating)) return;
	
	drawColorRX = rev;
	showColor();
	showCTab();
	//cookie.set(cprefix + 'color', drawColorRX + ',' + colorX);
	//E('rx-sel').style.background-color =colorRX[rev];
	
}

function switchColorTX(rev)
{
	if ((!svgReady) || (updating)) return;
	
	drawColorTX = rev;
	showColor();
	showCTab();
	//cookie.set(cprefix + 'color', drawColorTX + ',' + colorX);
}
// Viz add 2010.09 ^^^^^^^^^^

function showScale()
{
	if (scaleMode == scaleLast) return;
	showSelectedOption('scale', scaleLast, scaleMode);
	scaleLast = scaleMode;
}

function switchScale(n)
{
	scaleMode = n;
	showScale();
	showTab('speed-tab-' + ifname);
	cookie.set(cprefix + 'scale', scaleMode);
}

function showAvg()
{
	if (avgMode == avgLast) return;
	showSelectedOption('avg', avgLast, avgMode);
	avgLast = avgMode;
}

function switchAvg(n)
{
	if ((!svgReady) || (updating)) return;
	avgMode = n;
	showAvg();
	showCTab();
	cookie.set(cprefix + 'avg', avgMode);
}

function tabSelect(name)
{
	if (!updating) showTab(name);
}

function showTab(name)
{
	var h;
	var max;
	var i;
	var rx, tx;
	var e;

	ifname = name.replace('speed-tab-', '');
	cookie.set(cprefix + 'tab', ifname, 14);
	tabHigh(name);

	h = speed_history[ifname];
	if (!h) return;

	E('rx-current').innerHTML = xpsb(h.rx[h.rx.length - 1] / updateDiv); 
	E('rx-avg').innerHTML = xpsb(h.rx_avg);
	E('rx-max').innerHTML = xpsb(h.rx_max);

	E('tx-current').innerHTML = xpsb(h.tx[h.tx.length - 1] / updateDiv);
	E('tx-avg').innerHTML = xpsb(h.tx_avg);
	E('tx-max').innerHTML = xpsb(h.tx_max);

	E('rx-total').innerHTML = scaleSize(h.rx_total);
	E('tx-total').innerHTML = scaleSize(h.tx_total);

	if (svgReady) {
		max = scaleMode ? MAX(h.rx_max, h.tx_max) : xx_max
		if (max > 12500) max = Math.round((max + 12499) / 12500) * 12500;
			else max += 100;
	/*
		updateSVG(h.rx, h.tx, max, drawMode,
			colors[drawColor][1 + colorX], colors[drawColor][1 + (colorX ^ 1)],
			updateInt, updateMaxL, updateDiv, avgMode, clock);
	*/
		updateSVG(h.rx, h.tx, max, drawMode,
			colorRX[drawColorRX], colorTX[drawColorTX],
			updateInt, updateMaxL, updateDiv, avgMode, clock);	
	}
}

function loadData()
{
	var old;
	var t, e;
	var name;
	var i;
	var changed;

	xx_max = 0;
	old = tabs;
	tabs = [];
	clock = new Date();

	if (!speed_history) {
		speed_history = [];
	}
	else {
		for (var i in speed_history) {
			var h = speed_history[i];
			if ((typeof(h.rx) == 'undefined') || (typeof(h.tx) == 'undefined')) {
				delete speed_history[i];
				continue;
			}

			if (updateReTotal) {
				h.rx_total = h.rx_max = 0;
				h.tx_total = h.tx_max = 0;
				for (j = (h.rx.length - updateMaxL); j < h.rx.length; ++j) {
					t = h.rx[j];
					if (t > h.rx_max) h.rx_max = t;
					h.rx_total += t;
					t = h.tx[j];
					if (t > h.tx_max) h.tx_max = t;
					h.tx_total += t;
				}
				h.rx_avg = h.rx_total / updateMaxL;
				h.tx_avg = h.tx_total / updateMaxL;
			}

			if (updateDiv > 1) {
				h.rx_max /= updateDiv;
				h.tx_max /= updateDiv;
				h.rx_avg /= updateDiv;
				h.tx_avg /= updateDiv;
			}
			if (h.rx_max > xx_max) xx_max = h.rx_max;
			if (h.tx_max > xx_max) xx_max = h.tx_max;

			t = i;
			//if (i == nvram.wl_ifname) {
			if (i == "ra0")
				t = 'Wireless <small>(5GHz)</small>';
			else if (i == "rai0")
				t = 'Wireless <small>(2.4GHz)</small>';
			else if (i == "eth2")
				t = 'Wired';
			else if (i == "br0")				
				t = 'LAN';
			else if ((wan_proto == 'pptp') || (wan_proto == 'pppoe') || (wan_proto == 'l2tp')){
				if (i.indexOf('eth3') == 0) t = ' Internet'; // keep the space!
			}
			else if (nvram.wan_proto != 'disabled'){ 
				if (nvram.wan_ifname == i) t = ' Internet';
			}
			 
			// Viz 2010.09 added if loop 
			// Viz 2011.08 showhide by <% check_hwnat(); %> 
			// ||i=='lo''wds0''wds1''wds2''wds3''wdsi0''wdsi1''wdsi2''wdsi3'
			var chk_hwnat = '<% check_hwnat(); %>';			
			var chk_qos_enable = '<% nvram_get_x("",  "qos_global_enable"); %>';
			var preferred_lang = '<% nvram_get_x("",  "preferred_lang"); %>';
			if(preferred_lang=="JP"){
				if(chk_hwnat==1 || chk_qos_enable==0){
					if (i=='ra0'||i=='rai0')
						tabs.push(['speed-tab-' + i, t]);
				}else{
					if (i=='eth3'||i=='br0'||i=='eth2'||i=='ra0'||i=='rai0')
						tabs.push(['speed-tab-' + i, t]);
				}
			}else{
					if (i=='eth3'||i=='br0'||i=='eth2'||i=='ra0'||i=='rai0')
						tabs.push(['speed-tab-' + i, t]);				
			}
				
		}

		tabs = tabs.sort(
			function(a, b) {
				if (a[1] < b[1]) return -1;
				if (a[1] > b[1]) return 1;
				return 0;
			});
	}

	if (tabs.length == old.length) {
		for (i = tabs.length - 1; i >= 0; --i)
			if (tabs[i][0] != old[i][0]) break;
		changed = i > 0;
	}
	else changed = 1;

	if (changed) {
		E('tab-area').innerHTML = _tabCreate.apply(this, tabs);
	}
	if (((name = cookie.get(cprefix + 'tab')) != null) && ((speed_history[name] != undefined))) {
		showTab('speed-tab-' + name);
		return;
	}
	if (tabs.length) showTab(tabs[0][0]);
}

function initData()
{
	if (htmReady) {
		loadData();
		if (svgReady) {
			E('graph').style.visibility = 'visible';
			E('bwm-controls').style.visibility = 'visible';
		}
	}
}

function initCommon(defAvg, defDrawMode, defDrawColorRX, defDrawColorTX) //Viz modify defDrawColor 2010.09
{
	drawMode = fixInt(cookie.get(cprefix + 'draw'), 0, 1, defDrawMode);
	showDraw();

	var c = nvram.rstats_colors.split(',');
	/*
	while (c.length >= 3) {
		c[0] = escapeHTML(c[0]);
		colors.push(c.splice(0, 3));
	} */

	c = (cookie.get(cprefix + 'color') || '').split(',');
	/* alert(c);	//3,0 */

/*			//Banned by Viz 2010.09	
	if (c.length == 2) {
		//drawColor = fixInt(c[0], 0, colors.length - 1, defDrawColor);
		//colorX = fixInt(c[1], 0, 1, 0);    // Viz modify drawColorRX TX 2010.09		
		drawColorRX = fixInt(c[0], 0, colorRX.length - 1, defDrawColorRX);
		drawColorTX = fixInt(c[0], 0, colorTX.length - 1, defDrawColorTX);
	
	}
	else {
		drawColorRX = defDrawColorRX;
		drawColorTX = defDrawColorTX;
	}
	
	*/    // Banned by Viz 2010.09   ^^^^^^^^^^^
	
	drawColorRX = defDrawColorRX;
	drawColorTX = defDrawColorTX;		
	showColor();

	scaleMode = fixInt(cookie.get(cprefix + 'scale'), 0, 1, 0);  //cprefix = 'bw_r';
	showScale();

	avgMode = fixInt(cookie.get(cprefix + 'avg'), 1, 10, defAvg);
	showAvg();

	// if just switched
	if ((nvram.wan_proto == 'disabled') || (nvram.wan_proto == 'wet')) {
		nvram.wan_ifname = '';
	}

	htmReady = 1;
	initData();

}
