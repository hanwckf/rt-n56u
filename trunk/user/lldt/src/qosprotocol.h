/*
 * LICENSE NOTICE.
 *
 * Use of the Microsoft Windows Rally Development Kit is covered under
 * the Microsoft Windows Rally Development Kit License Agreement,
 * which is provided within the Microsoft Windows Rally Development
 * Kit or at http://www.microsoft.com/whdc/rally/rallykit.mspx. If you
 * want a license from Microsoft to use the software in the Microsoft
 * Windows Rally Development Kit, you must (1) complete the designated
 * "licensee" information in the Windows Rally Development Kit License
 * Agreement, and (2) sign and return the Agreement AS IS to Microsoft
 * at the address provided in the Agreement.
 */

/*
 * Copyright (c) Microsoft Corporation 2005.  All rights reserved.
 * This software is provided with NO WARRANTY.
 */

#ifndef QOS_PROTOCOL_H
#define QOS_PROTOCOL_H

/* Function (opcode) for demultiplex header */
typedef enum {
    Qopcode_InitializeSink       = 0x00,
    Qopcode_Ready,              /* 0x01 */
    Qopcode_Probe,              /* 0x02 */
    Qopcode_Query,              /* 0x03 */
    Qopcode_QueryResp,          /* 0x04 */
    Qopcode_Reset,              /* 0x05 */
    Qopcode_Error,              /* 0x06 */
    Qopcode_ACK,                /* 0x07 */
    Qopcode_CounterSnapshot,    /* 0x08 */
    Qopcode_CounterResult,      /* 0x09 */
    Qopcode_CounterLease,       /* 0x0A */
    Qopcode_INVALID		// must be last Qopcode
} qos_opcode_t;

typedef enum {
    Qoserror_InsufficientResources = 0x00,
    Qoserror_Busy,
    Qoserror_ModerationNotAvailable,
    Qoserror_INVALID		// must be last Qoserror
} qos_error_t;


static const char * const Qos_errors[] =
{
    "",	// Errors start at 1....
    "QosInsufficientResources",
    "QosBusy",
    "QosModerationNotAvailable",
    "Invalid-Error"
};


static const char * const Qos_opcode_names[] =
{
    "QosInitializeSink",
    "QosReady",
    "QosProbe",
    "QosQuery",
    "QosQueryResp",
    "QosReset",
    "QosError",
    "QosAck",
    "QosCounterSnapshot",
    "QosCounterResult",
    "QosCounterLease",
    "Invalid-Opcode"
};

/* The ethernet header with 802.1q tags included */
typedef struct {
    etheraddr_t qeh_dst;
    etheraddr_t qeh_src;
    uint16_t    qeh_qtag;
    uint16_t    qeh_ptag;
    uint16_t    qeh_ethertype;
} __attribute__ ((packed)) qos_ether_header_t;

typedef struct {
    uint8_t	qbh_version;	/* Version */
    uint8_t	qbh_tos;	/* Type of Svc (0=>Discovery, 1=>Quick Disc, 2=> QoS */
    uint8_t	qbh_resrvd;	/* Reserved, must be zero */
    uint8_t	qbh_opcode;	/* qos_opcode_t */
    etheraddr_t	qbh_realdst;	/* intended destination */
    etheraddr_t	qbh_realsrc;	/* actual source */
    uint16_t	qbh_seqnum;	/* 0 or a valid sequence number */
} __attribute__ ((packed)) qos_base_header_t;


typedef struct {
    uint8_t	init_intmod_ctrl;	/* 0=> disable; 1=> enable; 0xFF=> use existing */
} __attribute__ ((packed)) qos_initsink_header_t;


typedef struct {
    uint32_t	rdy_linkspeed;	/* units of 100 bits per second */
    uint64_t	rdy_tstampfreq;	/* units of ticks per second */
} __attribute__ ((packed)) qos_ready_header_t;


typedef struct {
    uint64_t	probe_txstamp;	/* set by Controller */
    uint64_t	probe_rxstamp;	/* sent as 0; set by Sink when received */
    uint64_t	probe_rtxstamp;	/* sent as 0; set by Sink on return (probegap only) */
    uint8_t	probe_testtype;	/* 0=> timed probe; 1=> probegap; 2=> probegap-return */
    uint8_t	probe_pktID;	/* Controller cookie */
    uint8_t	probe_pqval;	/* 1st bit==1 => ValueIsValid; next 7 bits are Value for 802.1p field */
    uint8_t	probe_payload[0]; /* indeterminate length; Controller determines, Sink just returns it */
} __attribute__ ((packed)) qos_probe_header_t;

/* qos_query_header_t is empty. only the base header appears in the msg */

typedef struct {
    uint16_t	qr_EvtCnt; /* count of 18-octet "qosEventDescr_t's" in payload (max = 82) */
//  qosEventDescr_t	qr_Events[qr_EvtCnt]
} __attribute__ ((packed)) qos_queryresponse_header_t; 

typedef struct {
    uint64_t	ctrlr_txstamp;	/* copied from probe_txstamp */
    uint64_t	sink_rxstamp;	/* copied from probe_rxstamp */
    uint8_t	evt_pktID;	/* returning the Controller cookie from probe_pktID */
    uint8_t	evt_reserved;	/* must be zero */
} __attribute__ ((packed)) qosEventDescr_t;

/* qos_reset_header_t is empty. only the base header appears in the msg */

typedef struct {
    uint16_t	qe_errcode;	/* enum is: qos_error_t */
} __attribute__ ((packed)) qos_error_header_t;

typedef struct {
    uint16_t	cnt_rqstd;	/* max # non-sub-sec samples to return */
} __attribute__ ((packed)) qos_snapshot_header_t;

typedef struct {
    uint8_t     subsec_span;
    uint8_t     byte_scale;
    uint8_t     pkt_scale;
    uint8_t     history_sz;
} __attribute__ ((packed)) qos_counter_hdr;       /* format of QosCounterResult */

typedef struct {
    uint16_t    bytes_rcvd;     /* all values stored in NETWORK byte order! */
    uint16_t    pkts_rcvd;
    uint16_t    bytes_sent;
    uint16_t    pkts_sent;
} __attribute__ ((packed)) qos_perf_sample;

#endif /* QOS_PROTOCOL_H */
