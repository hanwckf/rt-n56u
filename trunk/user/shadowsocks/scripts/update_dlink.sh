#!/bin/sh
#===================================================================
#Part of the code is extracted from https://github.com/hq450/fancyss
#chongshengB 20200228
#test......
#===================================================================

KEY_WORDS_1=`nvram get d_keyword_n`
KEY_WORDS_2=`nvram get d_keyword_y`
	
get_remote_config() {
	decode_link="$1"
	action="$2"
	server=$(echo "$decode_link" | awk -F':' '{print $1}')
	server_port=$(echo "$decode_link" | awk -F':' '{print $2}')
	protocol=$(echo "$decode_link" | awk -F':' '{print $3}')
	encrypt_method=$(echo "$decode_link" | awk -F':' '{print $4}')
	obfs=$(echo "$decode_link" | awk -F':' '{print $5}' | sed 's/_compatible//g')

	password=$(decode_url_link $(echo "$decode_link" | awk -F':' '{print $6}' | awk -F'/' '{print $1}'))
	password=$(echo $password)

	obfsparam_temp=$(echo "$decode_link" | awk -F':' '{print $6}' | grep -Eo "obfsparam.+" | sed 's/obfsparam=//g' | awk -F'&' '{print $1}')
	[ -n "$obfsparam_temp" ] && obfsparam=$(decode_url_link $obfsparam_temp) || obfsparam=''

	protoparam_temp=$(echo "$decode_link" | awk -F':' '{print $6}' | grep -Eo "protoparam.+" | sed 's/protoparam=//g' | awk -F'&' '{print $1}')
	[ -n "$protoparam_temp" ] && protoparam=$(decode_url_link $protoparam_temp | sed 's/_compatible//g') || protoparam=''

	remarks_temp=$(echo "$decode_link" | awk -F':' '{print $6}' | grep -Eo "remarks.+" | sed 's/remarks=//g' | awk -F'&' '{print $1}')
	if [ "$action" == "1" ]; then
		#订阅
		[ -n "$remarks_temp" ] && remarks=$(decode_url_link $remarks_temp) || remarks=""
	elif [ "$action" == "2" ]; then
		# ssr://添加
		[ -n "$remarks_temp" ] && remarks=$(decode_url_link $remarks_temp) || remarks='AutoSuB'
	fi

	group_temp=$(echo "$decode_link" | awk -F':' '{print $6}' | grep -Eo "group.+" | sed 's/group=//g' | awk -F'&' '{print $1}')
	if [ "$action" == "1" ]; then
		#订阅
		[ -n "$group_temp" ] && group=$(decode_url_link $group_temp) || group=""
	elif [ "$action" == "2" ]; then
		# ssr://添加
		[ -n "$group_temp" ] && group=$(decode_url_link $group_temp) || group='AutoSuBGroup'
	fi

	[ -n "$group" ] && group_base64=$(echo $group | sed 's/ -//g')
	[ -n "$server" ] && server_base64=$(echo $server | sed 's/ -//g')
	#把全部服务器节点写入文件 /usr/share/shadowsocks/serverconfig/all_onlineservers
	[ -n "$group" ] && [ -n "$server" ] && echo $server_base64 $group_base64 >>/tmp/all_onlineservers
	#echo ------
	#echo group: $group
	#echo remarks: $remarks
	#echo server: $server
	#echo server_port: $server_port
	#echo password: $password
	#echo encrypt_method: $encrypt_method
	#echo protocol: $protocol
	#echo protoparam: $protoparam
	#echo obfs: $obfs
	#echo obfsparam: $obfsparam
	#echo ------
	echo "$group" >>/tmp/all_group_info.txt
	[ -n "$group" ] && return 0 || return 1
}


start_link() {

	rm -rf /tmp/ssr_subscribe_file.txt >/dev/null 2>&1
	rm -rf /tmp/ssr_subscribe_file_temp1.txt >/dev/null 2>&1
	rm -rf /tmp/all_localservers >/dev/null 2>&1
	rm -rf /tmp/all_onlineservers >/dev/null 2>&1
	rm -rf /tmp/all_group_info.txt >/dev/null 2>&1
	rm -rf /tmp/group_info.txt >/dev/null 2>&1
	rm -rf /tmp/newdlink.txt >/dev/null 2>&1

	grep -v '^#' /etc/storage/ss_dlink.sh | grep -v "^$" >/tmp/dlist.txt
	for url in $(cat /tmp/dlist.txt); do
		[ -z "$url" ] && continue
		logger -t "SS" "从 $url 获取订阅..."
		addnum=0
		updatenum=0
		delnum=0
		get_oneline_rule_now "$url"

		case $? in
		0)
			continue
			;;
		2)
			logger -t "SS" "无法获取产品信息！请检查你的服务商是否更换了订阅链接！"
			rm -rf /tmp/ssr_subscribe_file.txt >/dev/null 2>&1 &
			let DEL_SUBSCRIBE+=1
			sleep 2
			echo "退出订阅程序..."
			;;
		3)
			logger -t "SS" "该订阅链接不包含任何节点信息！请检查你的服务商是否更换了订阅链接！"
			rm -rf /tmp/ssr_subscribe_file.txt >/dev/null 2>&1 &
			let DEL_SUBSCRIBE+=1
			sleep 2
			logger -t "SS" "退出订阅程序..."
			;;
		4)
			logger -t "SS" "订阅地址错误！检测到你输入的订阅地址并不是标准网址格式！"
			rm -rf /tmp/ssr_subscribe_file.txt >/dev/null 2>&1 &
			let DEL_SUBSCRIBE+=1
			sleep 2
			logger -t "SS" "退出订阅程序..."
			;;
		1 | *)
			logger -t "SS" "下载订阅失败...请检查你的网络..."
			rm -rf /tmp/ssr_subscribe_file.txt >/dev/null 2>&1 &
			let DEL_SUBSCRIBE+=1
			sleep 2
			logger -t "SS" "退出订阅程序..."
			;;
		esac
	done
	echo "var d_rules = [" >/etc/storage/dlink.js
	grep -v '^#' /tmp/newdlink.txt | grep -v "^$" >>/etc/storage/dlink.js
	echo "];" >>/etc/storage/dlink.js
	usleep 100000
	logger -t "SS" "已订阅全部节点,正在清理残余文件..."
	rm -rf /tmp/ssr_subscribe_file.txt >/dev/null 2>&1
	rm -rf /tmp/ssr_subscribe_file_temp1.txt >/dev/null 2>&1
	rm -rf /tmp/all_localservers >/dev/null 2>&1
	rm -rf /tmp/all_onlineservers >/dev/null 2>&1
	rm -rf /tmp/all_group_info.txt >/dev/null 2>&1
	rm -rf /tmp/group_info.txt >/dev/null 2>&1
	rm -rf /tmp/sub_group_info.txt >/dev/null 2>&1
	rm -rf /tmp/multi_*.txt >/dev/null 2>&1
	logger -t "SS" "已退出订阅程序...请手动刷新页面..."
}


get_oneline_rule_now() {
	i=0
	# ss订阅
	ssr_subscribe_link="$1"
	LINK_FORMAT=$(echo "$ssr_subscribe_link" | grep -E "^http://|^https://")
	[ -z "$LINK_FORMAT" ] && return 4

	logger -t "SS" "开始更新在线订阅列表..."
	logger -t "SS" "开始下载订阅链接到本地临时文件，请稍等..."
	rm -rf /tmp/ssr_subscribe_file* >/dev/null 2>&1

	if [ "$ss_basic_online_links_goss" == "1" ]; then
		open_socks_23456
		socksopen_b=$(netstat -nlp | grep -w 23456 | grep -E "local|v2ray")
		if [ -n "$socksopen_b" ]; then
			logger -t "SS" "使用$(get_type_name $ss_basic_type)提供的socks代理网络下载..."
			logger -t "SS" "下载地址：$ssr_subscribe_link"
			curl --connect-timeout 8 -s -L --socks5-hostname 127.0.0.1:23456 $ssr_subscribe_link >/tmp/ssr_subscribe_file.txt
		else
			logger -t "SS" "没有可用的socks5代理端口，改用常规网络下载..."
			curl --connect-timeout 8 -s -L $ssr_subscribe_link >/tmp/ssr_subscribe_file.txt
		fi
	else
		logger -t "SS" "使用curl下载订阅..."
		curl -k -s -o /tmp/ssr_subscribe_file.txt --connect-timeout 5 --retry 3 -L $ssr_subscribe_link
	fi

	#虽然为0但是还是要检测下是否下载到正确的内容
	if [ "$?" == "0" ]; then
		#订阅地址有跳转
		blank=$(cat /tmp/ssr_subscribe_file.txt | grep -E " |Redirecting|301")
		if [ -n "$blank" ]; then
			logger -t "SS" "订阅链接可能有跳转，尝试更换wget进行下载..."
			rm /tmp/ssr_subscribe_file.txt
			if [ "$(echo $ssr_subscribe_link | grep ^https)" ]; then
				wget --no-check-certificate --timeout=15 -qO /tmp/ssr_subscribe_file.txt $ssr_subscribe_link
			else
				wget -qO /tmp/ssr_subscribe_file.txt $ssr_subscribe_link
			fi
		fi
		#下载为空...
		if [ -z "$(cat /tmp/ssr_subscribe_file.txt)" ]; then
			logger -t "SS" "下载为空..."
			return 3
		fi
		#产品信息错误
		wrong1=$(cat /tmp/ssr_subscribe_file.txt | grep "{")
		wrong2=$(cat /tmp/ssr_subscribe_file.txt | grep "<")
		if [ -n "$wrong1" -o -n "$wrong2" ]; then
			return 2
		fi
	else
		logger -t "SS" "使用curl下载订阅失败，尝试更换wget进行下载..."
		rm /tmp/ssr_subscribe_file.txt
		if [ "$(echo $ssr_subscribe_link | grep ^https)" ]; then
			wget --no-check-certificate --timeout=15 -qO /tmp/ssr_subscribe_file.txt $ssr_subscribe_link
		else
			wget -qO /tmp/ssr_subscribe_file.txt $ssr_subscribe_link
		fi

		if [ "$?" == "0" ]; then
			#下载为空...
			if [ -z "$(cat /tmp/ssr_subscribe_file.txt)" ]; then
				echo_date 下载为空...
				return 3
			fi
			#产品信息错误
			wrong1=$(cat /tmp/ssr_subscribe_file.txt | grep "{")
			wrong2=$(cat /tmp/ssr_subscribe_file.txt | grep "<")
			if [ -n "$wrong1" -o -n "$wrong2" ]; then
				return 2
			fi
		else
			return 1
		fi
	fi
	if [ "$?" == "0" ]; then
		logger -t "SS" "开始解析节点信息..."
		decode_url_link $(cat /tmp/ssr_subscribe_file.txt) >/tmp/ssr_subscribe_file_temp1.txt
		# 检测ss ssr vmess
		NODE_FORMAT1=$(cat /tmp/ssr_subscribe_file_temp1.txt | grep -E "^ss://")
		NODE_FORMAT2=$(cat /tmp/ssr_subscribe_file_temp1.txt | grep -E "^ssr://")
		NODE_FORMAT3=$(cat /tmp/ssr_subscribe_file_temp1.txt | grep -E "^vmess://")
		if [ -n "$NODE_FORMAT2" ]; then
			# SSR 订阅
			NODE_NU=$(cat /tmp/ssr_subscribe_file_temp1.txt | grep -c "ssr://")
			logger -t "SS" "检测到ssr节点格式，共计$NODE_NU个节点..."
			#判断格式
			maxnum=$(decode_url_link $(cat /tmp/ssr_subscribe_file.txt) | grep "MAX=" | awk -F"=" '{print $2}' | grep -Eo "[0-9]+")
			if [ -n "$maxnum" ]; then
				urllinks=$(decode_url_link $(cat /tmp/ssr_subscribe_file.txt) | sed '/MAX=/d' | shuf -n $maxnum | sed 's/ssr:\/\///g')
			else
				urllinks=$(decode_url_link $(cat /tmp/ssr_subscribe_file.txt) | sed 's/ssr:\/\///g')
			fi
			[ -z "$urllinks" ] && continue
			for link in $urllinks; do
				decode_link=$(decode_url_link $link)
				#echo $decode_link >> /tmp/new.txt
				get_remote_config $decode_link 1
				[ "$?" == "0" ] && add_ssr_servers || logger -t "SS" "检测到一个错误节点，已经跳过！"
			done
			# 储存对应订阅链接的group信息
			if [ -n "$group" ]; then
				#dbus set ss_online_group_$z=$group
				echo $group >>/tmp/group_info.txt
			else
				# 如果最后一个节点是空的，那么使用这种方式去获取group名字
				group=$(cat /tmp/all_group_info.txt | sort -u | tail -n1)
				[ -n "$group" ] && dbus set ss_online_group_$z=$group
				[ -n "$group" ] && echo $group >>/tmp/group_info.txt
			fi
		elif [ -n "$NODE_FORMAT3" ]; then
			# v2ray 订阅
			# use domain as group
			v2ray_group_tmp=$(echo $ssr_subscribe_link | awk -F'[/:]' '{print $4}')
			# 储存对应订阅链接的group信息
			#dbus set ss_online_group_$z=$v2ray_group_tmp
			echo $v2ray_group_tmp >>/tmp/group_info.txt

			# detect format again
			if [ -n "$NODE_FORMAT1" ]; then
				#vmess://里夹杂着ss://
				NODE_NU=$(cat /tmp/ssr_subscribe_file_temp1.txt | grep -Ec "vmess://|ss://")
				logger -t "SS" "检测到vmess和ss节点格式，共计$NODE_NU个节点..."
				urllinks=$(decode_url_link $(cat /tmp/ssr_subscribe_file.txt) | sed 's/ssr:\/\///g')
			else
				#纯vmess://
				NODE_NU=$(cat /tmp/ssr_subscribe_file_temp1.txt | grep -Ec "vmess://")
				logger -t "SS" "检测到vmess节点格式，共计$NODE_NU个节点..."
				urllinks=$(decode_url_link $(cat /tmp/ssr_subscribe_file.txt) | sed 's/vmess:\/\///g')
				for link in $urllinks; do
					decode_link=$(decode_url_link $link)
					decode_link=$(echo $decode_link | jq -c .)
					echo $decode_link >>/tmp/new.txt
					if [ -n "$decode_link" ]; then
						get_v2ray_remote_config "$decode_link" "$v2ray_group_tmp"
						addv=$?
						if [ $check -ne 1 ]; then
							[ "$addv" == "0" ] && add_v2ray_servers || logger -t "SS" "检测到一个错误节点，已经跳过！"
						fi
					else
						logger -t "SS" "解析失败！！！"
					fi
				done
			fi
		fi
	fi
}


add_ssr_servers() {
	let i+=1
	check=0
	[ -n "$KEY_WORDS_1" ] && KEY_MATCH_1=$(echo $remarks | grep -Eo "$KEY_WORDS_1")
	[ -n "$KEY_WORDS_2" ] && KEY_MATCH_2=$(echo $remarks | grep -Eo "$KEY_WORDS_2")
	if [ -n "$KEY_WORDS_1" ] && [ -z "$KEY_WORDS_2" ]; then
		if [ -n "$KEY_MATCH_1" ]; then
			logger -t "SS" "不添加第$i【$remarks】节点，因为匹配了[排除]关键词"
			check=1
		fi

	elif [ -z "$KEY_WORDS_1" ] && [ -n "$KEY_WORDS_2" ]; then
		if [ -z "$KEY_MATCH_2" ]; then
			logger -t "SS" "不添加第$i【$remarks】节点，因为不匹配[包括]关键词"
			check=1
		fi

	elif [ -n "$KEY_WORDS_1" ] && [ -n "$KEY_WORDS_2" ]; then
		if [ -n "$KEY_MATCH_1" ] && [ -z "$KEY_MATCH_2" ]; then
			logger -t "SS" "不添加第$i【$remarks】节点，因为匹配了[排除]关键词"
			check=1
		elif [ -n "$KEY_MATCH_1" ] && [ -n "$KEY_MATCH_2" ]; then
			logger -t "SS" "不添加第$i【$remarks】节点，因为匹配了[排除+包括]关键词"
			check=1
		elif [ -z "$KEY_MATCH_1" ] && [ -z "$KEY_MATCH_2" ]; then
			logger -t "SS" "不添加第$i【$remarks】节点，因为不匹配[包括]关键词"
			check=1
		fi

	fi
	if [ $check = 0 ]; then
		logger -t "SS" "添加第$i【$remarks】节点..."
		add_link=""
		add_link="$add_link["'"ssr", '
		add_link="$add_link"'"'"$remarks"'", '
		add_link="$add_link"'"'"$server"'", '
		add_link="$add_link"'"'"$server_port"'", '
		add_link="$add_link"'"'"$password"'", '
		add_link="$add_link"'"'"$encrypt_method"'", '
		add_link="$add_link"'"'"$protocol"'", '
		add_link="$add_link"'"'"$protoparam"'", '
		add_link="$add_link"'"'"$obfs"'", '
		add_link="$add_link"'"'"$obfsparam"'", '
		add_link="$add_link"'], '
		echo "$add_link" >>/tmp/newdlink.txt
	fi
}



add_v2ray_servers() {
	logger -t "SS" "添加第$i【$v2ray_ps】节点..."
	add_link=""
	add_link="$add_link["'"v2ray", '
	add_link="$add_link"'"'"$v2ray_ps"'", '
	add_link="$add_link"'"'"$v2ray_add"'", '
	add_link="$add_link"'"'"$v2ray_port"'", '
	add_link="$add_link"'"'"$v2ray_aid"'", '
	add_link="$add_link"'"'"$v2ray_id"'", '
	add_link="$add_link"'"auto", '
	add_link="$add_link"'"'"$v2ray_net"'", '
	if [ "$v2ray_net" = "tcp" ]; then
		add_link="$add_link"'"'"$v2ray_type"'", '
		add_link="$add_link"'"'"$v2ray_host"'", '
	elif [ "$v2ray_net" = "kcp" ]; then
		add_link="$add_link"'"'"$v2ray_type"'", '
	elif [ "$v2ray_net" = "ws" ] || [ "$v2ray_net" = "h2" ]; then
		add_link="$add_link"'"'"$v2ray_host"'", '
		add_link="$add_link"'"'"$v2ray_path"'", '
	fi
	add_link="$add_link"'"'"$v2ray_tls"'", '
	add_link="$add_link"'], '
	echo "$add_link" >>/tmp/newdlink.txt
}


get_v2ray_remote_config() {
	let i+=1
	decode_link="$1"
	v2ray_group="$2"
	check=0
	v2ray_v=$(echo "$decode_link" | jq -r .v)
	v2ray_ps=$(echo "$decode_link" | jq -r .ps | sed 's/[ \t]*//g')
	[ -n "$KEY_WORDS_1" ] && KEY_MATCH_1=$(echo $v2ray_ps | grep -Eo "$KEY_WORDS_1")
	[ -n "$KEY_WORDS_2" ] && KEY_MATCH_2=$(echo $v2ray_ps | grep -Eo "$KEY_WORDS_2")
	if [ -n "$KEY_WORDS_1" ] && [ -z "$KEY_WORDS_2" ]; then
		if [ -n "$KEY_MATCH_1" ]; then
			logger -t "SS" "不添加第$i【$v2ray_ps】节点，因为匹配了[排除]关键词"
			check=1
		fi

	elif [ -z "$KEY_WORDS_1" ] && [ -n "$KEY_WORDS_2" ]; then
		if [ -z "$KEY_MATCH_2" ]; then
			logger -t "SS" "不添加第$i【$v2ray_ps】节点，因为不匹配[包括]关键词"
			check=1
		fi

	elif [ -n "$KEY_WORDS_1" ] && [ -n "$KEY_WORDS_2" ]; then
		if [ -n "$KEY_MATCH_1" ] && [ -z "$KEY_MATCH_2" ]; then
			logger -t "SS" "不添加第$i【$v2ray_ps】节点，因为匹配了[排除]关键词"
			check=1
		elif [ -n "$KEY_MATCH_1" ] && [ -n "$KEY_MATCH_2" ]; then
			logger -t "SS" "不添加第$i【$v2ray_ps】节点，因为匹配了[排除+包括]关键词"
			check=1
		elif [ -z "$KEY_MATCH_1" ] && [ -z "$KEY_MATCH_2" ]; then
			logger -t "SS" "不添加第$i【$v2ray_ps】节点，因为不匹配[包括]关键词"
			check=1
		fi

	fi

	if [ $check = 0 ]; then
		v2ray_add=$(echo "$decode_link" | jq -r .add | sed 's/[ \t]*//g')
		v2ray_port=$(echo "$decode_link" | jq -r .port | sed 's/[ \t]*//g')
		v2ray_id=$(echo "$decode_link" | jq -r .id | sed 's/[ \t]*//g')
		v2ray_aid=$(echo "$decode_link" | jq -r .aid | sed 's/[ \t]*//g')
		v2ray_net=$(echo "$decode_link" | jq -r .net)
		v2ray_type=$(echo "$decode_link" | jq -r .type)
		v2ray_tls_tmp=$(echo "$decode_link" | jq -r .tls)
		[ "$v2ray_tls_tmp"x == "tls"x ] && v2ray_tls="tls" || v2ray_tls="none"

		if [ "$v2ray_v" == "2" ]; then
			#echo_date "new format"
			v2ray_path=$(echo "$decode_link" | jq -r .path)
			v2ray_host=$(echo "$decode_link" | jq -r .host)
		else
			#echo_date "old format"
			case $v2ray_net in
			tcp)
				v2ray_host=$(echo "$decode_link" | jq -r .host)
				v2ray_path=""
				;;
			kcp)
				v2ray_host=""
				v2ray_path=""
				;;
			ws)
				v2ray_host_tmp=$(echo "$decode_link" | jq -r .host)
				if [ -n "$v2ray_host_tmp" ]; then
					format_ws=$(echo $v2ray_host_tmp | grep -E ";")
					if [ -n "$format_ws" ]; then
						v2ray_host=$(echo $v2ray_host_tmp | cut -d ";" -f1)
						v2ray_path=$(echo $v2ray_host_tmp | cut -d ";" -f1)
					else
						v2ray_host=""
						v2ray_path=$v2ray_host
					fi
				fi
				;;
			h2)
				v2ray_host=""
				v2ray_path=$(echo "$decode_link" | jq -r .path)
				;;
			esac
		fi

		#把全部服务器节点编码后写入文件 /usr/share/shadowsocks/serverconfig/all_onlineservers
		group_base64=$(echo $v2ray_group | sed 's/ -//g')
		server_base64=$(echo $v2ray_add | sed 's/ -//g')
		echo $server_base64 $group_base64 >>/tmp/all_onlineservers

		#echo ------
		#echo v2ray_v: $v2ray_v
		#echo v2ray_ps: $v2ray_ps
		#echo v2ray_add: $v2ray_add
		#echo v2ray_port: $v2ray_port
		#echo v2ray_id: $v2ray_id
		#echo v2ray_net: $v2ray_net
		#echo v2ray_type: $v2ray_type
		#echo v2ray_host: $v2ray_host
		#echo v2ray_path: $v2ray_path
		#echo v2ray_tls: $v2ray_tls
		#echo ------

		[ -z "$v2ray_ps" -o -z "$v2ray_add" -o -z "$v2ray_port" -o -z "$v2ray_id" -o -z "$v2ray_aid" -o -z "$v2ray_net" -o -z "$v2ray_type" ] && return 1 || return 0
	fi
}

			
decode_url_link() {
	link=$1
	len=$(echo $link | wc -L)
	mod4=$(($len % 4))
	if [ "$mod4" -gt "0" ]; then
		var="===="
		newlink=${link}${var:$mod4}
		echo -n "$newlink" | sed 's/-/+/g; s/_/\//g' | base64 -d 2>/dev/null
	else
		echo -n "$link" | sed 's/-/+/g; s/_/\//g' | base64 -d 2>/dev/null
	fi
}

reset_link() {
	echo "var d_rules = [" >/etc/storage/dlink.js
	echo "];" >>/etc/storage/dlink.js
	logger -t "SS" "已重置订阅节点文件,请手动刷新页面..."
}


case $1 in
start)
	start_link
	;;
reset)
	reset_link
	;;
*) 
    ;;

esac
