var helpcontent = new Array(5);
var help_enable = '<% nvram_get_x("", "help_enable"); %>';

helpcontent[0] = new Array("",
				"Plus + 模式，过滤效果稍差,推荐全局模式</br>内网IP列表模式默认是不过滤所有IP的<br>请在内网控制选项里添加需要过滤的IP。",);
				
helpcontent[1] = new Array("",
				"1.作为dnsmasq的上游服务器(在AGH中统计到的ip都为127.0.0.1，无法统计客户端及对应调整设置)</br>2.重定向53端口到 AdGuardHome",);

helpcontent[2] = new Array("",
				"本工具是通过域名解析层来屏蔽广告和保护隐私的，其将各大著名的hosts，ad filter lists，adblock list等的列表进行合并去重，再进行一系列的抽象化，例如主动剔除失效域名、easylist优化模糊匹配、增强的黑白名单机制等措施，最终生成期望的高命中率列表。不建议和AD host同时打开。",);

function openTooltip(obj, hint_array_id, hint_show_id)
{
	if (help_enable == "0" && hint_show_id > 0)
		return;

	if(hint_array_id >= helpcontent.length)
		return;

	if(hint_show_id >= helpcontent[hint_array_id].length)
		return;

	$j(obj).attr('data-original-title', obj.innerHTML).attr('data-content', helpcontent[hint_array_id][hint_show_id]);
	$j(obj).popover('show');
}
