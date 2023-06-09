﻿var SEcode = {
		1000 : '未知錯誤，請聯繫開發商',
		1001 : '請確認刪除',
		1002 : 'You did not select to connect',
		1003 : 'Port can not be zero',
		1004 : 'MAC address format is xx:xx:xx:xx:xx:xx',
		1005 : 'Device is not available',
		1006 : 'Reboot necessary',
		1007 : 'Exceed max MAC address number',
		1008 : 'Please confirm to modify',
		1009 : 'Please select only one item.',
		1010 : '密碼不相符',
		1011 : 'Reboot necessary,are you sure',
		1012 : 'Are you sure you want to quit Setup Wizard and discard settings?',
		1013 : 'Please select country and ISP.',
		1014 : 'User name or password cannot be empty!',
		1015 : 'Invalid PVC list!',
		1016 : 'PVC already existed!',
		1017 : '無效',
		1018 : 'Stale',
		2000 : '請選擇檔案升級',
		2001 : '檔案載入中，請稍後!',
		2002 : '載入的檔案必須是.xml檔案',
		2003 : '載入的檔案必須是.bin檔案',
		2004 : 'Please add a IPv6 connection!',
		2005 : 'Please confirm to activate',
		2006 : '載入的檔案必須是.lng檔案',
		2014 : 'Wrong ip, Please check',
		2015 : 'The ip address can not be same with the router address',
		2016 : 'Please enter the ServiceName',
		3000 : 'Enable white list but no rule will block all users.Do you want to continue?',
		4000 : 'WPS session in progress ==> Inprogress',
		4001 : 'WPS session overlap==> Overlap',
		4002 : 'Add new device failed! ==> Timeout',
		4003 : 'Add new device success! ==> Success',
		4004 : 'Attention:To configurate WPS ,the WLAN security mode must be None or WPA-PSK or WPA2-PSK mode and not the TKIP mode!',
		4005 : 'No PIN Value!',
		4006 : 'PIN must be 8 digits',
		4007 : '無效的 PIN 碼！',
		4008 : 'The Station-PIN is wrong!',
		4009 : 'WPS is connecting ,please wait for a moment.',
		8000 : '啟用',
		8001 : '停用',
		8002 : 'PASS',
		8003 : '失敗',
		8004 : '已連線',
		8005 : '已中斷連線',
		8006 : '無',
		8007 : '不可用',
		8008 : 'Hide',
		8009 : 'Visibility',
		8010 : 'LinkLocalConnected',
		8011 : 'GlobalConnected',
		8012 : 'OnDemand',
		9000 : 'Method not supported',
		9001 : 'Request rejected',
		9002 : 'Internal error',
		9003 : 'Invalid parameters',
		9004 : 'Resource inadequate',
		9005 : 'Invalid parameter name',
		9006 : 'Invalid parameter type',
		9007 : 'Invalid parameter value',
		9008 : 'Parameter(s) can not be written',
		9009 : 'Parameters change notification request was refused',
		9010 : 'Download fail',
		9011 : '上傳失敗',
		9012 : 'File transfer server verification failure',
		9013 : 'the file transfer protocol is not supported',
		9898 : 'Fail to send message',
		9897 : 'Memory inadequate',
		9896 : 'Parameter error',
		9895 : 'Inadequate other resources,such as control block',
		9894 : '逾時',
		9893 : 'Conflict,such as share error,illegal reload',
		9892 : 'Empty pointer',
		9886 : 'There is no corresponding instance',
		9885 : 'The system is now configuring, please wait',
		9887 : 'There must be at least one rule when you enable white list',
		9888 : 'Please select one item',
		lang_select_name : '請先選擇一個「應用程式名稱」!',
		lang_trigger_port : '請輸入觸發連接埠範圍。',
		lang_trigger_port_invalid : '無效的觸發埠設定!',
		lang_trigger_port_conflict_vs : '輸入的Trigger port不得與Virtual Server相同!',
		lang_trigger_port_conflict_pf : '輸入的Trigger port不得與Port Forwarding相同!',
		lang_firewall_port : '請輸入防火牆的連接埠範圍!',
		lang_firewall_port_invalid : '無效的防火牆通訊埠設定！',
		lang_firewall_port_conflict_vs : '輸入的Firewall port不能與Virtual Server相同!',
		lang_firewall_port_conflict_pf : '輸入的Firewall port不得與Port Forwarding相同!',
		lang_firewall_port_conflict : '輸入的Firewall port不得與管理功能中的遠端admin port相同!',
		lang_tcp_port : '無效的TCP連接埠範圍。',
		lang_tcp_port_conflict_vs : '輸入的TCP port不得與Virtual Server相同。',
		lang_tcp_port_conflict_pt : '輸入的TCP port不得與Port Trigger相同。',
		lang_udp_port : '無效的UDP連接埠範圍。',
		lang_udp_port_conflict_vs : '輸入的UDP port不得與Virtual Server相同。',
		lang_udp_port_conflict_pt : '輸入的UDP port不得與Port Trigger相同。',
		lang_port_invalid : '無效的通訊埠!',
		lang_tcp_port_overlap : 'TCP連接埠是重覆的!',
		lang_udp_port_overlap : 'UDP連接埠是重覆的!',
		lang_tcp_port_conflict : '您輸入的TCP連接埠不可以與遠端管理連接埠相同。',
		lang_ip_be_lan : 'IP 位址必須位在 LAN 子網路中',
		lang_destip_invalid : '無效的目的地IP位址!',
		lang_gateway_invalid : '無效的通訊閘IP位址',
		lang_whitelist_warning : '若您不新增一個MAC位址的話，所有的電腦將無法連上網路!',
		lang_fw_name_empty : '防火牆的名稱不可以空白!',
		lang_startip_source_empty : '來源的起始IP位址不可以空白。',
		lang_prefix_source_empty : 'prefixlen of source不得為空。',
		lang_prefix_source_invalid : '無效的prefixlen of source。',
		lang_interface_not_same : '來源與目的地的介面不應該相同!',
		lang_startip_dest_empty : '目的地的起始 IP 位址不可以空白!',
		lang_prefix_dest_empty : 'prefixlen of dest不得為空。',
		lang_prefix_dest_invalid : '無效的prefixlen of dest。',
		lang_start_port_empty : '目的地的起始連接埠號不可為空白!',
		lang_end_port_empty : '目的地的end port不得為空。',
		lang_name : '名稱 "',
		lang_already_used : '\"已經被使用!',
		lang_rule : '規則 "',
		lang_name_empty : '名稱不得為空',
		lang_destip_ipv6_empty : '目的地IPV6位址不得為空!',
		lang_prefix_empty : 'Prefix長度不得為空!',
		lang_metric_empty : 'Metric不得為空',
		lang_gateway_empty : 'Gateway不得為空!',
		lang_policy_name_empty : '規則名稱不可以空白!',
		lang_policy_name_same : '規則名稱不可以與其他規則相同!',
		lang_enter_machine : '請輸入一台裝置。',
		lang_select_filter : '請至少需選擇一種過濾方式。',
		lang_name_all_empty : '該名稱不得為空。',
		lang_name_same : '名稱不可以相同!',
		lang_destip_start : '不正確的目的地IP位址! 起始IP位址是無效的。',
		lang_destip_end : '不正確的目的地IP位址! 結束IP位址是無效的。',
		lang_endip_greater_startip : '結束IP位址應該大於起始IP位址。',
		lang_start_port : '無效的起始連接埠!',
		lang_end_port : '無效的結束連接埠!',
		lang_endport_greater_startport : '結束連接埠應該大於起始連接埠。',
		lang_max_machines : '允許的電腦數已超出上限。',
		lang_machine_conflict : '此電腦已存在!',
		lang_uplink_speed : '無效的上行速度!',
		lang_downlink_speed : '無效的下行速度!',
		lang_rules_not_same : '規則不能相同!',
		lang_localip_start : '無效的本地起始IP.',
		lang_localip_end_empty : '本地結束IP不得為空白!',
		lang_localip_end : '無效的本地結束IP!',
		lang_localip_start_empty : '本地起始IP不得為空白!',
		lang_remoteip_start : '無效的遠端起始IP!',
		lang_remoteip_end_empty : '遠端結束IP不得為空白!',
		lang_remoteip_end : '無效的本地結束IP!',
		lang_remoteip_start_empty : '遠端起始IP不得為空白!',
		lang_localip_range : '無效的本地IP範圍!',
		lang_remoteip_range : '無效的遠端IP範圍!',
		lang_localip_start_not_lan : '本地起始IP不得在LAN subnet中!',
		lang_localip_end_not_lan : '本地結束IP不得在LAN subnet中!',
		lang_remoteip_start_conflict : '遠端起始IP不得與LAN IP相同。',
		lang_remoteip_start_lan : '遠端起始IP必須在LAN subnet中!',
		lang_remoteip_end_conflict : '遠端結束IP不得與LAN IP相同。',
		lang_remoteip_end_lan : '遠端結束IP必須在LAN subnet中!',
		lang_localip_start_conflict : '本地起始IP不得與LAN IP相同。',
		lang_localip_start_lan : '本地起始IP必須在LAN subnet中!',
		lang_localip_end_conflict : '本地結束IP不得與LAN IP相同。',
		lang_localip_end_lan : '本地結束IP必須在LAN subnet中!',
		lang_remoteip_start_not_lan : '遠端起始IP不得在LAN subnet中!',
		lang_remoteip_end_not_lan : '遠端結束IP不得在LAN subnet中!',
		lang_max_rules_32 : '分類規則最多為32條!',
		lang_select_machine : '請先選擇一個裝置!',
		lang_dmzip_be_lan : 'DMZ IP位址應該在區域網路的網段內!',
		lang_dmzip_invalid : 'DMZ的IP位址無效!',
		lang_ssid_not_empty : 'SSID欄位不可以空白!',
		lang_ssid_printable : 'SSID需為可列印的字元!',
		lang_ssid_not_blank : '「無線網路名稱」的字首或字尾不可以有空白。',
		lang_wepkey_not_blank : '「WEP金鑰」的字首或字尾不可以有空白。',
		lang_wepkey_printable : 'WEP金鑰應該是可以被列印出來的字元。',
		lang_wepkey_not_hex : 'WEP金鑰應該是可以被列印出來的字元。',
		lang_wepkey_length : 'WEP金鑰必須為64bit或128bit。WEP金鑰應該是5、10、13或26個字元。',
		lang_wpa_interval : '金鑰的更新間隔時間數值應該介於30到65536之間。',
		lang_wpakey_length : 'PSK密碼的長度必須為8個字元以上。',
		lang_wpakey_64_hex : 'PSK應該是64個字元。',
		lang_wpakey_printable : 'PSK應該是8-63個可列印的字元。',
		lang_wpakey_not_blank : '「網路金鑰」的字首或字尾不可以有空白。',
		lang_radiuskey_not_blank : '「RADIUS伺服器Shared Secret」的字首或字尾不可以有空白。',
		lang_radius_ip_invalid : '無效的RADIUS  IP位址!',
		lang_radiuskey_length : '密碼長度必須為1到64字元。',
		lang_radiuskey_printable : 'RADIUS server共享密碼長度必須為1到64個字元。',
		lang_name_not_blank : '「名稱」欄位不能為空白。',
		lang_name_not_blank_space : '「名稱」欄位不可有空格或空白!',
		lang_one_range : '至少使用一個遠端IP位址範圍',
		lang_start_ip : '起始IP位址是無效的!',
		lang_start_ip_not_same : '起始IP位址不可以在區域網路內!',
		lang_end_ip : '結束IP位址是無效的!',
		lang_end_ip_not_same : '結束IP位址不可以在區域網路內!',
		lang_rule_is_used : '此規則已被使用。',
		lang_public_port : '您輸入的公用連接埠是無效的!',
		lang_public_port_conflict_pf : '輸入的public port不得與Port Forwarding相同。',
		lang_public_port_conflict_pt : '輸入的public port不得與Port Trigger相同。',
		lang_private_port : '您輸入的私人連接埠位址是無效的!',
		lang_private_port_conflict_pf : '輸入的private port不得與Port Forwarding相同。',
		lang_private_port_conflict_pt : '輸入的private port不得與Port Trigger相同。',
		lang_host_ip : '無效的主機 IP位址!',
		lang_ip_not_same : 'IP位址不可與LAN IP位址相同!',
		lang_lan_wan_conflict : 'LAN IP位址不得與WAN IP位址在同一個網段。',
		lang_lan_pptp_server_conflict : 'LAN IP位址不得與PPTP Server IP位址相同。',
		lang_lan_l2tp_server_conflict : 'LAN IP位址不得與L2TP Server IP位址相同。',
		lang_gw_not_same : 'Gateway IP位址不得與LAN IP位址相同。',
		lang_gw_lan_conflict : 'Gateway IP位址不得在LAN subnet中。',
		lang_host_name : '無效的主機名稱!',
		lang_name_not_same : '不同的規則不能被設定成相同的名稱。',
		lang_public_port_not_same : '不同的規則不可以使用相同公用連接埠。',
		lang_public_port_conflict : '您輸入的公用連接埠不可以與遠端管理連接埠相同。',
		lang_wps1 : 'WPS不支援這些加密方式:\n  - WPA-個人級(僅 WPA 或僅 TKIP)\n - WPA-企業\n  - WEP \n請至「設定」>「無線設定」頁面將安全模式設為其他類型來啟用WPS功能。',
		lang_wps2 : '',
		lang_wps3 : '當MAC位址過濾的功能啟用時無法啟用WPS。\n請至「進階」>「MAC位址過濾」的頁面選擇停用網路過濾功能來啟用WPS。',
		vilidity : '無效',
		tcpiplan_dhcp_001 : '如果您更改網段，設備將會重新啟動，DHCP Reservations List, Virtual Server rules, Port Forwarding rules 及 DMZ hosts list將會被刪除。\n您確定要變更LAN IP地址或是Netmask?',
		tcpiplan_dhcp_002 : '如果您更改路由器的IP位址，您必須重新登入。\n您確定要變更LAN IP地址?',
		lang_not_find_mac : '無法搜尋到您電腦的MAC位址，請手動輸入。',
		lang_invalid_mac : '無效的MAC位址!',
		lang_user_empty : '使用者名稱不可以是空白',
		lang_passwd_empty : '密碼不可空白',
		lang_passwd_not_match : '密碼不相符',
		lang_mtu_invalid : '無效的MTU數值。',
		lang_mtu_dhcp_small : 'MTU數值太小，有效的值應該介於1280 - 1500 之間。',
		lang_mtu_dhcp_large : 'MTU數值太大，有效的值應該介於 1280 - 1500 之間。',
		lang_mtu_pppoe_small : 'MTU數值太小，有效的數值不可小於1280。',
		lang_mtu_pppoe_large : 'MTU的數值不在要求的範圍內! 有效的MTU值必須介於1280-1492。',
		lang_mtu_small : 'MTU數值太小，有效的數值不可小於1280。',
		lang_mtu_largel : 'MTU的數值太大! 有效的MTU值必須介於1280-1460。',
		lang_mask_invalid : '無效的子網路遮罩!',
		lang_gate_invalid : '無效的預設閘道位址!',
		lang_gate_same_ip : 'Gateway IP位址與IP位址必須在同一個網路中!',
		lang_pri_dns_invalid : '無效的主要DNS 位址',
		lang_sec_dns_invalid : '無效的次要DNS 位址',
		lang_invalid_ip : '無效的IP位址',
		lang_pptp_ip_empty : 'PPTP IP位址不得為空!',
		lang_pptp_mask_empty : 'PPTP Subnet Mask不得為空!',
		lang_invalid_mask : '無效的子網路遮罩',
		lang_pptp_gw_empty : 'PPTP Gateway IP位址不得為空!',
		lang_invalid_def_gate : '無效的預設閘道位址',
		lang_pptp_gate_same_ip : ' PPTP Gateway IP位址不得與PPTP IP位址相同。',
		lang_gate_same_net_ip : 'Gateway IP位址與IP位址必須在同一個網路中!',
		lang_pptp_server_same_ip : 'PPTP Server IP位址不得與PPTP IP位址相同。',
		lang_server_same_ip : 'Server IP位址不得與IP位址在同一個網路中!',
		lang_pptp_server_empty : 'PPTP Server IP位址不得為空!',
		lang_invalid_pptp_server : '無效的PPTP服務器IP位址',
		lang_l2tp_ip_empty : 'L2TP IP位址不得為空!',
		lang_l2tp_mask_empty : 'L2TP Subnet Mask不得為空!',
		lang_l2tp_gate_empty : 'L2TP Gateway IP位址不得為空!',
		lang_l2tp_gate_same_ip : 'L2TP Gateway位址不得與L2TP IP位址相同。',
		lang_l2tp_server_same_ip : 'L2TP Server IP位址不得與L2TP IP位址相同。',
		lang_l2tp_server_empty : 'L2TP Server IP位址不得為空!',
		lang_invalid_l2tp_server : '無效的L2TP服務器IP位址',
		lang_AFTR_ipv6_addr_empty : 'AFTR IPv6位址不得為空。',
		lang_pppoe_sharewith_ipv4 : 'IPv6 PPPoE 與 IPv4 PPPoE 共享。請先將 IPv4 WAN 通訊協定改為 PPPoE !',
		lang_blank_idletime : '無效的閒置時間!有效的閒置時間必須介於1-1092。 ',
		lang_admin_passwd_empty : '系統管理員密碼不得為空。',
		lang_invalid_passwd : '無效密碼',
		lang_passwd_not_match : '請輸入兩行相同的密碼再重試',
		lang_ddns_server_addr : '請選擇伺服器地址。',
		lang_ddns_host_name : '請輸入主機名稱。',
		lang_ddns_user_account : '請輸入使用者帳戶。',
		lang_ddns_passwd : '請輸入密碼。',
		lang_ddns_vfpasswd : '密碼和確認密碼與新密碼不相符。',
		lang_ddns_period : '無效的時間! 逾時範圍應介於 1~8670之間!',
		lang_ddns_ajax : 'Ajax 回傳錯誤:',
		lang_email_addr : '請輸入有效的 Email 位址',
		lang_email_server : '請輸入另一個SMTP伺服器或IP位址。',
		lang_email_smtpport : 'SMTP連接埠是無效的!',
		lang_email_user : '請輸入一個使用者名稱。',
		lang_email_passwd : '請輸入一個有效的密碼。',
		lang_email_vfpasswd : '密碼不相符!',
		lang_syslog_host : 'SYSLOG主機需在同一區域網路內!',
		lang_syslog_select : '請先選擇一個裝置!',
		lang_ping_host : '請輸入主機名稱或 IP 位址以進行 Ping 測試。',
		lang_sec_num : '項目數最大為10',
		lang_sec_used : '此排程已被使用!',
		lang_time_server : '無效的NTP伺服器',
		lang_pass_port : '遠端管理通訊埠不是正確的。',
		lang_pass_pvused : '遠端管理連接埠己被用於虛擬伺服器功能中。',
		lang_pass_psused : '遠端管理連接埠己被用於連接埠轉傳功能中。',
		lang_pass_paused : '遠端admin port已被應用規則功能被使用。',
		lang_system_reboot : '\n這個動作將會導致所有使用中的連線中斷，您確定要重新啟動裝置嗎?',
		lang_system_reset : '\n這個動作將會把目前所有的設定全部清空，您確定要將裝置回復成原廠預設值嗎?',
		lang_ip_dup : '重覆的IP位址。',
		lang_mac_dup : '重覆的MAC位址。',
		lang_exce_maxnum : 'DHCP保留的規則已超出上限。',
		lang_start_less_end : '起始位址必須小於結束位址。',
		lang_cannot_share : '當WPS啟用時不可選擇共用金鑰',
		lang_ssid_empty : 'SSID 不可為空白',
		lang_prefix_blank : '在\"無線網路名稱\"的Prefixc或postfix不得為空',
		lang_pre_wep_blank : '「WEP金鑰」的字首或字尾不可以有空白。',
		lang_wep_printable : 'WEP金鑰應該是可以被列印出來的字元。',
		lang_wep_print_hex : 'WEP金鑰應該是可以被列印出來的字元。',
		lang_wep_length : 'WEP金鑰必須為64bit或128bit。WEP金鑰應該是5、10、13或26個字元。',
		lang_psk_least8 : 'PSK密碼的長度必須為8個字元以上。',
		lang_psk_64hex : 'PSK應該是64個字元。',
		lang_psk_printable : 'PSK密碼應為8至63個字元。',
		lang_pre_psk_blank : '「網路金鑰」的字首或字尾不可以有空白。',
		lang_groupkey_length : '金鑰的更新間隔時間數值應該介於30到65536之間。',
		lang_secret_length : '密碼長度必須為1到64字元。',
		lang_RADIUS_printable : 'RADIUS server共享密碼長度必須為1到64個字元。',
		lang_RADIUS_blank : '「RADIUS伺服器Shared Secret」的字首或字尾不可以有空白。',
		lang_RADIUS_ip_invalid : '無效的RADIUS  IP位址!',
		lang_wpa_disable_wps : 'WPS必須要停用才能使用僅WPA或僅TKIP的無線加密方式，您確定要繼續設定嗎?',
		lang_wpa_enter_disable_wps : '若要使用 WPA-企業級的安全模式必須停用 WPS功能，您確定要繼續設定嗎? ',
		lang_wep_disable_wps : 'WPS必須被停用時才能使用WEP加密，您確定要繼續設定嗎?',
		lang_hidden_ssid_disable_wps : 'WPS必須為停用時才能使用隱藏SSID功能，您確定要繼續設定嗎?',
		lang_warn_24_wlan : '注意!  將無線安全模式選擇為「無」時將可允許所有人連到您的網路，可能會有安全的風險或影響您的頻寬，您確定要繼續設定嗎?',
		lang_warn_5_wlan : '注意!  將無線安全模式選擇為「無」時將可允許所有人連到您的網路，可能會有安全的風險或影響您的頻寬，您確定要繼續設定嗎?',
		lang_invalid_pin : '無效的PIN碼!',
		lang_cancel_setting : '您想要放棄對此精靈所做的所有變更？',
		lang_passwd_blank : '密碼不可為空白!',
		lang_wan_as_gate : '您確認此WAN為預設閘道?',
		lang_changeV4_wan_pro : 'IPv6 PPPoE是和IPv4 PPPoE共用，請先將IPv4的連線類型更改為PPPoE。',
 	    lang_set_tunnel :  '請先更改IPv4的連線類型。',
		lang_set_dslite	:  '請先更改IPv6的連線類型。',
		lang_undefined : '未定義',
		lang_lanv6_empty : 'LAN的IPv6位址不得為空',
		lang_lanv6_null : 'LAN的IPv6位址為空',
		lang_invalid_mode : '無法選擇此模式，因為 IPv4 Internet 連線類型為「DS - Lite」。',
		lang_set_pppoev4 : 'IPv6 PPPoE是和IPv4 PPPoE共用，請先將IPv4的連線類型更改為PPPoE。',
		lang_select_mode : '請選擇一個模式!',
		lang_passwd_8_63 : '密碼長度不正確，應為8至63個字元'
};                                                                                      
var UEcode = { 
		1001 : 'This function is not supported right now',
		1002 : 'Invalid IP format',
		1003 : 'Invalid port format',
		1004 : 'Port cant be zero or negative,should be between 1-65535',
		1005 : 'Port should be between 1-65535',
		1006 : 'Invalid priority value',
		1007 : 'Priority too low,should be between 1-8',
		1008 : 'Priority too high,should be between 1-8',
		1009 : 'Net mask invalid format',
		1010 : 'Net mask can not be emty',
		1011 : 'Net mask can not be zero',
		1012 : 'the left part of net mask must be continuous binary 1',
		1013 : 'enable should not be zero',
		1014 : 'bool val must be "0" or "1"',
		1015 : 'must be int value',
		1016 : 'must be unsigned int',
		1017 : '無效的MAC位址!',
		1018 : 'ip number more than max limit',
		1019 : 'Invalid ip list format',
		1020 : 'Invalid mac list format',
		1021 : 'Too many MAC-adresses',
		1022 : 'Too many instances,cant add new one',
		1023 : 'This instance can not be deleted',
		1024 : 'inner msg format error',
		1025 : 'inner msg has repeated name',
		1026 : 'inner cache error',
		1027 : 'invalid username',
		1028 : 'invalid password',
		1029 : 'IP address does not match with the subnet mask ',
		1030 : 'invalid max bit rate',
		1031 : 'invalid duplex mode',
		1032 : 'IP address can not be broadcast address ',
		1033 : 'IP address can not be host address ',
		1034 : 'IP end address lower then start address',
		1035 : 'IP address pool overlaps ',
		1036 : 'String is too long ',
		1037 : 'can not be empty',
		1038 : 'get node value failed',
		1039 : 'Set node value failed',
		1040 : 'The input domain name is illegal',
		1041 : 'the value out of range',
		1042 : 'IP list has to repeat',
		1043 : 'wrong net section',
		1044 : 'IP address can not be broadcast address or net address',
		1045 : 'DHCP couldnt perform as server and relay simultaneously',
		1046 : 'invalid portrange (min > max)',
		1047 : 'invalid value',
		1048 : 'The list are duplicates',
		1049 : 'invalid username or password',
		1050 : '無效的主機名稱!',
		1051 : '無效的伺服器 IP 位址',
		1052 : 'Port not in range',
		1053 : 'IP not in range',
		1054 : 'Server IP can not be empty',
		1055 : 'IP Addr can not be empty',
		1056 : 'Gateway can not be empty',
		1060 : 'ERR_MSG_MID_INVALID',
		1061 : 'ERR_MSG_SOCKET',
		1062 : 'ERR_MSG_PROC_NOT_FOUND',
		1063 : 'ERR_MSG_SEND_FAIL',
		1064 : 'ERR_MSG_DSTMID_UNREGED',
		1065 : 'ERR_MSG_NOT_FULL',
		1066 : 'ERR_MSG_PART_NOEFFECT',
		1067 : 'ERR_MSG_PART_INVALID',
		1068 : 'ERR_MSG_TIMEOUT',
		1069 : 'ERR_MSG_PART_LIST_FULL',
		1070 : 'ERR_MALLOC_FAILED',
		1071 : 'ERR_BUF_NOT_ENOUGH',
		1072 : 'ERR_INTERNAL_ERR',
		1073 : 'ERR_PARA_INVALID',
		1074 : 'ERR_FILE_OPEN',
		1075 : 'ERR_FILE_READ',
		1076 : 'ERR_FILE_WRITE',
		1077 : 'ERR_FILE_CHKSUM',
		1078 : 'ERR_BUSY',
		1079 : 'ERR_CREATE_MSG_FAIL',
		1080 : 'ERR_MSG_EXT_MID_LIST_FULL',
		1087 : '結束IP位址應該大於起始IP位址。',
		1088 : 'Invalid ipv6 address format',
		1089 : 'Repetitive ipv6 address exists',
		1090 : 'Invalid ipv6 prefix length',
		1091 : 'Invalid DUID',
		1092 : 'IPv6 invalid prototype',
		1093 : 'IPv6 profixlen not match',
		1099 : 'Service Name DUPLICATE, please check the Service Name',
		1102 : 'Invalid filter mode',
		1103 : 'Invalid schedule path',
		1202 : 'cannot modify user name',
		1203 : 'cannot modify user name',
		1204 : 'cannot remove admin',
		1205 : 'have same user',
		1206 : 'password length must less than 32',
		1207 : 'user name length must less than 32',
		1208 : 'Too many users logged on, please wait a moment',
		1209 : 'Username wrong',
		1210 : 'Password wrong',
		1211 : 'Username or Password wrong',
		1212 : 'Session is timeout',
		1213 : 'Inadequate access',
		1214 : 'Please login in one minute',
		1215 : 'Logout success',
		1216 : 'Old password is wrong',
		1401 : 'Not support the opt to this path',
		1402 : 'Invalid index',
		1403 : 'Invalid leaf node',
		1404 : 'Invalid item',
		1405 : 'Invalid path',
		1406 : 'time out',
		1451 : 'Stat data not support set noti',
		1452 : 'Path not exist',
		1453 : 'Attemp to set val to obj',
		1454 : 'Invalid string val',
		1455 : '0 must expressed by "0", "+0""-0""00" are all invalid',
		1456 : 'non-zero int must begin with "+" or non-zero number',
		1457 : 'include invalid char',
		1458 : '0 must expressed by "0", "+0""-0""00" are all invalid',
		1459 : 'non-zero unit must begin non-zero number',
		1460 : 'include invalid char',
		1461 : 'bool val must be "0" or "1"',
		1462 : 'dateTime val must be like "0000-00-02T03:04:05"',
		1463 : 'Hex val must be number or A-F a-f',
		1464 : 'The path is not writable',
		1465 : 'Not in accesslist',
		1466 : 'The path is not a valid inst',
		1467 : 'The path is not a standard path',
		1468 : 'The node type is invalid',
		1469 : 'App config flash opt failed',
		1470 : 'Wildcard queue is full',
		1471 : 'Invalid path',
		1472 : 'The object instance count has reached the max allowed count',
		1475 : 'This node reject to set of active notication',
		1501 : 'Invalid IP format',
		1502 : '無效的LAN端IP位址',
		1503 : 'Mac address list too long',
		1504 : '無效的MAC 位址',
		1505 : 'Net Mask conflict with IP',
		1506 : 'Repeated IP',
		1507 : 'This is first IP config,cannot delete',
		1508 : 'Too many LAN IP config',
		1509 : 'same net region with other lan device,will bring route error',
		1510 : 'please select a lan interface',
		1515 : 'Invalid IPv6 address format',
		1516 : 'Router ip and wan ip should not in the same netmask',
		1602 : 'DMZ host ip should not be null',
		1680 : 'The ip address could not be in LAN network',
		16902 : '請輸入另一個 SMTP 伺服器或 IP 位址。',
		1901 : 'no resource',
		1902 : 'Invalid value',
		1903 : 'Invalid path',
		1904 : 'exceeded number of connections',
		1905 : 'Invalid instance',
		1906 : 'Invalid enable',
		1907 : 'Invalid  username',
		1908 : 'username  over  length',
		1909 : 'username empty',
		1910 : 'Invalid passwd',
		1911 : 'passwd over length',
		1912 : 'passwd empty',
		1913 : 'connected name invalid',
		1914 : 'connected name over length',
		1915 : 'Invalid auth',
		1916 : 'MRU invalid(576~1492)',
		1917 : 'Invalid trigger',
		1918 : 'connect button invalid',
		1919 : 'LCPECHO invalid, should be 5 ~ 65535',
		1920 : 'exceed active connect',
		1921 : 'inner message invalid',
		1922 : 'the index of message invalid',
		1923 : 'no pppsession in flash',
		1924 : 'Invalid pppsession idx',
		1925 : 'pppsession  value error',
		1926 : 'WAN connected name repeated',
		1932 : 'Invalid idle time',
		1933 : 'MTU invalid(576~1492)',
		1934 : 'Invalid AC name',
		1935 : 'Invalid service name',
		1939 : 'Invalid apn',
		1940 : 'apn empty',
		1941 : 'apn over  length',
		1942 : 'Invalid dialnumber',
		1943 : 'dialnumber empty',
		1944 : 'dialnumber over length',
		1945 : 'CPE can support up to 1 PPPoU connections',
		1946 : 'LCPECHO retry invalid, should be 0 ~ 65535',
		1947 : 'Backup delay time invalid, should be 0~600',
		1948 : 'Backup delay time empty',
		1949 : 'Backup delay time  must be divisible by 15',
		1950 : 'IPv6 MTU invalid(1280~1492)',
		1951 : 'IPv6 MRU invalid(1280~1492)',
		1952 : 'invalid pppoe address mode',
		1953 : 'Server IP address and Static IP address can not be the same',
		1954 : 'IP address does not match with the subnet mask',
		1955 : 'Static IP address and default gateway address can not be the same',
		1956 : 'Static IP address and default gateway address is not in the same network',
		2001 : 'no resourse',
		2002 : 'Invalid value',
		2003 : 'Invalid path',
		2004 : 'value empty',
		2005 : 'The number of Vap can not exceed 5',
		2006 : 'Invalid instance',
		2007 : 'Invalid Vap index',
		2008 : 'It is already in OOB status',
		2009 : 'name repeated',
		2011 : 'wps session is already started',
		2013 : 'the length of SSID can not exceed 32',
		2014 : 'WEP and TKIP encryption is not supported in gn/bgn/n/an mode',
		2023 : 'mac must be "xx:xx:xx:xx:xx:xx" format and must not be broadcast or multicast',
		2024 : 'the max  number of stations should be set between 1-32',
		2025 : 'Get channel or AutoChannelEnable failed',
		2026 : 'Channel and AutoChannelEnable conflict',
		2027 : 'The SSID should not be empty',
		2028 : 'The length of the passphrase (PSK) must be at least 8 characters',
		2029 : 'The length of the wep key must be 5 characters(10 Hex number) or 13 characters(26 Hex number)',
		2101 : 'Invalid URL',
		2102 : 'Invalid Cafile',
		2103 : 'unable to connect to the server',
		2104 : 'unable to connect to the server',
		2202 : 'Invalid state (must be "Requested")',
		2203 : 'Invalid intf (not exsited)',
		2204 : 'Invalid host (not support partly ip)',
		2205 : 'Invalid host (not support oct format ip)',
		2206 : 'Invalid host (must begin with number,char or "_")',
		2207 : 'Invalid Repeate times,should be 1 ~ 50',
		2208 : 'Invalid timeout,should be 1 ~ 20',
		2209 : 'Invalid data size,should be 1 ~ 5000',
		2210 : 'Invalid DSCP',
		2211 : 'not set host',
		2212 : 'Invalid host (must not contain char except ".", "_", "-", "@", a-z A-Z 0-9)',
		2213 : 'Invalid host (too long, exceeding 256)',
		2401 : 'Unknown class id',
		2402 : 'Already exist class id ',
		2403 : 'Interface address and dhcp address pool not match',
		2404 : 'lease time超出範圍，應大於或等於10',
		2410 : 'CPE IP conflicts with DHCP pool IP addr',
		2601 : 'the length of connection name can not over 256',
		2602 : 'Invalid connection name (only contains the following characters: [-_.@0-9a-zA-Z] ) ',
		2603 : 'Invalid address type value',
		2604 : 'the number of active connections to the ceiling',
		2605 : 'the number of WAN connections to the ceiling',
		2606 : 'Each WAN Device allow to create just a IP Connection',
		2607 : 'Static IP address and default gateway address can not be the same',
		2608 : 'Connection Name has exists',
		2610 : 'Invalid connection type',
		2611 : 'Static IP address and default gateway address is not in the same network',
		2617 : 'Invalid MTU value,the valid range is 616 to 1500 bytes',
		2619 : 'Invalid MTU format',
		2701 : 'The number of rules can not be over 20',
		2702 : 'this rule already exists in filter table',
		2703 : 'Source IP Address can not be empty !',
		2704 : 'Destination IP Address can not be empty !',
		2801 : 'Invalid SNMP version',
		2902 : 'Invalid Bandwidth value, should be 100-1024000Kbps',
		2903 : 'Invalid peak rate value, should be 100-1024000Kbps',
		2904 : 'Invalid burst size, should be 5-100bytes',
		2908 : 'Queue length should be 10-150packets or 15000-225000bytes',
		2910 : 'Invalid queue weight value,should be 0-100',
		2912 : 'Invalid queue bandwidth value, should be 0-1024000Kbps',
		2915 : 'Invalid ingress interface',
		2919 : 'Invalid vlan id, shoud be 1-4094',
		2920 : 'Invalid ip length value,should be 20-1500',
		2924 : 'Invalid index of queue instance',
		2925 : 'The queue instance does not exist',
		2928 : 'Committed rate should be less than or equal to peak rate',
		2929 : 'Queue length should be 20-150packets or 30000-225000bytes',
		2930 : 'The weighted sum of all enabled queues should be less than or equal to 100%',
		2931 : 'Two enabled queues binding the same SP interface should not have the same priority',
		2932 : 'The minimum ip length should be less than or equal to the maximum ip length',
		2933 : 'The minimum port value should be less than or equal to the maximum port value',
		2934 : 'The configuration with same app name has existed',
		2935 : 'The bandwidth sum of all enabled queues shouled be less than or equal to the global bandwidth value',
		2936 : 'Invalid IP format',
		2937 : 'Invalid IPV6 format,prefix length should be 0-128',
		2939 : 'Invalid source IP format',
		2940 : 'Invalid destination IP format',
		2941 : 'Invalid Flow Label value,should 0-1048575',
		2942 : 'Invalid value of the Traffic Class,should be 0-255',
		2943 : 'The same Classification Rules has existed',
		2944 : 'Classification Rules should not be empty',
		3001 : 'Invalid instance',
		3002 : 'Invalid interface',
		3003 : 'has no resource',
		3004 : 'value invalid',
		3005 : 'enable invalid',
		3006 : 'Invalid path',
		3007 : 'Invalid default route parameter',
		3008 : 'Invalid route para',
		3009 : 'Invalid parameter',
		3010 : 'del instance error',
		3011 : 'Invalid metric',
		3012 : 'Invalid IP address',
		3013 : 'IP empty',
		3014 : 'Invalid netmask',
		3015 : 'netmask empty',
		3016 : 'Invalid gateway',
		3017 : 'metric is out of limits(0~255)',
		3018 : 'Invalid log file',
		3021 : 'IP address does not match with the subnet mask ',
		3023 : 'Invalid interface',
		3024 : 'duplicate routing',
		3025 : 'duplicate destination address',
		3201 : 'Invalid DNS address',
		3202 : 'DNS address repeat',
		3203 : 'When allowed a custom DNS, DNS addresses can not be empty',
		3401 : 'inner msg error',
		3402 : 'get invalid value',
		3403 : 'Invalid Service List',
		3404 : 'Invalid Connection Type',
		3405 : 'Invalid Lan Intf',
		3406 : 'The lan intf mutil bind',
		3407 : 'Not allowed to create more than one bridge on the same vlan',
		3408 : 'Not allowed to mix bridge and route on the same vlan',
		3409 : 'The TR069 service type is not allowed to config on a bridge conn',
		3410 : 'Not allowed to bind lan to a TR069 wan conn',
		3411 : 'Invalid Vlan ID',
		3413 : 'please select a wan connection',
		3414 : 'For one (PVC,VLAN), only 1 MER, 1 Bridging and 4 PPPOE connections can be configured at the same time',
		3415 : 'For one VLAN,only 1 IPOE, 1 Bridging and 4 PPPOE connection can be configured at the same time',
		3417 : 'For one PVC, only one  can be configured',
		3418 : 'For one PVC, MER, PPPOE and Bridging are only allowed with IPOA and PPPOA',
		3419 : 'CPE can support up to 8 PVCs',
		3420 : 'For one PVC, more than one MER, PPPOE or Bridging connetion exist, can not change protocol to IPOA or PPPOA',
		3421 : 'CPE can support up to 4 enabled PPPOA and PPPOE connections',
		3422 : 'CPE can support up to 4 enabled PPPOE connections',
		3423 : 'For one (PVC,VLAN), only 1 MER, 1 Bridging connections can be configured at the same time',
		3424 : 'For one VLAN, only 1 IPOE, 1 Bridging connections can be configured at the same time',
		3431 : 'Only one TR069 service type can exist',
		3432 : 'Invalid DSL type',
		3433 : 'Vlan ID repeated',
		3501 : 'value invalid',
		3502 : 'Invalid configurate file',
		3503 : 'Invalid inner parament ',
		3504 : 'RIP version invalid',
		3505 : 'WAN device invalid',
		3506 : 'PC inner message invalid',
		3517 : 'RIP direction invalid',
		3601 : 'Invalid update interval value, shoud be 1-24(hours)',
		3602 : 'Invalid retry interval value, should be 1-60(minutes)',
		3603 : 'The Ip address or domain name of NTP server is invalid',
		3606 : 'Invalid time format',
		3607 : 'The NTP servers should not be null',
		3608 : 'Two NTP servers should not be same value',
		3609 : 'Theres a conflict between the start and the end of daylight saving time',
		3610 : 'Invalid year,should be 2000-2036/02/5',
		3901 : 'Log file upload failed',
		3902 : 'Invalid TFTP server',
		3903 : 'Log file clear failed',
		3904 : 'Server address cannot be empty',
		3905 : 'Invalid server',
		4301 : 'The number of rules can not be over 10',
		4302 : 'This open port has been occupied',
		4303 : 'When protocol is ICMP, the port value must be zero',
		4304 : 'When protocol is not ICMP, The port value can not be zero',
		4305 : 'This port is in use',
		4401 : 'This IP address is already in the list',
		4501 : 'Invalid bridge name',
		4502 : 'Invalid Vlan ID',
		4503 : 'Invalid Filter Bridge Reference',
		4504 : 'Invalid Filter Interface',
		4506 : 'the first bridge should not be deleted',
		4507 : 'the instance which is in use should not be deleted',
		4508 : 'Bridge name conflict',
		4509 : 'Vlan id conflict',
		4510 : 'One interface must belong to only one LAN Group',
		4511 : 'VAP interface should not be tagged mode',
		4512 : 'Bridge name cannot be empty',
		4531 : 'Invalid Vlan ID',
		4532 : 'Invalid Vlan ID list',
		4533 : 'too many Vlan ID in list',
		4903 : 'Invalid characters inclued in description',
		4904 : 'The minimum port value should be less than or equal to the maximum port value',
		4905 : 'Two instances conflict for the conflicting tuple composed of RemoteHost, ExternalPort and Protocol',
		4906 : 'Port 7547 is in use by TR069, please choose another port',
		4907 : 'Invalid Host Name',
		4908 : 'Service IP and LAN IP are not in the same subnet',
		4909 : 'The port is occupied',
		4910 : 'The port value of  Internal Port End is too big ',
		4911 : 'Host IP can not for empty and 255.255.255.255',
		5001 : 'Invalid inner parameter',
		5002 : 'Invalid value',
		5003 : 'value enable empty',
		5004 : 'value enable wrong',
		5005 : 'Invalid config file',
		5006 : 'value mode empty',
		5007 : 'value mode wrong',
		5008 : 'Duplicated URL',
		5009 : 'URL Number is over',
		5010 : 'URL length is over',
		5011 : 'Invalid URL',
		5012 : 'Invalid filter mode',
		5013 : 'Invalid time',
		5014 : 'Invalid day',
		5015 : 'Start time is higher than end time',
		5016 : 'MAC already exists',
		5101 : 'VPI/VCI is invalid',
		5102 : 'Invalid VPI value (0~255)',
		5103 : 'Invalid VCI value (32~65535)',
		5104 : 'Unkown encapsulation',
		5105 : 'Unkown Qos type',
		5106 : 'Invalid PCR value (0~7100)',
		5107 : 'Invalid MBR value (0~1000000)',
		5108 : 'Invalid SCR value (0~7099)',
		5109 : 'Wrong MCR value (0~pcr)',
		5111 : 'Wrong vlan id (0~4095)',
		5112 : 'Wrong vlan priority(0~7)',
		5113 : 'Same vlan id',
		5301 : 'Invalid DDNS provider',
		5302 : 'Invalid hostname',
		5303 : 'Hostname length is high',
		5304 : 'Invalid WAN connection ID',
		5305 : 'Invalid username',
		5306 : 'Username length is too high',
		5307 : 'Invalid password',
		5308 : 'Password length is high',
		5309 : 'Invalid email address',
		5310 : 'Email address length is high',
		5311 : 'Invalid key',
		5312 : 'Key length is too high',
		5313 : 'Hostname already exists',
		5314 : 'Invalid DDNS host',
		5315 : 'DDNS host length is too high',
		5316 : 'www.oray.cn exists already',
		5801 : 'username or password empty',
		5901 : 'Invalid filter mode',
		5902 : 'Invalid time',
		5903 : 'Invalid day',
		5904 : 'Start time is higher than end time',
		5905 : 'MAC already exists',
		5906 : 'Invalid inner parameter',
		5907 : 'Invalid username',
		5910 : 'In allow mode, not allow to delete the only one white list',
		5911 : 'Invalid MAC address value',
		6001 : 'Traceroute is not started',
		6002 : 'Traceroute is running',
		6401 : 'Name is invalid',
		6402 : '您輸入的開始時間是無效的',
		6403 : '您輸入的結束時間是無效的',
		6404 : 'No days are specified',
		6405 : 'Day select is invalid',
		6406 : 'Day select is invalid',
		6407 : 'Start time is invalid',
		6409 : '「名稱」不可以相同!',
		6601 : 'IP address is empty',
		6602 : 'IP address is invalid',
		6603 : 'You can use this function unless there is at least one IP',
		6604 : 'Have the same entry',
		6701 : 'IP address is empty',
		6702 : 'IP address is invalid',
		6703 : 'User interface is empty',
		6704 : 'User interface is invalid',
		6705 : 'Mode is empty',
		6706 : 'Mode must be one of AutoDeftGW,GWIP andUserInIf',
		7401 : 'Diagnostics state is invalid',
		7402 : 'Interface is invalid',
		7403 : 'Too many PVC pairs in search list',
		7404 : 'PVC INVALID',
		9201 : 'the life time is out of range',
		9202 : 'the start address is higher than the end one',
		9203 : 'the life time is out of range',
		9204 : 'DHCPv6LeaseTime must be lower than DHCPv6ValidTime',
		9205 : 'IPv6 Interface ID range error',
		9301 : 'the ipv6 static addr is wrong',
		9302 : 'Invalid DNS Address',
		9303 : 'the ipv6 router addr is wrong',
		9304 : 'Invalid IP Address Config Type',
		9305 : 'Invalid Route Config Type',
		9306 : 'the duid type is wrong',
		9307 : 'Invalid ipv6 prefix address,eg:2003::',
		9308 : 'Ipv6 prefix address conflict with prefix len',
		9309 : 'Ipv6 static address conflict with prefix len',
		9401 : 'Invalid hex number',
		9402 : 'Invalid global id,must be 40bit hex number',
		9403 : 'Invalid config type,must be Static or Delegated',
		9404 : 'lifetime range is 1~90',
		9405 : 'route preference must be low,medium or high',
		9407 : 'SubnetID range is 0~255',
		9408 : 'Invalid prefix length, it should be 0~128',
		9409 : 'Invalid prefix',
		9425 : 'PreferredLifeTime must smaller than ValidLifeTime',
		9501 : 'The length of PIN code must be 4',
		9502 : 'The length of PUK code must be 8',
		9503 : 'The PIN/PUK code must be decimal',
		9804 : 'Invalid IP ! Please input a right IPv6 address',
		9805 : 'Route Had Exist',
		9806 : 'Failed to get static ip,Geteway or Interface',
		9807 : 'ipv6 address can not be empty',
		9808 : 'V6_addr count error',
		10701 : 'no resourse',
		10702 : 'value empty',
		10703 : 'name over maxlength',
		10704 : 'name is valid string',
		10705 : 'Already exist tunnel name',
		10706 : 'Wan connection is in use',
		10707 : 'There is not Wan connection',
		10708 : 'IPTunnel ipv6 prefix must be XXXXX:XXXX/X model',
		10709 : 'IPTunnel ipv4 mask length can not be empty',
		10710 : 'IPTunnel ipv4 mask length must be 0~32',
		10711 : 'Invalid parameter',
		10712 : 'IP empty',
		10713 : 'Invalid IP address',
		10714 : 'IPTunnel ipv6 prefix length must be 1~64',
		10715 : 'IPTunnel ipv6 address format error',
		10716 : 'The total of ipv6 prefix length and ipv4 mask length must be 1~64'
}
