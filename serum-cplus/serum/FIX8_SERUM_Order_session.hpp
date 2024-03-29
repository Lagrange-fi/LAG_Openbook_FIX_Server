//-------------------------------------------------------------------------------------------------
/*

Fix8 is released under the GNU LESSER GENERAL PUBLIC LICENSE Version 3.

Fix8 Open Source FIX Engine.
Copyright (C) 2010-23 David L. Dight <fix@fix8.org>

Fix8 is free software: you can  redistribute it and / or modify  it under the  terms of the
GNU Lesser General  Public License as  published  by the Free  Software Foundation,  either
version 3 of the License, or (at your option) any later version.

Fix8 is distributed in the hope  that it will be useful, but WITHOUT ANY WARRANTY;  without
even the  implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

You should  have received a copy of the GNU Lesser General Public  License along with Fix8.
If not, see <http://www.gnu.org/licenses/>.

*******************************************************************************************
*                Special note for Fix8 compiler generated source code                     *
*                                                                                         *
* Binary works  that are the results of compilation of code that is generated by the Fix8 *
* compiler  can be released  without releasing your  source code as  long as your  binary *
* links dynamically  against an  unmodified version of the Fix8 library.  You are however *
* required to leave the copyright text in the generated code.                             *
*                                                                                         *
*******************************************************************************************

BECAUSE THE PROGRAM IS  LICENSED FREE OF  CHARGE, THERE IS NO  WARRANTY FOR THE PROGRAM, TO
THE EXTENT  PERMITTED  BY  APPLICABLE  LAW.  EXCEPT WHEN  OTHERWISE  STATED IN  WRITING THE
COPYRIGHT HOLDERS AND/OR OTHER PARTIES  PROVIDE THE PROGRAM "AS IS" WITHOUT WARRANTY OF ANY
KIND,  EITHER EXPRESSED   OR   IMPLIED,  INCLUDING,  BUT   NOT  LIMITED   TO,  THE  IMPLIED
WARRANTIES  OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.  THE ENTIRE RISK AS TO
THE QUALITY AND PERFORMANCE OF THE PROGRAM IS WITH YOU. SHOULD THE PROGRAM PROVE DEFECTIVE,
YOU ASSUME THE COST OF ALL NECESSARY SERVICING, REPAIR OR CORRECTION.

IN NO EVENT UNLESS REQUIRED  BY APPLICABLE LAW  OR AGREED TO IN  WRITING WILL ANY COPYRIGHT
HOLDER, OR  ANY OTHER PARTY  WHO MAY MODIFY  AND/OR REDISTRIBUTE  THE PROGRAM AS  PERMITTED
ABOVE,  BE  LIABLE  TO  YOU  FOR  DAMAGES,  INCLUDING  ANY  GENERAL, SPECIAL, INCIDENTAL OR
CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OR INABILITY TO USE THE PROGRAM (INCLUDING BUT
NOT LIMITED TO LOSS OF DATA OR DATA BEING RENDERED INACCURATE OR LOSSES SUSTAINED BY YOU OR
THIRD PARTIES OR A FAILURE OF THE PROGRAM TO OPERATE WITH ANY OTHER PROGRAMS), EVEN IF SUCH
HOLDER OR OTHER PARTY HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.

*/

//-------------------------------------------------------------------------------------------------
#include <fix8/f8config.h>
#if defined FIX8_MAGIC_NUM && FIX8_MAGIC_NUM > 16793601L
#error FIX8_SERUM_Order_session.hpp version 1.4.1 is out of date. Please regenerate with f8c.
#endif
//-------------------------------------------------------------------------------------------------
// FIX8_SERUM_Order_session.hpp
//-------------------------------------------------------------------------------------------------
#ifndef FIX8_464958385F534552554D5F4F726465725F73657373696F6E2E687070_
#define FIX8_464958385F534552554D5F4F726465725F73657373696F6E2E687070_

//-------------------------------------------------------------------------------------------------
// server session and router classes
//-------------------------------------------------------------------------------------------------
class FIX8_SERUM_Order_session_server;

class FIX8_SERUM_Order_router_server : public FIX8::SERUM_Order::FIX8_SERUM_Order_Router
{
   FIX8_SERUM_Order_session_server& _session; 

public:
   FIX8_SERUM_Order_router_server(FIX8_SERUM_Order_session_server& session) : _session(session) {}
   virtual ~FIX8_SERUM_Order_router_server() {}

   // Override these methods to receive specific message callbacks.
   // bool operator() (const FIX8::SERUM_Order::Heartbeat *msg) const;
   // bool operator() (const FIX8::SERUM_Order::TestRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ResendRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::Reject *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SequenceReset *msg) const;
   // bool operator() (const FIX8::SERUM_Order::Logout *msg) const;
   // bool operator() (const FIX8::SERUM_Order::IOI *msg) const;
   // bool operator() (const FIX8::SERUM_Order::Advertisement *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ExecutionReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::OrderCancelReject *msg) const;
   // bool operator() (const FIX8::SERUM_Order::Logon *msg) const;
   // bool operator() (const FIX8::SERUM_Order::DerivativeSecurityList *msg) const;
   // bool operator() (const FIX8::SERUM_Order::NewOrderMultileg *msg) const;
   // bool operator() (const FIX8::SERUM_Order::MultilegOrderCancelReplace *msg) const;
   // bool operator() (const FIX8::SERUM_Order::TradeCaptureReportRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::TradeCaptureReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::OrderMassStatusRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::QuoteRequestReject *msg) const;
   // bool operator() (const FIX8::SERUM_Order::RFQRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::QuoteStatusReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::QuoteResponse *msg) const;
   // bool operator() (const FIX8::SERUM_Order::Confirmation *msg) const;
   // bool operator() (const FIX8::SERUM_Order::PositionMaintenanceRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::PositionMaintenanceReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::RequestForPositions *msg) const;
   // bool operator() (const FIX8::SERUM_Order::RequestForPositionsAck *msg) const;
   // bool operator() (const FIX8::SERUM_Order::PositionReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::TradeCaptureReportRequestAck *msg) const;
   // bool operator() (const FIX8::SERUM_Order::TradeCaptureReportAck *msg) const;
   // bool operator() (const FIX8::SERUM_Order::AllocationReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::AllocationReportAck *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ConfirmationAck *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SettlementInstructionRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::AssignmentReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::CollateralRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::CollateralAssignment *msg) const;
   // bool operator() (const FIX8::SERUM_Order::CollateralResponse *msg) const;
   // bool operator() (const FIX8::SERUM_Order::News *msg) const;
   // bool operator() (const FIX8::SERUM_Order::CollateralReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::CollateralInquiry *msg) const;
   // bool operator() (const FIX8::SERUM_Order::NetworkCounterpartySystemStatusRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::NetworkCounterpartySystemStatusResponse *msg) const;
   // bool operator() (const FIX8::SERUM_Order::UserRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::UserResponse *msg) const;
   // bool operator() (const FIX8::SERUM_Order::CollateralInquiryAck *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ConfirmationRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::Email *msg) const;
   // bool operator() (const FIX8::SERUM_Order::NewOrderSingle *msg) const;
   // bool operator() (const FIX8::SERUM_Order::NewOrderList *msg) const;
   // bool operator() (const FIX8::SERUM_Order::OrderCancelRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::OrderCancelReplaceRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::OrderStatusRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::AllocationInstruction *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ListCancelRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ListExecute *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ListStatusRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ListStatus *msg) const;
   // bool operator() (const FIX8::SERUM_Order::AllocationInstructionAck *msg) const;
   // bool operator() (const FIX8::SERUM_Order::DontKnowTrade *msg) const;
   // bool operator() (const FIX8::SERUM_Order::QuoteRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::Quote *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SettlementInstructions *msg) const;
   // bool operator() (const FIX8::SERUM_Order::MarketDataRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::MarketDataSnapshotFullRefresh *msg) const;
   // bool operator() (const FIX8::SERUM_Order::MarketDataIncrementalRefresh *msg) const;
   // bool operator() (const FIX8::SERUM_Order::MarketDataRequestReject *msg) const;
   // bool operator() (const FIX8::SERUM_Order::QuoteCancel *msg) const;
   // bool operator() (const FIX8::SERUM_Order::QuoteStatusRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::MassQuoteAcknowledgement *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SecurityDefinitionRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SecurityDefinition *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SecurityStatusRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SecurityStatus *msg) const;
   // bool operator() (const FIX8::SERUM_Order::TradingSessionStatusRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::TradingSessionStatus *msg) const;
   // bool operator() (const FIX8::SERUM_Order::MassQuote *msg) const;
   // bool operator() (const FIX8::SERUM_Order::BusinessMessageReject *msg) const;
   // bool operator() (const FIX8::SERUM_Order::BidRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::BidResponse *msg) const;
   // bool operator() (const FIX8::SERUM_Order::ListStrikePrice *msg) const;
   // bool operator() (const FIX8::SERUM_Order::RegistrationInstructions *msg) const;
   // bool operator() (const FIX8::SERUM_Order::RegistrationInstructionsResponse *msg) const;
   // bool operator() (const FIX8::SERUM_Order::OrderMassCancelRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::OrderMassCancelReport *msg) const;
   // bool operator() (const FIX8::SERUM_Order::NewOrderCross *msg) const;
   // bool operator() (const FIX8::SERUM_Order::CrossOrderCancelReplaceRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::CrossOrderCancelRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SecurityTypeRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SecurityTypes *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SecurityListRequest *msg) const;
   // bool operator() (const FIX8::SERUM_Order::SecurityList *msg) const;
   // bool operator() (const FIX8::SERUM_Order::DerivativeSecurityListRequest *msg) const;
};

//-------------------------------------------------------------------------------------------------
class FIX8_SERUM_Order_session_server : public FIX8::Session
{
   FIX8_SERUM_Order_router_server _router; 

public:
   FIX8_SERUM_Order_session_server(const FIX8::F8MetaCntx& ctx, FIX8::Persister *persist=0,
      FIX8::Logger *logger=0, FIX8::Logger *plogger=0) : Session(ctx, persist, logger, plogger), _router(*this) {} 

   // Override these methods if required but remember to call the base class method first.
   // bool handle_logon(const unsigned seqnum, const FIX8::Message *msg);
   // Message *generate_logon(const unsigned heartbeat_interval, const f8String davi=f8String());
   // bool handle_logout(const unsigned seqnum, const FIX8::Message *msg);
   // Message *generate_logout();
   // bool handle_heartbeat(const unsigned seqnum, const FIX8::Message *msg);
   // Message *generate_heartbeat(const f8String& testReqID);
   // bool handle_resend_request(const unsigned seqnum, const FIX8::Message *msg);
   // Message *generate_resend_request(const unsigned begin, const unsigned end=0);
   // bool handle_sequence_reset(const unsigned seqnum, const FIX8::Message *msg);
   // Message *generate_sequence_reset(const unsigned newseqnum, const bool gapfillflag=false);
   // bool handle_test_request(const unsigned seqnum, const FIX8::Message *msg);
   // Message *generate_test_request(const f8String& testReqID);
   // bool handle_reject(const unsigned seqnum, const FIX8::Message *msg);
   // Message *generate_reject(const unsigned seqnum, const char *what);
   // bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);
   // void modify_outbound(FIX8::Message *msg);
   // bool authenticate(SessionID& id, const FIX8::Message *msg);

   // Override these methods to intercept admin and application methods.
   // bool handle_admin(const unsigned seqnum, const FIX8::Message *msg);

   bool handle_application(const unsigned seqnum, const FIX8::Message *&msg);
   /* In your compilation unit, this should be implemented with something like the following:
   bool FIX8_SERUM_Order_session_server::handle_application(const unsigned seqnum, const FIX8::Message *&msg)
   {
      return enforce(seqnum, msg) || msg->process(_router);
   }
   */
};

#endif // FIX8_464958385F534552554D5F4F726465725F73657373696F6E2E687070_
