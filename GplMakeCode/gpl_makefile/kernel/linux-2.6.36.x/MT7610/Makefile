EXTRA_CFLAGS = -Idrivers/net/wireless/MT7610/rlt_wifi/include \
				-Idrivers/net/wireless/MT7610/rlt_wifi/ate/include

DRV_NAME = rlt_wifi
RT_WIFI_DIR = ../rlt_wifi

obj-$(CONFIG_MT7610_AP) += MT7610_ap.o

MT7610_ap-objs += $(RT_WIFI_DIR)/rate_ctrl/ra_ctrl.o
MT7610_ap-objs += $(RT_WIFI_DIR)/rate_ctrl/alg_grp.o
MT7610_ap-objs += $(RT_WIFI_DIR)/rate_ctrl/alg_legacy.o
MT7610_ap-objs += $(RT_WIFI_DIR)/mgmt/mgmt_entrytb.o
MT7610_ap-objs += $(RT_WIFI_DIR)/mgmt/mgmt_hw.o
MT7610_ap-objs += $(RT_WIFI_DIR)/mgmt/mgmt_vht.o
MT7610_ap-objs += $(RT_WIFI_DIR)/mgmt/mgmt_ht.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/ap_ioctl.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/rt_rbus_pci_drv.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/rt_pci_rbus.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/rt_rbus_pci_util.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/br_ftph.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/pci_main_dev.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/rt_profile.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/rt_linux.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/rt_proc.o
MT7610_ap-objs += $(RT_WIFI_DIR)/os/linux/rt_main_dev.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/mcu/rtmp_and.o
MT7610_ap-objs += $(RT_WIFI_DIR)/mcu/mcu.o
MT7610_ap-objs += $(RT_WIFI_DIR)/mcu/mcu_and.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_data_pci.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/eeprom.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_profile.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_sync.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/rtmp_init_inf.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_cfg.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/rt_rf.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/action.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/rtmp_init.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/txpower.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_cmd.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/rtmp_timer.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_info.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/ba_action.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_mac_pci.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_tkip.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_asic.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/vht.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_aes.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/crypt_md5.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_wep.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/crypt_arc4.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/crypt_hmac.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_wpa.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/ps.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_sanity.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_radar.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/crypt_aes.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/ee_efuse.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_data.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/spectrum.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/crypt_sha2.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/mlme.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/ee_prom.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/rt_os_util.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/rt_channel.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/scan.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/uapsd.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/rt_led.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_qload.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_auth.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_sync.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_mlme.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_wpa.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_assoc.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_cfg.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_autoChSel.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_sanity.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_data.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_connect.o
MT7610_ap-objs += $(RT_WIFI_DIR)/chips/rtmp_chip.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/chips/rt30xx.o
MT7610_ap-objs += $(RT_WIFI_DIR)/chips/rt65xx.o
MT7610_ap-objs += $(RT_WIFI_DIR)/chips/mt76x0.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/FPGA/verf.o
MT7610_ap-objs += $(RT_WIFI_DIR)/phy/rtmp_phy.o
MT7610_ap-objs += $(RT_WIFI_DIR)/phy/rlt_rf.o
MT7610_ap-objs += $(RT_WIFI_DIR)/phy/rlt_phy.o
MT7610_ap-objs += $(RT_WIFI_DIR)/mac/ral_nmac.o
MT7610_ap-objs += $(RT_WIFI_DIR)/mac/rtmp_mac.o
MT7610_ap-objs += $(RT_WIFI_DIR)/common/netlink.o

ifeq ($(CONFIG_MT7610_AP_FLASH),y)
	EXTRA_CFLAGS += -DRTMP_FLASH_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/ee_flash.o
endif

ifeq ($(CONFIG_MT7610_AP_WMM_ACM),y)
	EXTRA_CFLAGS += -DWMM_ACM_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/acm_edca.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/acm_comm.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/acm_iocl.o
endif

ifeq ($(CONFIG_MT7610_AP_LED),y)
	EXTRA_CFLAGS += -DWLAN_LED -DLED_CONTROL_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/rt_led.o
ifeq ($(CONFIG_MT7610_AP_WSC),y)
#	EXTRA_CFLAGS += -DWSC_LED_SUPPORT
endif
endif

ifeq ($(CONFIG_MT7610_AP_WSC),y)
	EXTRA_CFLAGS += -DWSC_AP_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/wsc.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/wsc_tlv.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/crypt_dh.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/crypt_biginteger.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/wsc_ufd.o
ifeq ($(CONFIG_MT7610_AP_WSC_V2),y)
	EXTRA_CFLAGS += -DWSC_V2_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/wsc_v2.o
endif
endif

ifeq ($(CONFIG_MT7610_AP_WDS),y)
	EXTRA_CFLAGS += -DWDS_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_wds.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_wds_inf.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/client_wds.o
endif

ifeq ($(CONFIG_MT7610_AP_MBSS),y)
	EXTRA_CFLAGS += -DMBSS_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_mbss.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_mbss_inf.o
endif

ifeq ($(CONFIG_MT7610_AP_APCLI),y)
	EXTRA_CFLAGS += -DAPCLI_SUPPORT
	EXTRA_CFLAGS += -DMAT_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_apcli.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_apcli_inf.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/apcli_assoc.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/apcli_auth.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/apcli_ctrl.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/apcli_sync.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_mat.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_mat_iparp.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_mat_pppoe.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_mat_ipv6.o
endif

ifeq ($(CONFIG_MT7610_AP_IGMP_SNOOP),y)
	EXTRA_CFLAGS += -DIGMP_SNOOP_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/igmp_snoop.o
endif

ifeq ($(CONFIG_MT7610_AP_NETIF_BLOCK),y)
	EXTRA_CFLAGS += -DBLOCK_NET_IF
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/netif_block.o
endif

ifeq ($(CONFIG_MT7610_AP_DFS),y)
	EXTRA_CFLAGS += -DDFS_SUPPORT
	EXTRA_CFLAGS += -DDFS_HARDWARE_SUPPORT -DDFS_FCC_BW40_FIX -DDFS_DEBUG 
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_dfs.o
endif

ifeq ($(CONFIG_MT7610_AP_CARRIER),y)
	EXTRA_CFLAGS += -DCARRIER_DETECTION_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_cs.o
endif

ifeq ($(CONFIG_MT7610_AP_DLS),y)
	EXTRA_CFLAGS += -DQOS_DLS_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_dls.o
endif

ifeq ($(CONFIG_MT7610_AP_IDS),y)
	EXTRA_CFLAGS += -DIDS_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_ids.o
endif

ifeq ($(CONFIG_MT7610_AP_WAPI),y)
	EXTRA_CFLAGS   += -DWAPI_SUPPORT -DSOFT_ENCRYPT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/wapi.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/wapi_sms4.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/wapi_crypt.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/libwapi.a
endif

ifeq ($(CONFIG_MT7610_80211R_FT),y)
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/ft.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/ft_iocl.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/ft_tlv.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/ft_rc.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_ftkd.o
endif

ifeq ($(CONFIG_MT7610_80211K_RR),y)
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/rrm_tlv.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/rrm_sanity.o
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/rrm.o
endif

ifeq ($(CONFIG_MT7610_AP_VIDEO_TURBINE),y)
	EXTRA_CFLAGS   += -DVIDEO_TURBINE_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/common/cmm_video.o
endif

ifeq ($(CONFIG_MT7610_AP_CSO),y)
	EXTRA_CFLAGS   += -DCONFIG_CSO_SUPPORT -DCONFIG_TSO_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/naf/net_acc.o
endif

#ifeq ($(CONFIG_MT7610_AP_MESH),y)
#EXTRA_CFLAGS += -DMESH_SUPPORT -DINTEL_CMPC -DSCAN_SUPPOR
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh_link_mng.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh_ctrl.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh_forwarding.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh_bmpkt.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh_inf.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh_tlv.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh_sanity.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/common/mesh_path_mng.o
#endif

###################
#  CFLAGS
##################

	EXTRA_CFLAGS += -DDOT11_N_SUPPORT -DDOT11_VHT_AC -DSTATS_COUNT_SUPPORT -DNEW_RATE_ADAPT_SUPPORT
	EXTRA_CFLAGS += -DAGGREGATION_SUPPORT -DPIGGYBACK_SUPPORT -DWMM_SUPPORT  -DLINUX -Wall -Wstrict-prototypes -Wno-trigraphs \
			-DSYSTEM_LOG_SUPPORT -DRESOURCE_PRE_ALLOC -DFPGA_MODE -DENHANCED_STAT_DISPLAY
			EXTRA_CFLAGS += -DCONFIG_AP_SUPPORT  -DUAPSD_SUPPORT -DIAPP_SUPPORT -DDOT1X_SUPPORT -DAP_SCAN_SUPPORT -DSCAN_SUPPORT -DLED_CONTROL_SUPPORT
#EXTRA_CFLAGS += -DRALINK_ATE -DCONFIG_RT2880_ATE_CMD_NEW -DRALINK_QA
EXTRA_CFLAGS += -DMT76x0 -DRT65xx -DMT7610 -DMT7630 -DMT7650 -DRLT_MAC -DRLT_RF -DRTMP_MAC_PCI -DRTMP_PCI_SUPPORT -DA_BAND_SUPPORT -DRX_DMA_SCATTER -DRTMP_EFUSE_SUPPORT -DRA_NAT_SUPPORT -DCONFIG_ANDES_SUPPORT
EXTRA_CFLAGS += -DVCORECAL_SUPPORT -DFIFO_EXT_SUPPORT
EXTRA_CFLAGS += -DMCS_LUT_SUPPORT -DPEER_DELBA_TX_ADAPT -DDYNAMIC_VGA_SUPPORT
#EXTRA_CFLAGS += -DDBG_DIAGNOSE -DDBG_RX_MCS -DDBG_TX_MCS

ifeq ($(CONFIG_MT7610_AP_ATE),y)
EXTRA_CFLAGS += -Idrivers/net/wireless/MT7610_ap/ate/include
MT7610_ap-objs += $(RT_WIFI_DIR)/ate/common/rt_ate.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ate/common/ate_pci.o
MT7610_ap-objs += $(RT_WIFI_DIR)/ate/chips/rt6590_ate.o
#MT7610_ap-objs += $(RT_WIFI_DIR)/ate/chips/rt85592_ate.o
EXTRA_CFLAGS += -DRALINK_ATE
EXTRA_CFLAGS += -DCONFIG_RT2880_ATE_CMD_NEW
#EXTRA_CFLAGS += -DRTMP_RF_RW_SUPPORT
#EXTRA_CFLAGS += -DRF_BANK
endif

ifeq ($(CONFIG_MT7610_AP_QA),y)
MT7610_ap-objs += $(RT_WIFI_DIR)/ate/common/rt_qa.o
EXTRA_CFLAGS += -DRALINK_QA
EXTRA_CFLAGS += -DENHANCED_STAT_DISPLAY
endif


#EXTRA_CFLAGS += -DCONFIG_RA_NAT_NONE
EXTRA_CFLAGS += -DRESOURCE_PRE_ALLOC

ifeq ($(CONFIG_MT7610_AUTO_CH_SELECT_ENCANCE),y)
	EXTRA_CFLAGS += -DAUTO_CH_SELECT_ENHANCE
endif

ifeq ($(CONFIG_MT7610_AP_80211N_DRAFT3),y)
	EXTRA_CFLAGS += -DDOT11N_DRAFT3
endif

ifeq ($(CONFIG_SINGLE_SKU),y)
	EXTRA_CFLAGS += -DSINGLE_SKU
endif

ifeq ($(CONFIG_MT7610_SNMP),y)
	EXTRA_CFLAGS += -DSNMP_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_LLTD),y)
	EXTRA_CFLAGS += -DLLTD_SUPPORT
endif

ifeq ($(CONFIG_NEW_MBSSID_MODE),y)
	EXTRA_CFLAGS += -DNEW_MBSSID_MODE
#ifeq ($(CONFIG_RALINK_MT7620),y)
	EXTRA_CFLAGS += -DENHANCE_NEW_MBSSID_MODE
#	endif
endif

ifeq ($(CONFIG_MT7610_AP_COC),y)
	EXTRA_CFLAGS   += -DGREENAP_SUPPORT -DCOC_SUPPORT
endif

ifeq ($(CONFIG_MT7610_EXT_CHANNEL_LIST),y)
	EXTRA_CFLAGS   += -DEXT_BUILD_CHANNEL_LIST
endif

ifeq ($(CONFIG_KTHREAD),y)
	EXTRA_CFLAGS   += -DKTHREAD_SUPPORT
endif

ifeq ($(CONFIG_MT7610_AP_MEMORY_OPTIMIZATION),y)
	EXTRA_CFLAGS   += -DMEMORY_OPTIMIZATION
else
	EXTRA_CFLAGS   += -DDBG
endif

ifeq ($(CONFIG_RA_NETWORK_WORKQUEUE_BH),y)
	EXTRA_CFLAGS   += -DWORKQUEUE_BH
endif
# Mod by sungeyu:3005驱动mac地址穿透功能有缺陷，RTMPInsertRepeaterEntry插入条目失败。
# 先去掉该功能，和2.4G的功能保持一致。
ifeq ($(CONFIG_MT7610_AP_MAC_REPEATER),y)
	EXTRA_CFLAGS += -DMAC_APCLI_SUPPORT
#	EXTRA_CFLAGS += -DMAC_REPEATER_SUPPORT
#	EXTRA_CFLAGS += -DAPCLI_AUTO_CONNECT_SUPPORT
	MT7610_ap-objs += $(RT_WIFI_DIR)/ap/ap_repeater.o
endif

#ifeq ($(CONFIG_MT7610_AP_HDR_TRANS),y)
#	EXTRA_CFLAGS += -DHDR_TRANS_SUPPORT
#endif

#ifeq ($(CONFIG_MT7610_AP_BIG_ENDIAN),y)
#	EXTRA_CFLAGS += -DRT_BIG_ENDIAN
#endif

ifeq ($(CONFIG_MT7610_AP_TSSI_COMPENSATION),y)
	EXTRA_CFLAGS += -DMT76x0_TSSI_CAL_COMPENSATION
endif

ifeq ($(CONFIG_RTMP_TEMPERATURE_COMPENSATION),y)
	EXTRA_CFLAGS += -DRTMP_TEMPERATURE_COMPENSATION
endif

ifeq ($(CONFIG_MT7610_AP_SINGLE_SKU),y)
	EXTRA_CFLAGS += -DSINGLE_SKU_V2
endif

ifeq ($(CONFIG_GUEST_ACCESS_WEB_FILTER),y)
EXTRA_CFLAGS += -DGUEST_ACCESS_WEB_FILTER
endif
