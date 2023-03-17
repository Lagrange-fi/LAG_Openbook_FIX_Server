#pragma once

namespace marketlib
{
    enum security_request_result_t
    {
        /*
         Valid values: Int
        '0' 	Valid request
        '1' 	Invalid or unsupported request
        '2' 	No instruments found that match selection criteria
        '3' 	Not authorized to retrieve instrument data
        '4' 	Instrument data temporarily unavailable
        '5' 	Request for instrument data not supported
         */

        srr_valid=0,
        srr_invalid_or_unsupported=1,
        srr_no_instruments=2,
        srr_not_authorized=3,
        srr_instrument_temporary_unavailable=4,
        srr_req_not_supported=5,
    };
    enum subscription_type
    {
        snapshot = '0',
        shapshot_update = '1',
        snapshot_update_disable = '2',
    };

    enum subscription_update_type_t
    {
        full_refresh = 0,
        incemental_refresh = 1,
    };

    enum security_type_t{
        st_undef = -1,
        st_crypto,
        st_forex,
        st_stock,
        st_futures,
        st_cfd
    };

    enum order_type_t 	{
        ot_Market='1',    //market
        ot_Limit='2',     //limit
        ot_Stop_Loss='3', //stop
        ot_Stop_Limit='4',//stoplimit
        ot_Undefined
    };

    enum time_in_force_t {
        tf_Day='0',                //day
        tf_Good_Till_Cancel='1',   //gtc
        tf_At_Open='2',
        tf_Immediate_Or_Cancel='3',//ioc
        tf_Fill_Or_Kill='4',       //fok
        tf_Good_Till_Crossing='5',
        tf_Good_Till_Date='6',
        tf_At_Close='7',
        tf_Undefined
    };

    enum order_side_t {
        os_Buy = '1',  //buy
        os_Sell ='2',  //sell
        os_Undefined
    };


    /*
        FIX 4.4 : OrdStatus <39> field
        Type: char
        Used In
        Description
        Identifies current status of order.
        Valid values:
        0 = New
        1 = Partially filled
        2 = Filled
        3 = Done for day
        4 = Canceled
        5 = Replaced (Removed/Replaced)
        6 = Pending Cancel (e.g. result of Order Cancel Request <F>)
        7 = Stopped
        8 = Rejected
        9 = Suspended
        A = Pending New
        B = Calculated
        C = Expired
        D = Accepted for bidding
        E = Pending Replace (e.g. result of Order Cancel/Replace Request <G>)
     */
    enum order_state_t {
        ost_New = '0',
        ost_Partially_Filled= '1',
        ost_Filled= '2',
        ost_Done_For_Day= '3',
        ost_Canceled= '4',
        ost_Replaced= '5',
        ost_Pending_Cancel= '6',
        ost_Stopped= '7',
        ost_Rejected= '8',
        ost_Suspended= '9',
        ost_Pending = 'A',
        ost_Calculated = 'B',
        ost_Expired = 'C',
        ost_Accepted_For_Bidding = 'D',
        ost_Cancel_Rejected,
        ost_Pending_Replace,
        ost_Replace_Rejected,
        ost_Undefined
    };

    /*
        FIX 4.4 : ExecType <150> field
        Type: char
        Used In
        Description
        Describes the specific Execution Report (i.e. Pending Cancel) while OrdStatus <39> will
        always identify the current order status (i.e. Partially Filled) *** SOME VALUES HAVE BEEN REPLACED
        - See "Replaced Features and Supported Approach" ***
        Valid values:
        0 = New
        1 = Partial fill (Replaced)
        2 = Fill (Replaced)
        3 = Done for day
        4 = Canceled
        5 = Replace
        6 = Pending Cancel (e.g. result of Order Cancel Request <F>)
        7 = Stopped
        8 = Rejected
        9 = Suspended
        A = Pending New
        B = Calculated
        C = Expired
        D = Restated (Execution Report <8> sent unsolicited by sellside, with ExecRestatementReason <378> set)
        E = Pending Replace (e.g. result of Order Cancel/Replace Request <G>)
        F = Trade (partial fill or fill)
        G = Trade Correct (formerly an ExecTransType <20>)
        H = Trade Cancel (formerly an ExecTransType <20>)
        I = Order Status (formerly an ExecTransType <20>)
     */
    enum report_type_t {
        rt_new = '0',
        rt_partial_fill = '1',
        rt_fill = '2',
        rt_done_for_day = '3',
        rt_canceled = '4',
        rt_replaced = '5',
        rt_pending_cancel = '6',
        rt_stopped = '7',
        rt_rejected = '8',
        rt_suspended = '9',
        rt_pending_new = 'A',
        rt_calculated = 'B',
        rt_expired = 'C',
        rt_restated = 'D',
        rt_pending_replace = 'E',
        rt_fill_trade = 'F',
        rt_trade_correct = 'G',
        rt_trade_cancel = 'H',
        rt_trade_status = 'I',
        rt_cancel_rejected ,
        rt_replace_rejected ,
        rt_undefined ,
    };

    enum exec_trans_t
    {
        ett_undefined = -1,
        ett_new ='0',
        ett_cancel='1',
        ett_correct='2',
        ett_status='3'
    };

    enum ord_reject_response_to_t
    {
        rrt_undefined = -1,
        rrt_cancel_request = '1',
        rrt_cancel_reprace_request = '2'
    };

    enum business_reject_reason
    {
        brr_unavailable = -1,
        brr_other = 0,
        brr_unknown_id = 1,
        brr_unknown_security = 2,
        brr_unsupported_message_type = 3,
        brr_application_not_available = 4,
        brr_conditionally_Required_field_missing = 5,
        brr_not_autorized = 6
    };

    enum market_depth_t
    {
        full=0,
        top = 1,
    };

    /*
        0 = Broker / Exchange option
        1 = Unknown symbol
        2 = Exchange closed
        3 = Order exceeds limit
        4 = Too late to enter
        5 = Unknown Order
        6 = Duplicate Order (e.g. dupe ClOrdID <11> ())
        7 = Duplicate of a verbally communicated order
        8 = Stale Order
        9 = Trade Along required
        99 = Other
     */

    enum ord_rej_reason
    {
        rr_broker=0,
        rr_unknown_symbol=1,
        rr_exchange_closed =2,
        rr_order_exceeds_limit=3,
        rr_too_late_to_enter=4,
        rr_unknown_order=5,
        rr_duplicate_order=6,
        rr_duplicate_communicated_order=7,
        rr_stale_order=8,
        rr_trade_along_required=9,
        rr_other=99
    };

      enum broker_event {
		info,
		debug,
		error,
		session_logon,
		session_logout,
		coin_subscribed,
		coin_unsubscribed,
		connector_started,
		connector_stopped,
		coin_subscribed_fault,
		coin_unsubscribed_fault,
		subscribed_coin_is_not_valid
	};

    enum md_reject_reason_t
    {
        unknown_symbol = '0',
    };
}