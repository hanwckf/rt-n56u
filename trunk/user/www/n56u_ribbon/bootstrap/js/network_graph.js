/**
 * Created with JetBrains PhpStorm.
 * User: eagle23
 * Date: 03.07.12
 * Time: 13:04
 * To change this template use File | Settings | File Templates.
 */

var week = ['Sun','Mon','Tue','Wed','Thu','Fri','Sat'];
var pointLoc = 0;
var vWidth = 1000;
var vHeight = 300;
var crossH = 0;
var crossV = 0;
var maxV = 0;
var maxL = 0;
var time24 = 0;
var advX = 1;
var interval = 0;
var hours = 0;
var xpstVis = 1;
var ignoreNext = 0;
var eraseL = 0;
var samp = 1;

function E(id)
{
    return document.getElementById(id);
    //return $j('#' + id);
}

function offsetX(ev)
{
    return $j.browser.mozilla ? (ev.layerX - document.getElementById("chart").offsetLeft) : ev.offsetX;
}

function offsetY(ev)
{
    return $j.browser.mozilla ? (ev.layerY - document.getElementById("chart").offsetTop) : ev.offsetY;
}

function pad(n)
{
    n = n.toString();
    while (n.length < 2) n = '0' + n;
    return n;
}

function xps(n)
{
    n /= 1024;
    return n.toFixed(2) + ' KB/s';
}

function setText(e, text)
{
    // Adobe/IE doesn't do textContent=
    if (e.firstChild) e.removeChild(e.firstChild);
    e.appendChild(document.createTextNode(text));
}

function init(ev)
{
    var n;
    var obj = E('svgraph');
    vWidth = $j("#chart").width() - ($j.browser.msie ? 12 : 0);

    obj.addEventListener("mousemove", mMove, false);
    obj.addEventListener("mousedown", mClick, false);


//  if (typeof(svgDocument) == 'undefined') svgDocument = ev.srcElement.ownerDocument;

    crossX = E('crossX');
    crossY = E('crossY');
    polyRx = E('polyRx');
    polyTx = E('polyTx');
    pointTextBack = E('pointTextBack');
    pointText = E('pointText');
    pointG = E('pointGroup');
    crossTextBack = E('crossTextBack');
    crossText = E('crossText');
    crossTime = E('crossTime');
    background = E('background');
    hori = E('hori');

    tickLines = [];
    for (n = 0; n < 24; ++n) {
        tickLines[n] = E('tick' + n);
    }
    tickHours = [];
    for (n = 0; n < 12; ++n) {
        tickHours[n] = E('h' + n);
    }

    xpst = [];
    for (n = 0; n < 4; ++n)
        xpst[n] = E('xpst' + n);

    time24 = ((new Date(2000, 0, 1, 23, 0, 0, 0)).toLocaleString().indexOf('23') != -1);

    top.updateSVG = updateSVG;
    top.svgReady = 1;
    top.initData();
}

function drawData(poly, data, mode, color)
{
    var i;
    var pt;
    var x, y;
    var d, j;

    if (data.length == 0) return;

    x = 0;
    if (mode == 0) {
        poly.setAttribute('fill-opacity', '0.5');
        pt = '0,' + vHeight;
    }
    else {
        poly.setAttribute('fill-opacity', '0');
        pt = '';
    }
    poly.setAttribute('stroke', color);
    poly.setAttribute('fill', color);

    for (i = data.length - maxL; i < data.length; ++i) {
        if (i < 0) {
            d = 0;
        }
        else if (i >= samp) {
            d = 0;
            for (j = samp - 1; j >= 0; --j) {
                d += data[i - j];
            }
            d /= (dataDiv * samp);
        }
        else {
            d = data[i] / dataDiv;
        }
        y = (vHeight - Math.floor((d * vHeight) / maxV))
        pt += ' ' + Math.floor(x) + ',' + y;
        x += advX;
    }
    pt += ' ' + (vWidth + 5) + ',' + y + ' ' + (vWidth + 10) + ',' + vHeight;
    poly.setAttribute('points', pt);
}

function updateSVG(rxData, txData, maxValue, mode, rxColor, txColor, intv, maxLen, dataD, avgSamp, clock)
{
    var x, y, z, i;
    var v, e;

    maxV = maxValue;
    interval = intv;
    maxL = maxLen;
    advX = vWidth / maxL;
    dataDiv = dataD;
    samp = avgSamp;

    tockD = clock;
//	tockD = new Date();
    tock = tockD.getTime();

    if (intv < 60) {
        // realtime
        x = z = (vWidth / 5);
        for (i = 0; i < 4; ++i) {
            tickLines[i].setAttribute('x1', Math.round(x));
            tickLines[i].setAttribute('x2', Math.round(x));
            x += z;
        }
    }
    else {
        advM = (60 / interval) * advX;
        x = (60 - tockD.getMinutes()) * advM;
        v = advM * 60;
        for (i = 0; i < 24; ++i) {
            z = Math.round(x);
            y = (new Date(tock - (Math.round((vWidth - z - 1) / advX) * interval * 1000))).getHours();
            x += v;

            e = tickLines[i];
            e.setAttribute('x1', z);
            e.setAttribute('x2', z);
            e.setAttribute('class', (y & 1) ? 'a' : 'b');

            if ((y & 1) == 0) {
                e = tickHours[i >> 1];
                e.setAttribute('x', z);
                z = y % 24;
                if (!time24) {
                    if (z < 12) {
                        if (z == 0) z = 12;
                        z += ' am';
                    }
                    else {
                        z -= 12;
                        if (z == 0) z = 12;
                        z += ' pm';
                    }
                }
                setText(e, z);
            }
        }
    }

    if (maxV <= 0) {
        polyRx.setAttribute('points', '');
        polyTx.setAttribute('points', '');
        return;
    }

    if (crossV > 0) drawCross(crossH, vHeight - Math.floor((crossV / maxV) * vHeight));
    drawData(polyRx, rxData, mode, rxColor);
    drawData(polyTx, txData, mode, txColor);

    setText(xpst[0], xps(maxV * 0.70));
    setText(xpst[1], xps(maxV * 0.5));
    setText(xpst[2], xps(maxV * 0.25));
    setText(xpst[3], xps(maxV));
    if (eraseL > 0) {
        if (--eraseL == 0) pointG.setAttribute('visibility', 'visible');
    }
}

function vY(y)
{
    return maxV - (maxV * (y / vHeight))
}

function pointTime(x)
{
    var t, hh, h, s;

    t = new Date(tock - (Math.round((vWidth - x - 1) / advX) * interval * 1000));
    h = t.getHours();
    s = week[t.getDay()] + ' ';
    if (time24)	{
        s += pad(h) + ':' + pad(t.getMinutes());
    }
    else {
        hh = h % 12;
        s += pad((hh == 0) ? 12 : hh) + ':' + pad(t.getMinutes()) + ((h < 12) ? ' am' : ' pm');
    }

    return s;
}

function setXY(e, x, y)
{
    e.setAttribute('x', x);
    e.setAttribute('y', y);
}

function drawCross(x, y)
{
    var n;

    crossX.setAttribute('x1', x - 5);
    crossX.setAttribute('x2', x + 5);
    crossX.setAttribute('y1', y);
    crossX.setAttribute('y2', y);

    crossY.setAttribute('x1', x);
    crossY.setAttribute('x2', x);
    crossY.setAttribute('y1', y - 5);
    crossY.setAttribute('y2', y + 5);

//	n = Math.max(crossText.getComputedTextLength(), crossTime.getComputedTextLength()) + 20;
    n = 164;

    crossTextBack.setAttribute('width', n);

    if (x > (vWidth - n - 10)) {
        crossText.setAttribute('style', 'text-anchor:end');
        crossTime.setAttribute('style', 'text-anchor:end');
        crossTextBack.setAttribute('x', (x - n) - 10);
        x -= 20;
    }
    else {
        crossText.setAttribute('style', '');
        crossTime.setAttribute('style', '');
        crossTextBack.setAttribute('x', x + 10);
        x += 20;
    }

    crossTextBack.setAttribute('y', y - 17);
    setXY(crossTime, x, y - 5);
    setXY(crossText, x, y + 10);
}

function mMove(ev)
{
    var x;

    if (maxV <= 0) return;
    if (offsetX(ev) > (vWidth - 120)) {
        if (pointLoc == 0) {
            if (offsetY(ev) < 30) {
                pointLoc = 1;
                pointText.setAttribute('y', '98%');
                pointTextBack.setAttribute('y', '283');
            }
        }
        else {
            if (offsetY(ev) > (vHeight - 30)) {
                pointLoc = 0;
                pointText.setAttribute('y', '5%');
                pointTextBack.setAttribute('y', '1%');
            }
        }
    }

    setText(pointText, pointTime(offsetX(ev)) + ' / ' + xps(vY(offsetY(ev))));

    x = 230;
    pointTextBack.setAttribute('x', (vWidth - x) - 22);
    pointTextBack.setAttribute('width', x + 20);
    pointText.setAttribute('style', 'text-anchor:end');
    pointG.setAttribute('visibility', 'visible');
    eraseL = 5;
}

function mClick(ev)
{
    if (ignoreNext) {
        ignoreNext = 0;
        return;
    }
    if (maxV <= 0) return;

    crossH = offsetX(ev);
    crossV = vY(offsetY(ev));
    setText(crossText, xps(crossV));
    setText(crossTime, pointTime(crossH));
    drawCross(offsetX(ev), offsetY(ev));
}

