var keyPressed;
var wItem;
var change = 0;
var pageChanged = 0;

function changeDate() {
    pageChanged = 1;
    return true;
}

function inet_network(ip_str) {
    if (!ip_str)
        return -1;

    var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
    if (re.test(ip_str)) {
        var v1 = parseInt(RegExp.$1);
        var v2 = parseInt(RegExp.$2);
        var v3 = parseInt(RegExp.$3);
        var v4 = parseInt(RegExp.$4);
        if (v1 < 256 && v2 < 256 && v3 < 256 && v4 < 256)
            return v1*256*256*256+v2*256*256+v3*256+v4;
    }

    return -2;
}

function is1to0(num) {
    if (typeof(num) != "number")
        return 0;

    if (num == 255 || num == 254 || num == 252 || num == 248 ||
        num == 240 || num == 224 || num == 192 || num == 128)
        return 1;

    return 0;
}

function isMask(ip_str) {
    if (!ip_str)
        return 0;

    var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
    if (re.test(ip_str)) {
        var v1 = parseInt(RegExp.$1);
        var v2 = parseInt(RegExp.$2);
        var v3 = parseInt(RegExp.$3);
        var v4 = parseInt(RegExp.$4);
        if (!(v4 == 0 || (is1to0(v4) && v1 == 255 && v2 == 255 && v3 == 255)))
            return -4;
        if (!(v3 == 0 || (is1to0(v3) && v1 == 255 && v2 == 255)))
            return -3;
        if (!(v2 == 0 || (is1to0(v2) && v1 == 255)))
            return -2;
        if (!is1to0(v1))
            return -1;
    }

    return 1;
}

function get_subnet_num(addr_str,mask_str,is_end) {
    var addr_num, mask_num, ret_val;

    if (!addr_str || !mask_str)
        return -1;

    if (isMask(mask_str) <= 0)
        return -2;

    addr_num = inet_network(addr_str);
    mask_num = inet_network(mask_str);
    if (addr_num < 0 || mask_num < 0)
        return -3;

    ret_val = addr_num - (addr_num & ~mask_num);
    if (is_end)
        ret_val += ~mask_num;

    return ret_val;
}

function num2ip4(addr) {
    var addr_num = Number(addr);
    var v1 = (addr_num>>24)&255;
    var v2 = (addr_num>>16)&255;
    var v3 = (addr_num>>8)&255;
    var v4 = (addr_num&255);
    return v1.toString()+"."+v2.toString()+"."+v3.toString()+"."+v4.toString();
}

function str2val(v) {
    for (i = 0; i < v.length; i++) {
        if (v.charAt(i) != '0') break;
    }
    return v.substring(i);
}

function inputRCtrl1(o, flag) {
    if (flag == 0) {
        o[0].disabled = 1;
        o[1].disabled = 1;
    }
    else {
        o[0].disabled = 0;
        o[1].disabled = 0;
    }
}

function inputRCtrl2(o, flag) {
    if (flag == 0) {
        o[0].checked = true;
        o[1].checked = false;
    }
    else {
        o[0].checked = false;
        o[1].checked = true;
    }
}

function checkPass(o, o1, o2) {
    if (o1.value == o2.value) {
        document.form.action_mode.value = "  Save  ";
        return true;
    }
    alert("<#JS_checkpass#>");
    return false;
}

function spoiler_toggle(element){
    if (document.getElementById(element).style.display == "none") {
        document.getElementById(element).style.display = "";
    } else {
        document.getElementById(element).style.display = "none";
    }
}

function markGroup(o, s, c, b) {
    var bFlag, cFlag;

    document.form.group_id.value = s;
    bFlag = 0; //Judge the input field is blank or not. 1:blank;
    cFlag = 0; //Judge the input item number is overload or not.

    if (b == " Add ") {
        if (s == 'RBRList') {
            if (document.form.wl_wdsnum_x_0.value >= c)
                cFlag = 1;
            else if (!validate_hwaddr(document.form.wl_wdslist_x_0))
                return false;
            else if (document.form.wl_wdslist_x_0.value == "")
                bFlag = 1;
            else if (!validate_duplicate(document.form.RBRList_s, document.form.wl_wdslist_x_0.value, 12, 0))
                return false;
        }
        else if (s == 'rt_RBRList') {
            if (document.form.rt_wdsnum_x_0.value >= c)
                cFlag = 1;
            else if (!validate_hwaddr(document.form.rt_wdslist_x_0))
                return false;
            else if (document.form.rt_wdslist_x_0.value == "")
                bFlag = 1;
            else if (!validate_duplicate(document.form.rt_RBRList_s, document.form.rt_wdslist_x_0.value, 12, 0))
                return false;
        }
        else if (s == 'MFList') {
            if (document.form.macfilter_num_x_0.value >= c) cFlag = 1;
            else if (!validate_hwaddr(document.form.macfilter_list_x_0)) return false;
            else if (document.form.macfilter_list_x_0.value == "") bFlag = 1;
            else if (!validate_duplicate(document.form.MFList_s, document.form.macfilter_list_x_0.value, 12, 0)) return false;
        }
        else if (s == 'ACLList') {
            if (document.form.wl_macnum_x_0.value >= c)
                cFlag = 1;
            else if (!validate_hwaddr(document.form.wl_maclist_x_0))
                return false;
            else if (document.form.wl_maclist_x_0.value == "")
                bFlag = 1;
            else if (!validate_duplicate(document.form.ACLList_s, document.form.wl_maclist_x_0.value, 12, 0))
                return false;
        }
        else if (s == 'rt_ACLList') {
            if (document.form.rt_macnum_x_0.value >= c)
                cFlag = 1;
            else if (!validate_hwaddr(document.form.rt_maclist_x_0))
                return false;
            else if (document.form.rt_maclist_x_0.value == "")
                bFlag = 1;
            else if (!validate_duplicate(document.form.rt_ACLList_s, document.form.rt_maclist_x_0.value, 12, 0))
                return false;
        }
        else if (s == 'UrlList') {
            if (document.form.url_num_x_0.value >= c)
                cFlag = 1;
            else if (document.form.url_keyword_x_0.value == "")
                bFlag = 1;
            else if (!validate_duplicate(document.form.UrlList_s, document.form.url_keyword_x_0.value, 32, 0))
                return false;
        }
    }

    if (bFlag == 1)
        alert("<#JS_fieldblank#>");
    else if (cFlag == 1)
        alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
    else {    // b == " Del "
        pageChanged = 0;

        document.form.action_mode.value = b;
        return true;
    }
    return false;
}

function portrange_min(o, v) {
    var num = 0;
    var common_index = o.substring(0, v).indexOf(':');

    if (common_index == -1)
        num = parseInt(o.substring(0, v));
    else
        num = parseInt(o.substring(0, common_index));

    return num;
}

function portrange_max(o, v) {
    var num = 0;
    var common_index = o.substring(0, v).indexOf(':');

    if (common_index == -1)
        num = parseInt(o.substring(0, v));
    else
        num = parseInt(o.substring(common_index + 1, v + 1));

    return num;
}

function isBlank(s) {
    var i,c;
    for (i = 0; i < s.length; i++) {
        c = s.charAt(i);
        if ((c != ' ') && (c != '\n') && (c != '\t'))
            return false;
    }
    return true;
}

function numbersonly() {
    if (keyPressed > 47 && keyPressed < 58)
        return true;
    else
        return false;
}

function check_ptl() {
    if (keyPressed == 38)
        return false;
    else
        return true;
}

function entry_cmp(entry, match, len) {  //compare string length function
    var j,c1,c2;
    if (entry.length < match.length)
        return (1);

    for (j = 0; j < entry.length && j < len; j++) {
        c1 = entry.charCodeAt(j);
        if (j >= match.length)
            c2 = 160;
        else
            c2 = match.charCodeAt(j);

        if (c1 == 160)
            c1 = 32;

        if (c2 == 160)
            c2 = 32;

        if (c1 > c2)
            return (1);
        else if (c1 < c2)
            return(-1);
    }
    return 0;
}

function validate_duplicate_noalert(o, v, l, off) {
    var i;
    for (i = 0; i < o.options.length; i++) {
        if (entry_cmp(o.options[i].text.substring(off).toLowerCase(), v.toLowerCase(), l) == 0)
            return false;
    }
    return true;
}

function validate_duplicate(o, v, l, off) {
    var i;
    for (i = 0; i < o.options.length; i++) {
        if (entry_cmp(o.options[i].text.substring(off).toLowerCase(), v.toLowerCase(), l) == 0) {
            alert("<#JS_duplicate#>");

            return false;
        }
    }
    return true;
}

function validate_duplicate2(o, v, l, off) {
    var i;
    for (i = 0; i < o.options.length; i++) {
        if (entry_cmp(o.options[i].text.substring(off).toLowerCase(), v.toLowerCase(), l) == 0) {
            return false;
        }
    }
    return true;
}

function is_control_key(e){
    if(e.which === 0){
        if (e.keyCode == 35 //End
         || e.keyCode == 36 //Home
         || e.keyCode == 37 //<-
         || e.keyCode == 39 //->
         || e.keyCode == 45 //Insert
         || e.keyCode == 46 //Del
            )
            return true;
    }
    if (e.keyCode == 8  //Backspace
     || e.keyCode == 9  //Tab
     || e.keyCode == 27 //Esc
        )
        return true;
    return false;
}

function is_hwaddr(e) {
    e = e || event;
    if (is_control_key(e))
        return true;
    keyPressed = e.keyCode ? e.keyCode : e.which;
    if (keyPressed == 0)
        return true;
    if ((keyPressed > 47 && keyPressed < 58) ||
        (keyPressed > 64 && keyPressed < 71) ||
        (keyPressed > 96 && keyPressed < 103))
        return true;
    return false;
}

function validate_hwaddr(o) {
    var i,c;
    if (o.value.length == 0) return true;
    if (o.value != "") {
        if (o.value.length == 12) {
            for (i = 0; i < o.value.length; i++) {
                c = o.value.charAt(i);
                if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F')) {
                    alert("<#JS_validmac#>");
                    o.value = "";
                    o.focus();
                    o.select();
                    return false;
                }
            }
            return true;
        }
    }
    alert("<#JS_validmac#>");
    o.value = "";
    o.focus();
    o.select();
    return false;
}

function is_string(o,e) {
    e = e || event;
    if (is_control_key(e))
        return true;
    keyPressed = e.keyCode ? e.keyCode : e.which;
    if (keyPressed == 0)
        return true;
    if (keyPressed > 0 && keyPressed <= 126)
        return true;
    alert("<#JS_validchar#>");
    return false;
}

function is_string2(o,e) {
    e = e || event;
    if (is_control_key(e))
        return true;
    keyPressed = e.keyCode ? e.keyCode : e.which;
    if (keyPressed == 0)
        return true;
    if ((keyPressed >= 48 && keyPressed <= 57) ||
        (keyPressed >= 97 && keyPressed <= 122) ||
        (keyPressed >= 65 && keyPressed <= 90) ||
        (keyPressed == 45))
        return true;
    alert("<#JS_validchar#>");
    return false;
}

function validate_ssidchar(ch) {
    if (ch >= 32 && ch <= 126)
        return true;
    if (ch >= 0x4e00 && ch <= 0x9fa5)
        return true;
    return false;
}

function validate_string_ssid(o) {
    var i,c;
    for (i = 0; i < o.value.length; ++i) {
        c = o.value.charCodeAt(i);
        if (!validate_ssidchar(c)) {
            alert("<#JS_validSSID1#> " + o.value.charAt(i) + " <#JS_validSSID2#>");
            o.value = "";
            o.focus();
            o.select();
            return false;
        }
    }
    return true;
}

function is_number(o,e) {
    e = e || event;
    if (is_control_key(e))
        return true;
    keyPressed = e.keyCode ? e.keyCode : e.which;
    if (keyPressed == 0)
        return true;
    if (keyPressed > 47 && keyPressed < 58) {
        if (keyPressed == 48 && o.length == 0)
            return false;
        return true;
    }
    return false;
}

function validate_range(o, min, max) {
    var i;
    for (i = 0; i < o.value.length; i++) {
        if (o.value.charAt(i) < '0' || o.value.charAt(i) > '9') {
            alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
            o.focus();
            o.select();
            return false;
        }
    }
    if (o.value < min || o.value > max) {
        alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
        o.focus();
        o.select();
        return false;
    }
    else {
        o.value = str2val(o.value);
        if (o.value == "")
            o.value = "0";
        return true;
    }
}

function validate_range_hex(o, min, max) {
    var o_val = parseInt("0x"+o.value);
    if (o_val < min || o_val > max) {
        alert('<#JS_validrange#> ' + min.toString(16) + ' <#JS_validrange_to#> ' + max.toString(16));
        o.focus();
        o.select();
        return false;
    }
    return true;
}

function validate_range_sp(o, min, max) {
    if (o.value.length == 0) return true;

    if (o.value < min || o.value > max) {
        alert('<#JS_validrange#> ' + min + ' to ' + max + '.');
        o.value = min;
        o.focus();
        o.select();
        return false;
    }
    else {
        o.value = str2val(o.value);
        if (o.value == "") o.value = "0";
        return true;
    }
}

function decimalToHex(d, padding) {
  var hex = Number(d).toString(16);
  while (hex.length < padding)
    hex = "0" + hex;
  return hex;
}

function change_ipaddr(o) {
}

function is_ipaddr(o,e) {
    var i,j;
    e = e || event;
    if (is_control_key(e))
        return true;
    keyPressed = e.keyCode ? e.keyCode : e.which;
    if (keyPressed == 0)
        return true;
    if (o.value.length >= 16)
        return false;
    if(keyPressed > 47 && keyPressed < 58){
        j = 0;
        for (i = 0; i < o.value.length; i++){
            if (o.value.charAt(i) == '.')
                j++;
        }
        if (j < 3 && i >= 3) {
            if (o.value.charAt(i - 3) != '.' && o.value.charAt(i - 2) != '.' && o.value.charAt(i - 1) != '.')
                o.value = o.value + '.';
        }
        return true;
    }
    else if(keyPressed == 46){
        j = 0;
        for (i = 0; i < o.value.length; i++){
            if (o.value.charAt(i) == '.')
                j++;
        }
        if (o.value.charAt(i - 1) == '.' || j == 3)
            return false;
        return true;
    }
    return false;
}

function matchSubnet(ip1, ip2, sb1) {
    var nsb = inet_network(sb1);

    if ((inet_network(ip1) & nsb) == (inet_network(ip2) & nsb))
        return true;
    return false;
}

function matchSubnet2(ip1, sb1, ip2, sb2) {
    var nsb;
    var nsb1 = inet_network(sb1);
    var nsb2 = inet_network(sb2);

    if (nsb1 < nsb2)
        nsb = nsb1;
    else
        nsb = nsb2;

    if ((inet_network(ip1) & nsb) == (inet_network(ip2) & nsb))
        return true;
    return false;
}

function parse_ipv4_addr(ip_str) {
    var ip4 = new Array(4);

    var re = /^(\d+)\.(\d+)\.(\d+)\.(\d+)$/;
    if (re.test(ip_str)) {
        ip4[0] = parseInt(RegExp.$1);
        ip4[1] = parseInt(RegExp.$2);
        ip4[2] = parseInt(RegExp.$3);
        ip4[3] = parseInt(RegExp.$4);
        if (ip4[0] < 256 && ip4[1] < 256 && ip4[2] < 256 && ip4[3] < 256)
            return ip4;
    }

    return null;
}

function validate_ipaddr_final(o, v) {
    if (o.value.length == 0) {
        if (v == 'dhcp_start' || v == 'dhcp_end' || v == 'wan_ipaddr' || v == 'lan_ipaddr') {
            alert("<#JS_fieldblank#>");
            if (v == 'wan_ipaddr') {
                document.form.wan_ipaddr.value = "10.1.1.1";
                document.form.wan_netmask.value = "255.0.0.0";
            }
            else if (v == 'lan_ipaddr') {
                document.form.lan_ipaddr.value = "192.168.1.1";
                document.form.lan_netmask.value = "255.255.255.0";
            }
            o.focus();
            o.select();
            return false;
        }
        else
            return true;
    }

    if (v == 'lan_ipaddr' && document.form.lan_netmask.value == "")
        document.form.lan_netmask.value = "255.255.255.0";
    else if (v == 'wan_ipaddr' && document.form.wan_netmask.value == "")
        document.form.wan_netmask.value = "255.0.0.0";

    if (v == 'lan_netmask' || v == 'wan_netmask') {
        if (isMask(o.value) <= 0) {
            alert(o.value + " <#JS_validmask#>");
            o.focus();
            o.select();
            return false;
        }
    }

    var ip4 = parse_ipv4_addr(o.value);
    if (ip4 == null){
        alert(o.value + " <#JS_validip#>");
        o.focus();
        o.select();
        return false;
    }

    if (v == 'lan_netmask') {
        if (ip4[3] > 252) {
            alert(o.value + " <#JS_validmask#>");
            o.focus();
            o.select();
            return false;
        }
    }

    if (v == 'wan_ipaddr' || v == 'lan_ipaddr') {
        if (ip4[0] == 0 && ip4[1] == 0 && ip4[2] == 0 && ip4[3] == 0) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
    }

    if (v == 'dhcp_start' || v == 'dhcp_end' ||
        v == 'lan_ipaddr' || v == 'staticip' || v == 'dmz_ip' ||
        v == 'dhcp_dns_x' || v == 'dhcp_gateway_x' || v == 'dhcp_wins_x') {
        if (ip4[0] == 255 || ip4[0] == 0 || ip4[0] == 127 || ip4[0] == 224) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
    }

    if (v == 'radius_ipaddr') {
        if (ip4[0] == 255 || ip4[0] == 0 || ip4[0] == 224) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
    }

    o.value = ip4[0].toString()+"."+ip4[1].toString()+"."+ip4[2].toString()+"."+ip4[3].toString();

    return true;
}

function change_ipaddrport(o) {
}

function is_ipaddrport(o,e) {
    e = e || event;
    if (is_control_key(e))
        return true;
    keyPressed = e.keyCode ? e.keyCode : e.which;
    if (keyPressed == 0)
        return true;
    if ((keyPressed > 47 && keyPressed < 58) || keyPressed == 46 || keyPressed == 58)
        return true;
    return false;
}

function validate_ipaddrport(o,v) {
    num = -1;
    pos = 0;
    if (o.value.length == 0)
        return true;
    str = o.value;
    portidx = str.indexOf(":");
    if (portidx != -1) {
        port = str.substring(portidx + 1);
        len = portidx;
        if (port > 65535) {
            alert(port + " <#JS_validport#>");
            o.value = "";
            o.focus();
            o.select();
            return false;
        }
    }
    else {
        len = o.value.length;
    }
    for (i = 0; i < len; i++) {
        c = o.value.charAt(i);
        if (c >= '0' && c <= '9') {
            if (num == -1) {
                num = (c - '0');
            }
            else {
                num = num * 10 + (c - '0');
            }
        }
        else {
            if (num < 0 || num > 255 || c != '.') {
                alert(o.value + " <#JS_validip#>");
                o.value = "";
                o.focus();
                o.select();
                return false;
            }
            num = -1;
            pos++;
        }
    }
    if (pos != 3 || num < 0 || num > 255) {
        alert(o.value + " <#JS_validip#>");
        o.value = "";
        o.focus();
        o.select();
        return false;
    }
    return true;
}

function change_iprange(o) {
}

function is_iprange(o,e) {
    var ret = is_ipaddr(o,e);
    if (!ret && o.value.length < 16 && keyPressed == 42)
        return true;
    return ret;
}

function validate_iprange(o,v) {
    num = -1;
    pos = 0;
    if (o.value.length == 0)
        return true;
    for (i = 0; i < o.value.length; i++) {
        c = o.value.charAt(i);
        if (c >= '0' && c <= '9') {
            if (num == -1) {
                num = (c - '0');
            }
            else {
                num = num * 10 + (c - '0');
            }
        }
        else if (c == '*' && num == -1) {
            num = 0;
        }
        else {
            if (num < 0 || num > 255 || (c != '.')) {
                alert(o.value + " <#JS_validip#>");
                o.value = "";
                o.focus();
                o.select();
                return false;
            }
            num = -1;
            pos++;
        }
    }
    if (pos != 3 || num < 0 || num > 255) {
        alert(o.value + " <#JS_validip#>");
        o.value = "";
        o.focus();
        o.select();
        return false;
    }
    return true;
}

function is_portrange(o,e) {
    e = e || event;
    if (is_control_key(e))
        return true;
    keyPressed = e.keyCode ? e.keyCode : e.which;
    if (keyPressed == 0)
        return true;
    if ((keyPressed > 47 && keyPressed < 58)){
        return true;
    }
    else if (keyPressed == 58 && o.value.length > 0) {
        for (i = 0; i < o.value.length; i++) {
            c = o.value.charAt(i);
            if (c == ':' || c == '>' || c == '<' || c == '=')
                return false;
        }
        return true;
    }
    else if (keyPressed == 44) {
        if (o.value.length == 0)
            return false;
        else
            return true;
    }
    else if (keyPressed == 60 || keyPressed == 62) {
        if (o.value.length == 0)
            return true;
        else
            return false;
    }
    return false;
}

function validate_portrange(o,v) {
    if (o.value.length == 0)
        return true;

    prev = -1;
    num = -1;
    num_front = 0;
    for (var i = 0; i < o.value.length; i++) {
        c = o.value.charAt(i);
        if (c >= '0' && c <= '9') {
            if (num == -1) num = 0;
            num = num * 10 + (c - '0');
        }
        else {
            if (num > 65535 || num == 0 || (c != ':' && c != '>' && c != '<' && c != ',')) {
                alert(o.value + " <#JS_validport#>");
                o.focus();
                o.select();
                return false;
            }

            if (c == '>') prev = -2;
            else if (c == '<') prev = -3;
            else if (c == ',') {
                prev = -4;
                num = 0;
            }
            else { //when c=":"
                if (prev == -4)
                    prev == -4;
                else {
                    prev = num;
                    num = 0;
                }
            }
        }
    }

    if ((num > 65535 && prev != -3) || (num < 1 && prev != -2) || (prev > num) || (num >= 65535 && prev == -2) || (num <= 1 && prev == -3)) {
        if (num > 65535) {
            alert(o.value + " <#JS_validport#>");
            o.focus();
            o.select();
            return false;
        }
        else {
            alert(o.value + " <#JS_validportrange#>");
            o.focus();
            o.select();
            return false;
        }
    } // wrong port 
    else {
        if (prev == -2) {
            if (num == 65535) o.value = num;
            else o.value = (num + 1) + ":65535";  //ex. o.value=">2000", it will change to 2001:65535
        }
        else if (prev == -3) {
            if (num == 1) o.value = num;
            else o.value = "1:" + (num - 1);     //ex. o.value="<2000", it will change to 1:1999
        }
        else if (prev == -4) {
            if (document.form.current_page.value == "Advanced_VirtualServer_Content.asp") {
                multi_vts_port = o.value.split(",");
                //o.value = multi_vts_port[0];
                split_vts_rule(multi_vts_port);
                return false;
            }
            else {
                alert(o.value + " <#JS_validport#>");
                o.focus();
                o.select();
                return false;
            }
        }
        else if (prev != -1)
            o.value = prev + ":" + num;
        else
            o.value = num;                  //single port number case;
    }
    return true;
}

function is_portlist(o,e) {
    e = e || event;
    if (is_control_key(e))
        return true;
    keyPressed = e.keyCode ? e.keyCode : e.which;
    if (keyPressed == 0)
        return true;
    if (o.value.length > 36)
        return false;
    if ((keyPressed > 47 && keyPressed < 58) || keyPressed == 32) {
        return true;
    }
    return false;
}

function validate_portlist(o, v) {
    if (o.value.length == 0)
        return true;
    num = 0;
    for (i = 0; i < o.value.length; i++) {
        c = o.value.charAt(i);
        if (c >= '0' && c <= '9') {
            num = num * 10 + (c - '0');
        }
        else {
            if (num > 255) {
                alert(num + " <#JS_validport#>");
                o.value = "";
                o.focus();
                o.select();
                return false;
            }
            num = 0;
        }
    }
    if (num > 255) {
        alert(num + " <#JS_validport#>");
        o.value = "";
        o.focus();
        o.select();
        return false;
    }
    return true;
}

function add_option_match(o, s, f) {
    tail = o.options.length;
    o.options[tail] = new Option(s);
    o.options[tail].value = s;
    if (f == s) {
        o.options[tail].selected = 1;
        return(1);
    }
    else return(0);
}

function add_option_match_x(o, s, f) {
    tail = o.options.length;
    o.options[tail] = new Option(s);
    o.options[tail].value = tail;
    if (tail == f) {
        o.options[tail].selected = 1;
        return(1);
    }
    else return(0);
}

function find_option(o) {
    count = o.options.length;
    for (i = 0; i < count; i++) {
        if (o.options[i].selected)
            return(o.options[i].value);
    }
    return(null);
}

function add_options(o, arr, orig) {
    for (var i = 0; i < arr.length; i++) {
        if (orig == arr[i])
            add_option(o, arr[i], arr[i], 1);
        else
            add_option(o, arr[i], arr[i], 0);
    }
}

function add_options_x(o, arr, orig) {
    for (var i = 0; i < arr.length; i++) {
        if (orig == i)
            add_option(o, i, arr[i], arr[i], 1);
        else
            add_option(o, i, arr[i], arr[i], 0);
    }
}

function add_options_x2(o, arr, varr, orig) {
    free_options(o);

    for (var i = 0; i < arr.length; ++i) {
        if (orig == varr[i])
            add_option(o, arr[i], varr[i], 1);
        else
            add_option(o, arr[i], varr[i], 0);
    }
}

function add_a_option(o, i, s) {
    tail = o.options.length;
    o.options[tail] = new Option(s);
    o.options[tail].value = i;
}

function rcheck(o) {
    if (o[0].checked == true)
        return("1");
    else
        return("0");
}

function getDateCheck(str, pos) {
    if (str.charAt(pos) == '1')
        return true;
    else
        return false;
}

function getTimeRange(str, pos) {
    if (pos == 0)
        return str.substring(0, 2);
    else if (pos == 1)
        return str.substring(2, 4);
    else if (pos == 2)
        return str.substring(4, 6);
    else if (pos == 3)
        return str.substring(6, 8);
}

function setDateCheck(d1, d2, d3, d4, d5, d6, d7) {
    str = "";
    if (d7.checked == true) str = "1" + str;
    else str = "0" + str;
    if (d6.checked == true) str = "1" + str;
    else str = "0" + str;
    if (d5.checked == true) str = "1" + str;
    else str = "0" + str;
    if (d4.checked == true) str = "1" + str;
    else str = "0" + str;
    if (d3.checked == true) str = "1" + str;
    else str = "0" + str;
    if (d2.checked == true) str = "1" + str;
    else str = "0" + str;
    if (d1.checked == true) str = "1" + str;
    else str = "0" + str;
    return str;
}

function setTimeRange(sh, sm, eh, em) {
    return(sh.value + sm.value + eh.value + em.value);
}

function load_body() {
    document.form.next_host.value = location.host;
    change = 0;
}

function onSubmit() {
    change = 0;
    pageChanged = 0;

    return true;
}

function onSubmitCtrl(o, s) {
    document.form.action_mode.value = s;
    return (onSubmit());
}

function validate_ddns_hostname(o) {
    dot = 0;
    s = o.value;

    var unvalid_start = new RegExp("^[0-9].*", "gi");
    if (unvalid_start.test(s))
    {
        alert("<#LANHostConfig_x_DDNS_alarm_7#>");
        return false;
    }
    if (!validate_string(o)) {
        return false;
    }
    for (i = 0; i < s.length; i++) {
        c = s.charCodeAt(i);
        if (c == 46) {
            dot++;
            if (dot > 0) {
                alert("<#LANHostConfig_x_DDNS_alarm_7#>");
                return false;
            }
        }
        if (!validate_hostnamechar(c)) {
            if (document.form.current_page.value == "Advanced_DDNS_Content.asp")
                document.form.ddns_hostname_x.value = "";
            alert("<#LANHostConfig_x_DDNS_alarm_13#> '" + s.charAt(i) + "' !");
            return(false);
        }
    }
    return(true);
}

function validate_hostnamechar(ch) {
    if (ch >= 48 && ch <= 57) return true;
    if (ch >= 97 && ch <= 122) return true;
    if (ch >= 65 && ch <= 90) return true;
    if (ch == 45) return true;
    if (ch == 46) return true;
    return false;
}

function onSubmitApply(s) {
    pageChanged = 0;

    if (document.form.current_page.value == "Advanced_DDNS_Content.asp") {
        if (s == "hostname_check") {
            if (document.form.DDNSName.value == "" || !validate_ddns_hostname(document.form.DDNSName)) {
                document.form.DDNSName.focus();
                document.form.DDNSName.select();
                return false;
            }
            document.form.ddns_hostname_x.value = document.form.DDNSName.value+".asuscomm.com";
        }
        showLoading();
    }

    document.form.action_mode.value = "Update";
    document.form.action_script.value = s;

    return true;
}

function change_common(o, s, v) {
    change = 1;
    pageChanged = 1;

    if (s == "FirewallConfig" && v == "WanLanDefaultAct") {
        if (o.value == "DROP")
            alert("<#JS_WanLanAlert#>");
    }
    else if (s == "FirewallConfig" && v == "LanWanDefaultAct") {
        if (o.value == "DROP")
            alert("<#JS_LanWanAlert#>");
    }

    return true;
}

function change_common_radio(o, s, v, r) {
    change = 1;
    pageChanged = 1;
    if (s == "PrinterStatus" && v == "usb_webhttpport_x") {
        if (document.form.usb_webhttpport_x_check.checked) {
            document.form.usb_webhttpcheck_x.value = "1";
        }
        else {
            document.form.usb_webhttpcheck_x.value = "0";
        }
    }
    else if (v == "sw_mode") {
        if (r == '1') {
            document.form.sw_mode.value = "1";
        } else {
            document.form.sw_mode.value = "4";
        }
    }
    else if (s == "PPPConnection" && v == "wan_pppoe_idletime") {
        if (document.form.wan_pppoe_idletime_check.checked) {
            document.form.wan_pppoe_txonly_x.value = "1";
        }
        else {
            document.form.wan_pppoe_txonly_x.value = "0";
        }
    }
    else if (s == "PPPConnection" && v == "x_IdleTime1") {
        if (document.form.PPPConnection_x_IdleTime1_check.checked) {
            document.form.PPPConnection_x_IdleTxOnly1.value = "1";
        }
        else {
            document.form.PPPConnection_x_IdleTxOnly1.value = "0";
        }
    }
    else if (s == "PPPConnection" && v == "x_MultiPPPoEEnable1") {
        if (document.form.PPPConnection_x_MultiPPPoEEnable1[0].checked == true) {
            flag = 1;
        }
        else {
            flag = 0;
        }
        inputCtrl(document.form.PPPConnection_x_UserName1, flag);
        inputCtrl(document.form.PPPConnection_x_Password1, flag);
        inputCtrl(document.form.PPPConnection_x_IdleTime1, flag);
        inputCtrl(document.form.PPPConnection_x_IdleTime1_check, flag);
        inputCtrl(document.form.PPPConnection_x_PPPoEMTU1, flag);
        inputCtrl(document.form.PPPConnection_x_PPPoEMRU1, flag);
        inputCtrl(document.form.PPPConnection_x_ServiceName1, flag);
        inputCtrl(document.form.PPPConnection_x_AccessConcentrator1, flag);
    }
    else if (s == "PPPConnection" && v == "x_IdleTime2") {
        if (document.form.PPPConnection_x_IdleTime2_check.checked) {
            document.form.PPPConnection_x_IdleTxOnly2.value = "1";
        }
        else {
            document.form.PPPConnection_x_IdleTxOnly2.value = "0";
        }
    }
    else if (s == "PPPConnection" && v == "x_MultiPPPoEEnable2") {
        if (document.form.PPPConnection_x_MultiPPPoEEnable2[0].checked == true) {
            flag = 1;
        }
        else {
            flag = 0;
        }
        inputCtrl(document.form.PPPConnection_x_UserName2, flag);
        inputCtrl(document.form.PPPConnection_x_Password2, flag);
        inputCtrl(document.form.PPPConnection_x_IdleTime2, flag);
        inputCtrl(document.form.PPPConnection_x_IdleTime2_check, flag);
        inputCtrl(document.form.PPPConnection_x_PPPoEMTU2, flag);
        inputCtrl(document.form.PPPConnection_x_PPPoEMRU2, flag);
        inputCtrl(document.form.PPPConnection_x_ServiceName2, flag);
        inputCtrl(document.form.PPPConnection_x_AccessConcentrator2, flag);
    }
    return true;
}

function valid_WPAPSK(o) {
    if (o.value.length >= 64) {
        o.value = o.value.substring(0, 63);
        alert("<#JS_wpapass#>");
        return false;
    }

    return true;
}

function encryptionType(authType, wepType) {
    pflag = "1";
    if (authType.value == "1") {
        if (wepType.value == "0") wepLen = "64";
        else wepLen = "128";
    }
    else if (authType.value == "2") {
        wepLen = "0";
    }
    else if (authType.value == "3") {
        wepLen = "0";
        pflag = "0";
    }
    else if (authType.value == "4") {
        if (wepType.value == "0") wepLen = "64";
        else wepLen = "128";
    }
    else {
        if (wepType.value == "0") {
            wepLen = "0";
            pflag = "0";
        }
        else if (wepType.value == "1") wepLen = "64";
        else wepLen = "128";
    }
    return(pflag + wepLen);
}

function validate_timerange(o, p) {
    if (o.value.length == 0)
        o.value = "00";
    else if (o.value.length == 1)
        o.value = "0" + o.value;

    if (o.value.charAt(0) < '0' || o.value.charAt(0) > '9')
        o.value = "00";
    else if (o.value.charAt(1) < '0' || o.value.charAt(1) > '9')
        o.value = "00";
    else if (p == 0 || p == 2) {
        if (o.value > 23)
            o.value = "00";
    }
    else {
        if (o.value > 59)
            o.value = "00";
    }
    return true;
}

function updateDateTime(s) {
}

function blur_body() {
    alert("<#JS_focus#>");
}

function showhide(e, sh) {
    var status = (sh == 0) ? "none" : "block";
    if (document.getElementById){
        var o = document.getElementById(e);
        if (o !== null)
            o.style.display = status;
    }
    else if (document.all)
        document.all[e].style.display = status;
    else if (document.layers)
        document.layers[e].display = status;
}

function showhide_div(e, sh) {
    var status = (sh == 0) ? "none" : "";
    if (document.getElementById){
        var o = document.getElementById(e);
        if (o !== null)
            o.style.display = status;
    }
    else if (document.all)
        document.all[e].style.display = status;
    else if (document.layers)
        document.layers[e].display = status;
}
