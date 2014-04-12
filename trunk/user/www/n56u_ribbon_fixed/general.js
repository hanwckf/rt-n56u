var keyPressed;
var wItem;
var final_flag = 0;	// for validate_ipaddr() always return true.
var change = 0;
var pageChanged = 0;

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
            return v1 * 256 * 256 * 256 + v2 * 256 * 256 + v3 * 256 + v4;
    }

    return -2;
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

        if (v4 == 255 || !(v4 == 0 || (is1to0(v4) && v1 == 255 && v2 == 255 && v3 == 255)))
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

function is1to0(num) {
    if (typeof(num) != "number")
        return 0;

    if (num == 255 || num == 254 || num == 252 || num == 248
        || num == 240 || num == 224 || num == 192 || num == 128)
        return 1;

    return 0;
}

function getSubnet(ip_str, mask_str, flag) {
    var ip_num, mask_num;
    var sub_head, sub_end;

    if (!ip_str || !mask_str)
        return -1;

    if (isMask(mask_str) <= 0)
        return -2;

    if (!flag || (flag != "head" && flag != "end"))
        flag = "head";

    ip_num = inet_network(ip_str);
    mask_num = inet_network(mask_str);

    if (ip_num < 0 || mask_num < 0)
        return -3;

    sub_head = ip_num - (ip_num & ~mask_num);
    sub_end = sub_head + ~mask_num;

    if (flag == "head")
        return sub_head;
    else
        return sub_end;
}

function changeDate() {
    pageChanged = 1;
    return true;
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
        else if (s == 'LWFilterList') {
            if (document.form.filter_lw_num_x_0.value >= c) cFlag = 1;
            else if (!validate_iprange(document.form.filter_lw_srcip_x_0, "") ||
                !validate_portrange(document.form.filter_lw_srcport_x_0, "") ||
                !validate_iprange(document.form.filter_lw_dstip_x_0, "") ||
                !validate_portrange(document.form.filter_lw_dstport_x_0, "")) return false;
            else if (document.form.filter_lw_srcip_x_0.value == "" &&
                document.form.filter_lw_srcport_x_0.value == "" &&
                document.form.filter_lw_dstip_x_0.value == "" &&
                document.form.filter_lw_dstport_x_0.value == "") bFlag = 1;

            for (var i = 0; i < LWFilterList.length; i++) { //validate if the entry is duplicated in list.
                if (document.form.filter_lw_srcip_x_0.value == LWFilterList[i][0] &&
                    document.form.filter_lw_srcport_x_0.value == LWFilterList[i][1] &&
                    document.form.filter_lw_dstip_x_0.value == LWFilterList[i][2] &&
                    document.form.filter_lw_dstport_x_0.value == LWFilterList[i][3] &&
                    document.form.filter_lw_proto_x_0.value == LWFilterList[i][4]) {
                    alert("<#JS_duplicate#>");
                    return false;
                }
            }
        }
        else if (s == 'UrlList') {
            if (document.form.url_num_x_0.value >= c)
                cFlag = 1;
            else if (document.form.url_keyword_x_0.value == "")
                bFlag = 1;
            else if (!validate_duplicate(document.form.UrlList_s, document.form.url_keyword_x_0.value, 32, 0))
                return false;
        }
        else if (s == 'KeywordList') {
            if (document.form.keyword_num_x_0.value >= c)
                cFlag = 1;
            else if (document.form.keyword_keyword_x_0.value == "")
                bFlag = 1;
            else if (!validate_duplicate(document.form.KeywordList_s, document.form.keyword_keyword_x_0.value, 32, 0))
                return false;
        }
    }

    if (bFlag == 1)
        alert("<#JS_fieldblank#>");
    else if (cFlag == 1)
        alert("<#JS_itemlimit1#> " + c + " <#JS_itemlimit2#>");
    else {    // b == " Del "
        if (s == 'LWFilterList') {
            updateDateTime("Advanced_Firewall_Content.asp");
        }
        else if (s == 'UrlList') {
            updateDateTime("Advanced_URLFilter_Content.asp");
        }

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
    for (i = 0; i < s.length; i++) {
        c = s.charAt(i);
        if ((c != ' ') && (c != '\n') && (c != '\t'))return false;
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

    var j;

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

    for (var i = 0; i < o.options.length; i++) {
        if (entry_cmp(o.options[i].text.substring(off).toLowerCase(), v.toLowerCase(), l) == 0) {
            return false;
        }
    }
    return true;
}

function validate_duplicate(o, v, l, off) {    // 2008.01 James.	
    for (var i = 0; i < o.options.length; i++) {
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

function is_hwaddr() {
    keyPressed = event.keyCode ? event.keyCode : event.which;
    if ((keyPressed > 47 && keyPressed < 58) || (keyPressed > 64 && keyPressed < 71) || (keyPressed > 96 && keyPressed < 103))
        return true;
    else if (keyPressed == 0)
        return true;
    else
        return false;
}

function validate_hwaddr(o) {
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

function is_string(o) {
    keyPressed = event.keyCode ? event.keyCode : event.which;
    if (keyPressed == 0)
        return true;
    else if (keyPressed >= 0 && keyPressed <= 126)
        return true;
    alert("<#JS_validchar#>");
    return false;
}

function is_string2(o) {
    keyPressed = event.keyCode ? event.keyCode : event.which;
    if (keyPressed == 0)
        return true;
    else if ((keyPressed >= 48 && keyPressed <= 57) ||
        (keyPressed >= 97 && keyPressed <= 122) ||
        (keyPressed >= 65 && keyPressed <= 90) ||
        (keyPressed == 45)
        )
        return true;
    alert("<#JS_validchar#>");
    return false;
}

function validate_ssidchar(ch) {
    if (ch >= 32 && ch <= 126)
        return true;

    return false;
}

function validate_string_ssid(o) {
    var c;

    for (var i = 0; i < o.value.length; ++i) {
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

function is_number(o) {
    keyPressed = event.keyCode ? event.keyCode : event.which;
    if (keyPressed == 0) return true;
    if (keyPressed > 47 && keyPressed < 58) {
        if (keyPressed == 48 && o.length == 0) return false;
        return true;
    }
    else {
        return false;
    }
}

function validate_range(o, min, max) {
    for (i = 0; i < o.value.length; i++) {
        if (o.value.charAt(i) < '0' || o.value.charAt(i) > '9') {
            alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
            //o.value = max;
            o.focus();
            o.select();
            return false;
        }
    }
    if (o.value < min || o.value > max) {
        alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
        o.value = "";
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

function is_ipaddr(o) {
    keyPressed = event.keyCode ? event.keyCode : event.which;

    if (keyPressed == 0) {
        return true;
    }

    if (o.value.length >= 16)
        return false;

    if ((keyPressed > 47 && keyPressed < 58)) {
        j = 0;

        for (i = 0; i < o.value.length; i++) {
            if (o.value.charAt(i) == '.') {
                j++;
            }
        }

        if (j < 3 && i >= 3) {
            if (o.value.charAt(i - 3) != '.' && o.value.charAt(i - 2) != '.' && o.value.charAt(i - 1) != '.') {
                o.value = o.value + '.';
            }
        }

        return true;
    }
    else if (keyPressed == 46) {
        j = 0;

        for (i = 0; i < o.value.length; i++) {
            if (o.value.charAt(i) == '.') {
                j++;
            }
        }

        if (o.value.charAt(i - 1) == '.' || j == 3) {
            return false;
        }

        return true;
    }
    else {
        return false;
    }

    return false;
}

function intoa(ip) {
    n = 0;
    vip = 0;
    for (i = 0; i < ip.length; i++) {
        c = ip.charAt(i);
        if (c == '.') {
            vip = vip * 256 + n;
            n = 0;
        }
        else if (c >= '0' && c <= '9') {
            n = n * 10 + (c - '0');
        }
    }
    vip = vip * 256 + n;
    return(vip);
}

function requireWANIP(v) {
    if (v == 'wan_ipaddr' || v == 'wan_netmask' ||
        v == 'lan_ipaddr' || v == 'lan_netmask') {
        if (wan_proto == "static")
            return 1;
        else if (wan_proto == "pppoe" && intoa(document.form.wan_ipaddr.value))
            return 1;
        else if ((wan_proto == "pptp" || wan_proto == "l2tp")
            && document.form.wan_ipaddr.value != '0.0.0.0')
            return 1;
        else
            return 0;
    }

    else return 0;
}

function matchSubnet2(ip1, sb1, ip2, sb2) {
    var nsb;
    var nsb1 = intoa(sb1);
    var nsb2 = intoa(sb2);

    if (nsb1 < nsb2)
        nsb = nsb1;
    else
        nsb = nsb2;

    if ((intoa(ip1) & nsb) == (intoa(ip2) & nsb))
        return 1;
    else
        return 0;
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

function validate_ipaddr(o, v) {
    if (final_flag)
        return true;

    if (o.value.length == 0) {
        if (v == 'dhcp_start' || v == 'dhcp_end' || v == 'wan_ipaddr' || v == 'lan_ipaddr' || v == 'lan_netmask') {
            alert("<#JS_fieldblank#>");
            if (v == 'wan_ipaddr') {
                document.form.wan_ipaddr.value = "10.1.1.1";
                document.form.wan_netmask.value = "255.0.0.0";
            }
            else if (v == 'lan_ipaddr') {
                document.form.lan_ipaddr.value = "192.168.1.1";
                document.form.lan_netmask.value = "255.255.255.0";
            }
            else if (v == 'lan_netmask') document.form.lan_netmask.value = "255.255.255.0";
            else if (v == 'dhcp_start') document.form.dhcp_start.value = document.form.dhcp_end.value;
            else if (v == 'dhcp_end') document.form.dhcp_end.value = document.form.dhcp_start.value;
            o.focus();
            o.select();
            return false;
        }
        else return true;
    }

    if (v == 'wan_ipaddr' && document.form.wan_netmask.value == "")
        document.form.wan_netmask.value = "255.255.255.0";

    var ip4 = parse_ipv4_addr(o.value);
    if (ip4 == null){
        alert(o.value + " <#JS_validip#>");
        o.focus();
        o.select();
        return false;
    }

    if (v == 'dhcp_start' || v == 'dhcp_end' || v == 'wan_ipaddr' || v == 'lan_ipaddr' || v == 'staticip') {

        if (v != 'wan_ipaddr' && (ip4[0] == 255 || ip4[3] == 255 || ip4[0] == 0 || ip4[3] == 0 || ip4[0] == 127 || ip4[0] == 224)) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
        if ((v == 'wan_ipaddr' && matchSubnet2(o.value, document.form.wan_netmask.value, document.form.lan_ipaddr.value, document.form.lan_netmask.value)) ||
            (v == 'lan_ipaddr' && matchSubnet2(o.value, document.form.lan_netmask.value, document.form.wan_ipaddr.value, document.form.wan_netmask.value))) {
            alert(o.value + " <#JS_validip#>");
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

    }
    else if (v == 'lan_netmask') {
        if (ip4[0] == 255 && ip4[1] == 255 && ip4[2] == 255 && ip4[3] == 255) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
    }
    if (requireWANIP(v) && (
            (v == 'wan_netmask' && matchSubnet2(document.form.wan_ipaddr.value, o.value, document.form.lan_ipaddr.value, document.form.lan_netmask.value)) ||
            (v == 'lan_netmask' && matchSubnet2(document.form.lan_ipaddr.value, o.value, document.form.wan_ipaddr.value, document.form.wan_netmask.value)))) {
        alert(o.value + " <#JS_validip#>");
        if (v == 'wan_netmask') {
            document.form.wan_ipaddr.value = "10.1.1.1";
            document.form.wan_netmask.value = "255.0.0.0";
        }
        else if (v == 'lan_netmask') {
            document.form.lan_ipaddr.value = "192.168.1.1";
            document.form.lan_netmask.value = "255.255.255.0";
        }
        o.focus();
        o.select();
        return false;
    }
    o.value = ip4[0] + "." + ip4[1] + "." + ip4[2] + "." + ip4[3];
    if ((ip4[0] > 0) && (ip4[0] < 127)) mask = "255.0.0.0";
    else if ((ip4[0] > 127) && (ip4[0] < 192)) mask = "255.255.0.0";
    else if ((ip4[0] > 191) && (ip4[0] < 224)) mask = "255.255.255.0";
    else mask = "0.0.0.0";
    if (v == 'wan_ipaddr' && document.form.wan_netmask.value == "") {
        document.form.wan_netmask.value = mask;
    }
    else if (v == 'lan_ipaddr' && document.form.lan_netmask.value == "") {
        document.form.lan_netmask.value = mask;
    }
    else if (v == 'dhcp_start') {
        if (!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3)) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
        if (intoa(o.value) > intoa(document.form.dhcp_end.value)) {
            tmp = document.form.dhcp_start.value;
            document.form.dhcp_start.value = document.form.dhcp_end.value;
            document.form.dhcp_end.value = tmp;
        }
    }
    else if (v == 'dhcp_end') {
        if (!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3)) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
        if (intoa(document.form.dhcp_start.value) > intoa(o.value)) {
            tmp = document.form.dhcp_start.value;
            document.form.dhcp_start.value = document.form.dhcp_end.value;
            document.form.dhcp_end.value = tmp;
        }
    }
    return true;
}

function validate_ipaddr_final(o, v) {
    if (o.value.length == 0) {
        if (v == 'dhcp_start' || v == 'dhcp_end' ||
            v == 'wan_ipaddr' ||
            v == 'lan_ipaddr' || v == 'lan_netmask' ||
            v == 'wl_radius_ipaddr' || v == 'rt_radius_ipaddr') {
            alert("<#JS_fieldblank#>");

            if (v == 'wan_ipaddr') {
                document.form.wan_ipaddr.value = "10.1.1.1";
                document.form.wan_netmask.value = "255.0.0.0";
            }
            else if (v == 'lan_ipaddr') {
                document.form.lan_ipaddr.value = "192.168.1.1";
                document.form.lan_netmask.value = "255.255.255.0";
            }
            else if (v == 'lan_netmask')
                document.form.lan_netmask.value = "255.255.255.0";

            o.focus();
            o.select();

            return false;
        }
        else
            return true;
    }

    if (v == 'wan_ipaddr' && document.form.wan_netmask.value == "")
        document.form.wan_netmask.value = "255.255.255.0";

    var ip4 = parse_ipv4_addr(o.value);
    if (ip4 == null){
        alert(o.value + " <#JS_validip#>");
        o.focus();
        o.select();
        return false;
    }

    if (v == 'dhcp_start' || v == 'dhcp_end' ||
        v == 'wan_ipaddr' || v == 'wan_dns1_x' ||
        v == 'lan_ipaddr' ||
        v == 'staticip' || v == 'wl_radius_ipaddr' || v == 'rt_radius_ipaddr' ||
        v == 'dhcp_dns1_x' || v == 'dhcp_gateway_x' || v == 'dhcp_wins_x') {
        if ((v != 'wan_ipaddr') && (ip4[0] == 255 || ip4[3] == 255 || ip4[0] == 0 || ip4[3] == 0 || ip4[0] == 127 || ip4[0] == 224)) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }

        if ((wan_route_x == "IP_Bridged" && wan_nat_x == "0") || sw_mode == "2" || sw_mode == "3")    // variables are defined in state.js
            ;    // there is no WAN in AP mode, so it wouldn't be compared with the wan ip..., etc.
        else if (requireWANIP(v) && (
            (v == 'wan_ipaddr' && matchSubnet2(o.value, document.form.wan_netmask.value, document.form.lan_ipaddr.value, document.form.lan_netmask.value)) ||
            (v =='wan_gateway' && matchSubnet2(o.value, document.form.wan_netmask.value, document.form.lan_ipaddr.value, document.form.lan_netmask.value)) ||
            (v == 'lan_ipaddr' && matchSubnet2(o.value, document.form.lan_netmask.value, document.form.wan_ipaddr.value, document.form.wan_netmask.value)))) {
            alert("WAN and LAN should have different IP addresses and subnet.");
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
    }
    else if (v == 'lan_netmask') {
        if (ip4[0] == 255 && ip4[1] == 255 && ip4[2] == 255 && ip4[3] == 255) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
    }

    if ((wan_route_x == "IP_Bridged" && wan_nat_x == "0") || sw_mode == "2" || sw_mode == "3")    // variables are defined in state.js
        ;    // there is no WAN in AP mode, so it wouldn't be compared with the wan ip..., etc.
    else if (requireWANIP(v) && (
            (v == 'lan_netmask' && matchSubnet2(document.form.lan_ipaddr.value, o.value, document.form.wan_ipaddr.value, document.form.wan_netmask.value)))) {
        alert(o.value + " <#JS_validip#>");

        if (v == 'wan_netmask') {
            document.form.wan_ipaddr.value = "10.1.1.1";
            document.form.wan_netmask.value = "255.0.0.0";
        }
        else if (v == 'lan_netmask') {
            document.form.lan_ipaddr.value = "192.168.1.1";
            document.form.lan_netmask.value = "255.255.255.0";
        }
        o.focus();
        o.select();
        return false;
    }

    o.value = ip4[0] + "." + ip4[1] + "." + ip4[2] + "." + ip4[3];

    if ((ip4[0] > 0) && (ip4[0] < 127)) mask = "255.0.0.0";
    else if ((ip4[0] > 127) && (ip4[0] < 192)) mask = "255.255.0.0";
    else if ((ip4[0] > 191) && (ip4[0] < 224)) mask = "255.255.255.0";
    else mask = "0.0.0.0";

    if (v == 'wan_ipaddr' && document.form.wan_netmask.value == "") {
        document.form.wan_netmask.value = mask;
    }
    else if (v == 'lan_ipaddr' && document.form.lan_netmask.value == "") {
        document.form.lan_netmask.value = mask;
    }
    else if (v == 'dhcp_start') {
        if (!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_start.value, 3)) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
    }
    else if (v == 'dhcp_end') {
        if (!matchSubnet(document.form.lan_ipaddr.value, document.form.dhcp_end.value, 3)) {
            alert(o.value + " <#JS_validip#>");
            o.focus();
            o.select();
            return false;
        }
    }

    return true;
}

function change_ipaddrport(o) {
}

function is_ipaddrport(o) {
    keyPressed = event.keyCode ? event.keyCode : event.which;
    if (keyPressed == 0) {
        return true;
    }
    if ((keyPressed > 47 && keyPressed < 58) || keyPressed == 46 || keyPressed == 58) {
        return true;
    }
    return false;
}

function validate_ipaddrport(o, v) {
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
    if (v == 'ExternalIPAddress' && document.form.wan_netmask.value == '') {
        document.form.wan_netmask.value = "255.255.255.0";
    }
    else if (v == 'IPRouters' && document.form.lan_netmask.value == '') {
        document.form.lan_netmask.value = "255.255.255.0";
    }
    return true;
}
function change_iprange(o) {
}
function is_iprange(o) {
    keyPressed = event.keyCode ? event.keyCode : event.which;
    if (keyPressed == 0) {
        return true;
    }
    if (o.value.length >= 15) return false;
    if ((keyPressed > 47 && keyPressed < 58)) {
        j = 0;
        for (i = 0; i < o.value.length; i++) {
            if (o.value.charAt(i) == '.') {
                j++;
            }
        }
        if (j < 3 && i >= 3) {
            if (o.value.charAt(i - 3) != '.' && o.value.charAt(i - 2) != '.' && o.value.charAt(i - 1) != '.')
                o.value = o.value + '.';
        }
        return true;
    }
    else if (keyPressed == 46) {
        j = 0;
        for (i = 0; i < o.value.length; i++) {
            if (o.value.charAt(i) == '.') {
                j++;
            }
        }
        if (o.value.charAt(i - 1) == '.' || j == 3) {
            return false;
        }
        return true;
    }
    else if (keyPressed == 42) /* '*' */
    {
        return true;
    }
    else {
        return false;
    }
    return false;
}
function validate_iprange(o, v) {
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
    if (v == 'ExternalIPAddress' && document.form.wan_netmask.value == '') {
        document.form.wan_netmask.value = "255.255.255.0";
    }
    else if (v == 'IPRouters' && document.form.lan_netmask.value == '') {
        document.form.lan_netmask.value = "255.255.255.0";
    }
    return true;
}
function is_portrange(o) {
    keyPressed = event.keyCode ? event.keyCode : event.which;
    if (keyPressed == 0) return true;
    //if (o.value.length>11) return false;  //limit the input length;

    if ((keyPressed > 47 && keyPressed < 58)) {
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
    else if (keyPressed == 44) {  // can be type in first charAt  ::: 0220 Lock add
        if (o.value.length == 0)
            return false;
        else
            return true;
    }
    else if (keyPressed == 60 || keyPressed == 62) {  //">" and "<" only can be type in first charAt ::: 0220 Lock add
        if (o.value.length == 0)
            return true;
        else
            return false;
    }
    else {
        return false;
    }
}

function validate_portrange(o, v) {
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
                //o.value = "";
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
            //o.value = "";
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
            if (document.form.current_page.value == "Advanced_VirtualServer_Content.asp") {  //2008.10.09 Lock add Allow input "," in Virtual Server page
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
    }// correct port		
    return true;
}

function is_portlist(o) {
    keyPressed = event.keyCode ? event.keyCode : event.which;
    if (keyPressed == 0) return true;
    if (o.value.length > 36) return false;
    if ((keyPressed > 47 && keyPressed < 58) || keyPressed == 32) {
        return true;
    }
    else {
        return false;
    }
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
    if (document.form.current_page.value == "Advanced_BasicFirewall_Content.asp") {
        change_firewall(rcheck(document.form.fw_enable_x));
    }
    else if (document.form.current_page.value == "Advanced_Firewall_Content.asp") {
        wItem = new Array(
            new Array("WWW", "80", "TCP"),
            new Array("TELNET", "23", "TCP"),
            new Array("FTP", "20:21", "TCP")
        );
        free_options(document.form.LWKnownApps);
        add_option(document.form.LWKnownApps, "User Defined", "User Defined", 1);
        for (i = 0; i < wItem.length; i++) {
            add_option(document.form.LWKnownApps, wItem[i][0], wItem[i][0], 0);
        }
        document.form.filter_lw_date_x_Sun.checked = getDateCheck(document.form.filter_lw_date_x.value, 0);
        document.form.filter_lw_date_x_Mon.checked = getDateCheck(document.form.filter_lw_date_x.value, 1);
        document.form.filter_lw_date_x_Tue.checked = getDateCheck(document.form.filter_lw_date_x.value, 2);
        document.form.filter_lw_date_x_Wed.checked = getDateCheck(document.form.filter_lw_date_x.value, 3);
        document.form.filter_lw_date_x_Thu.checked = getDateCheck(document.form.filter_lw_date_x.value, 4);
        document.form.filter_lw_date_x_Fri.checked = getDateCheck(document.form.filter_lw_date_x.value, 5);
        document.form.filter_lw_date_x_Sat.checked = getDateCheck(document.form.filter_lw_date_x.value, 6);
        document.form.filter_lw_time_x_starthour.value = getTimeRange(document.form.filter_lw_time_x.value, 0);
        document.form.filter_lw_time_x_startmin.value = getTimeRange(document.form.filter_lw_time_x.value, 1);
        document.form.filter_lw_time_x_endhour.value = getTimeRange(document.form.filter_lw_time_x.value, 2);
        document.form.filter_lw_time_x_endmin.value = getTimeRange(document.form.filter_lw_time_x.value, 3);
        document.form.filter_lw_time_x_1_starthour.value = getTimeRange(document.form.filter_lw_time_x_1.value, 0);	//Viz add 2011.11
        document.form.filter_lw_time_x_1_startmin.value = getTimeRange(document.form.filter_lw_time_x_1.value, 1);
        document.form.filter_lw_time_x_1_endhour.value = getTimeRange(document.form.filter_lw_time_x_1.value, 2);
        document.form.filter_lw_time_x_1_endmin.value = getTimeRange(document.form.filter_lw_time_x_1.value, 3);
    }
    else if (document.form.current_page.value == "Advanced_LFirewall_Content.asp") {
        document.form.FirewallConfig_WanLocalActiveDate_Sun.checked = getDateCheck(document.form.FirewallConfig_WanLocalActiveDate.value, 0);
        document.form.FirewallConfig_WanLocalActiveDate_Mon.checked = getDateCheck(document.form.FirewallConfig_WanLocalActiveDate.value, 1);
        document.form.FirewallConfig_WanLocalActiveDate_Tue.checked = getDateCheck(document.form.FirewallConfig_WanLocalActiveDate.value, 2);
        document.form.FirewallConfig_WanLocalActiveDate_Wed.checked = getDateCheck(document.form.FirewallConfig_WanLocalActiveDate.value, 3);
        document.form.FirewallConfig_WanLocalActiveDate_Thu.checked = getDateCheck(document.form.FirewallConfig_WanLocalActiveDate.value, 4);
        document.form.FirewallConfig_WanLocalActiveDate_Fri.checked = getDateCheck(document.form.FirewallConfig_WanLocalActiveDate.value, 5);
        document.form.FirewallConfig_WanLocalActiveDate_Sat.checked = getDateCheck(document.form.FirewallConfig_WanLocalActiveDate.value, 6);
        document.form.FirewallConfig_WanLocalActiveTime_starthour.value = getTimeRange(document.form.FirewallConfig_WanLocalActiveTime.value, 0);
        document.form.FirewallConfig_WanLocalActiveTime_startmin.value = getTimeRange(document.form.FirewallConfig_WanLocalActiveTime.value, 1);
        document.form.FirewallConfig_WanLocalActiveTime_endhour.value = getTimeRange(document.form.FirewallConfig_WanLocalActiveTime.value, 2);
        document.form.FirewallConfig_WanLocalActiveTime_endmin.value = getTimeRange(document.form.FirewallConfig_WanLocalActiveTime.value, 3);
    }
    else if (document.form.current_page.value == "Advanced_URLFilter_Content.asp") {
        document.form.url_date_x_Sun.checked = getDateCheck(document.form.url_date_x.value, 0);
        document.form.url_date_x_Mon.checked = getDateCheck(document.form.url_date_x.value, 1);
        document.form.url_date_x_Tue.checked = getDateCheck(document.form.url_date_x.value, 2);
        document.form.url_date_x_Wed.checked = getDateCheck(document.form.url_date_x.value, 3);
        document.form.url_date_x_Thu.checked = getDateCheck(document.form.url_date_x.value, 4);
        document.form.url_date_x_Fri.checked = getDateCheck(document.form.url_date_x.value, 5);
        document.form.url_date_x_Sat.checked = getDateCheck(document.form.url_date_x.value, 6);
        document.form.url_time_x_starthour.value = getTimeRange(document.form.url_time_x.value, 0);
        document.form.url_time_x_startmin.value = getTimeRange(document.form.url_time_x.value, 1);
        document.form.url_time_x_endhour.value = getTimeRange(document.form.url_time_x.value, 2);
        document.form.url_time_x_endmin.value = getTimeRange(document.form.url_time_x.value, 3);
        document.form.url_time_x_starthour_1.value = getTimeRange(document.form.url_time_x_1.value, 0);
        document.form.url_time_x_startmin_1.value = getTimeRange(document.form.url_time_x_1.value, 1);
        document.form.url_time_x_endhour_1.value = getTimeRange(document.form.url_time_x_1.value, 2);
        document.form.url_time_x_endmin_1.value = getTimeRange(document.form.url_time_x_1.value, 3);
    }
    else if (document.form.current_page.value == "Advanced_DHCP_Content.asp" ||
        document.form.current_page.value == "Advanced_RDHCP_Content.asp") {
        final_flag = 1;
    }
    else if (document.form.current_page.value == "Advanced_DDNS_Content.asp") {
    }
    else if (document.form.current_page.value == "Main_GStatus_Content.asp") {
    }
    else if (document.form.current_page.value == "Advanced_QOSUserSpec_Content.asp") {
        if (document.form.qos_dfragment_enable_w.checked == true) {
            inputCtrl(document.form.qos_dfragment_size, 1);
        }
        else {
            inputCtrl(document.form.qos_dfragment_size, 0);
        }
    }
    change = 0;
}

function change_firewall(r) {
    if (r == "0") {
        inputCtrl(document.form.misc_httpport_x, 0);
        inputCtrl(document.form.sshd_wport, 0);

        inputRCtrl1(document.form.misc_http_x, 0);
        inputRCtrl2(document.form.misc_http_x, 1);
        inputRCtrl1(document.form.misc_ping_x, 0);
        inputRCtrl2(document.form.misc_ping_x, 1);
        inputRCtrl1(document.form.https_wopen, 0);
        inputRCtrl2(document.form.https_wopen, 1);
        inputRCtrl1(document.form.sshd_wopen, 0);
        inputRCtrl2(document.form.sshd_wopen, 1);
        inputRCtrl1(document.form.ftpd_wopen, 0);
        inputRCtrl2(document.form.ftpd_wopen, 1);
        inputRCtrl1(document.form.trmd_ropen, 0);
        inputRCtrl2(document.form.trmd_ropen, 1);
        $("row_misc_ping").style.display = "none";
        $("access_section").style.display = "none";
    }
    else {
        inputCtrl(document.form.misc_httpport_x, 1);
        inputCtrl(document.form.https_wport, 1);
        inputCtrl(document.form.sshd_wport, 1);

        inputRCtrl1(document.form.misc_http_x, 1);
        inputRCtrl1(document.form.misc_ping_x, 1);
        inputRCtrl1(document.form.https_wopen, 1);
        inputRCtrl1(document.form.sshd_wopen, 1);
        inputRCtrl1(document.form.ftpd_wopen, 1);
        inputRCtrl1(document.form.trmd_ropen, 1);
        $("row_misc_ping").style.display = "";
        $("access_section").style.display = "";
    }
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

function onSubmitCtrlOnly(o, s) {
    if (s != 'Upload' && s != 'Upload1')
        document.form.action_mode.value = s;

    if (s == 'Upload1') {
        disableCheckChangedStatus();
        document.form.submit();
    }
    stopFlag = 1;
    disableCheckChangedStatus();
    return true;
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
            if (document.form.current_page.value == "Advanced_ASUSDDNS_Content.asp")
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

    if (document.form.current_page.value == "Advanced_ASUSDDNS_Content.asp") {
        if (s == "hostname_check") {
            if (document.form.DDNSName.value == "" || !validate_ddns_hostname(document.form.DDNSName)) {
                document.form.DDNSName.focus();
                document.form.DDNSName.select();
                return false;
            }
            document.form.ddns_hostname_x.value = document.form.DDNSName.value+".asuscomm.com";
            showLoading();
        }
        document.form.action_mode.value = "Update";
        document.form.action_script.value = s;
    } else {
        document.form.action_mode.value = "Update";
        document.form.action_script.value = s;
    }

    return true;
}

function setup_script(s) {
    if (document.form.current_page.value == "Advanced_ACL_Content.asp") {
        document.form.action_script.value = s;
    }
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
    else if (v == "ddns_server_x") {
        change_ddns_setting(o.value);
    }

    return true;
}

function change_ddns_setting(v) {
    if (v == "WWW.ASUS.COM") {
        inputCtrl(document.form.ddns_username_x, 0);
        inputCtrl(document.form.ddns_passwd_x, 0);
    } else {
        inputCtrl(document.form.ddns_username_x, 1);
        inputCtrl(document.form.ddns_passwd_x, 1);
    }

    if (v == "WWW.ASUS.COM") {
        showhide("ddnsname_input", 0);
        showhide("asusddnsname_input", 1);
        document.getElementById("ddnsname2_row").style.display = "none";
        document.getElementById("ddnsname3_row").style.display = "none";
        document.form.ddns_wildcard_x[0].disabled = 1;
        document.form.ddns_wildcard_x[1].disabled = 1;
        document.form.LANHostConfig_x_DDNSHostnameCheck_button.disabled = 0;
        showhide("link", 0);
    } else {
        showhide("ddnsname_input", 1);
        showhide("asusddnsname_input", 0);
        document.getElementById("ddnsname2_row").style.display = "";
        document.getElementById("ddnsname3_row").style.display = "";
        document.form.ddns_wildcard_x[0].disabled = 0;
        document.form.ddns_wildcard_x[1].disabled = 0;
        document.form.LANHostConfig_x_DDNSHostnameCheck_button.disabled = 1;
        showhide("link", 1);
    }
}

function change_common_radio(o, s, v, r) {
    change = 1;
    pageChanged = 1;
    if (v == "qos_dfragment_enable") {
        if (r == '1') {
            inputCtrl(document.form.qos_dfragment_size, 1);
        }
        else {
            inputCtrl(document.form.qos_dfragment_size, 0);
        }
    }
    else if (v == "fw_enable_x") {
        change_firewall(r);
    }
    else if (s == "PrinterStatus" && v == "usb_webhttpport_x") {
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

function matchSubnet(ip1, ip2, count) {
    var c = 0;
    var v1 = 0;
    var v2 = 0;
    for (i = 0; i < ip1.length; i++) {
        if (ip1.charAt(i) == '.') {
            if (ip2.charAt(i) != '.')    return false;
            c++;
            if (v1 != v2) return false;
            v1 = 0;
            v2 = 0;
        } else {
            if (ip2.charAt(i) == '.') return false;
            v1 = v1 * 10 + (ip1.charAt(i) - '0');
            v2 = v2 * 10 + (ip2.charAt(i) - '0');
        }

        if (c == count) return true;
    }
    return false;
}

function subnetPrefix(ip, orig, count) {
    r = '';
    c = 0;
    for (i = 0; i < ip.length; i++) {
        if (ip.charAt(i) == '.')    c++;
        if (c == count) break;
        r = r + ip.charAt(i);
    }
    c = 0;
    for (i = 0; i < orig.length; i++) {
        if (orig.charAt(i) == '.')    c++;
        if (c >= count)    r = r + orig.charAt(i);
    }
    return (r);
}

function subnetPostfix(ip, num, count) {
    r = '';
    orig = "";
    c = 0;
    for (i = 0; i < ip.length; i++) {
        if (ip.charAt(i) == '.')    c++;
        r = r + ip.charAt(i);
        if (c == count) break;
    }
    c = 0;
    orig = String(num);
    for (i = 0; i < orig.length; i++) {
        r = r + orig.charAt(i);
    }
    return (r);
}

function wan_netmask_check(o) {
    var ip = intoa(document.form.wan_ipaddr.value);
    var gw = intoa(document.form.wan_gateway.value);
    var nm = intoa(document.form.wan_netmask.value);
    var lip = intoa(document.form.lan_ipaddr.value);
    var lnm = intoa(document.form.lan_netmask.value);

    if (document.form.wan_ipaddr.value != '0.0.0.0' && (ip & lnm) == (lip & lnm))
    {
        alert(o.value + " <#JS_validip#>");
        document.form.wan_ipaddr.value = "10.1.1.1";
        document.form.wan_netmask.value = "255.0.0.0";
        o.focus();
        o.select();
        return false;
    }
    if (gw == 0 || gw == 0xffffffff || (ip & nm) == (gw & nm)) {
        return true;
    }
    else {
        alert(o.value + " <#JS_validip#>");
        o.focus();
        o.select();
        return false;
    }
}

function updateDateTime(s) {
    if (s == "Advanced_Firewall_Content.asp") {
        document.form.filter_lw_date_x.value = setDateCheck(
            document.form.filter_lw_date_x_Sun,
            document.form.filter_lw_date_x_Mon,
            document.form.filter_lw_date_x_Tue,
            document.form.filter_lw_date_x_Wed,
            document.form.filter_lw_date_x_Thu,
            document.form.filter_lw_date_x_Fri,
            document.form.filter_lw_date_x_Sat);
        document.form.filter_lw_time_x.value = setTimeRange(
            document.form.filter_lw_time_x_starthour,
            document.form.filter_lw_time_x_startmin,
            document.form.filter_lw_time_x_endhour,
            document.form.filter_lw_time_x_endmin);
        document.form.filter_lw_time_x_1.value = setTimeRange(
            document.form.filter_lw_time_x_1_starthour,
            document.form.filter_lw_time_x_1_startmin,
            document.form.filter_lw_time_x_1_endhour,
            document.form.filter_lw_time_x_1_endmin);
    }
    else if (s == "Advanced_URLFilter_Content.asp") {
        document.form.url_date_x.value = setDateCheck(
            document.form.url_date_x_Sun,
            document.form.url_date_x_Mon,
            document.form.url_date_x_Tue,
            document.form.url_date_x_Wed,
            document.form.url_date_x_Thu,
            document.form.url_date_x_Fri,
            document.form.url_date_x_Sat);
        document.form.url_time_x.value = setTimeRange(
            document.form.url_time_x_starthour,
            document.form.url_time_x_startmin,
            document.form.url_time_x_endhour,
            document.form.url_time_x_endmin);
        document.form.url_time_x_1.value = setTimeRange(
            document.form.url_time_x_starthour_1,
            document.form.url_time_x_startmin_1,
            document.form.url_time_x_endhour_1,
            document.form.url_time_x_endmin_1);
    }
}

function openLink(s) {
    var link_params = "toolbar=yes,location=yes,directories=no,status=yes,menubar=yes,scrollbars=yes,resizable=yes,copyhistory=no,width=640,height=480";
    if (s == 'x_DDNSServer') {
        if (document.form.ddns_server_x.value.indexOf("WWW.DYNDNS.ORG") != -1)
            tourl = "https://www.dyndns.com/account/create.html";
        else if (document.form.ddns_server_x.value == 'WWW.ZONEEDIT.COM')
            tourl = "http://www.zoneedit.com/signUp.html";
        else if (document.form.ddns_server_x.value == 'WWW.TZO.COM')
            tourl = "http://signup.tzo.com";
        else if (document.form.ddns_server_x.value == 'WWW.EASYDNS.COM')
            tourl = "https://web.easydns.com/Open_Account/";
        else if (document.form.ddns_server_x.value == 'WWW.NO-IP.COM')
            tourl = "http://www.noip.com/newUser.php";
        else if (document.form.ddns_server_x.value == 'WWW.TUNNELBROKER.NET')
            tourl = "http://www.tunnelbroker.net/register.php";
        else if (document.form.ddns_server_x.value == 'DNS.HE.NET')
            tourl = "http://ipv6.he.net/certification/register.php";
        else if (document.form.ddns_server_x.value == 'WWW.DNSEXIT.COM')
            tourl = "https://www.dnsexit.com/Direct.sv?cmd=signup";
        else if (document.form.ddns_server_x.value == 'WWW.CHANGEIP.COM')
            tourl = "https://www.changeip.com/signup.asp?";
        else if (document.form.ddns_server_x.value == 'FREEDNS.AFRAID.ORG')
            tourl = "http://freedns.afraid.org/signup";
        else
            return;
        link = window.open(tourl, "DDNSLink", link_params);
    }
    else if (s == 'x_NTPServer1') {
        tourl = "http://support.ntp.org/bin/view/Servers/WebHome"
        link = window.open(tourl, "NTPLink", link_params);
    }
    else if (s == 'x_FIsAnonymous' || s == 'x_FIsSuperuser') {
        urlstr = location.href;
        url = urlstr.indexOf("http://");
        port = document.form.usb_ftpport_x.value;
        if (url == -1) urlpref = LANIP;
        else {
            urlstr = urlstr.substring(7, urlstr.length);
            url = urlstr.indexOf(":");
            if (url != -1) {
                urlpref = urlstr.substring(0, url);
            }
            else {
                url = urlstr.indexOf("/");
                if (url != -1) urlpref = urlstr.substring(0, url);
                else urlpref = urlstr;
            }
        }
        if (s == 'x_FIsAnonymous') {
            user = 'anonymous';
            tourl = "ftp://" + urlpref;
        }
        else {
            user = 'admin';
            tourl = "ftp://" + user + "@" + urlpref;
        }
        if (port != 21) tourl = tourl + ":" + port;
        link = window.open(tourl, "FTPServer", link_params);
    }
    if (!link.opener) link.opener = self;
}

function blur_body() {
    alert("<#JS_focus#>");
}

function showhide(element, sh) {
    var status;
    if (sh == 1) {
        status = "block";
    }
    else {
        status = "none"
    }

    if (document.getElementById) {
        document.getElementById(element).style.display = status;
    }
    else if (document.all) {
        document.all[element].style.display = status;
    }
    else if (document.layers) {
        document.layers[element].display = status;
    }
}

