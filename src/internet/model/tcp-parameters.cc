/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * 
 */


#include "tcp-parameters.h"

namespace ns3 {
  TcpParameters::TcpParameters () : m_msl (0)
                                  , m_nullIsn (true)
                                  , m_minRto (Time::Max ())
                                  , m_clockGranularity (Seconds (0.001))
                                  , m_retxThresh (3)
                                  , m_limitedTx (false)
                                  , m_maxWinSize (0)
                                  , m_mptcpEnabled (false)
                                  , m_winScalingEnabled (false)
                                  , m_timestampEnabled (false)
                                  , m_cnTimeout (Seconds (0.0))
                                  , m_synRetries (0)
                                  , m_dataRetries (0)
                                  , m_delAckTimeout (Seconds (0.0))
                                  , m_delAckMaxCount (0)
                                  , m_noDelay (false)
                                  , m_persistTimeout (Seconds (0.0))
                                  , m_closeNotified (false)
                                  , m_closeOnEmpty (false)
                                  , m_shutdownSend (false)
                                  , m_shutdownRecv (false)
  
  {
    
  }
  
  TcpParameters::TcpParameters (const TcpParameters& params) :  m_msl (params.m_msl)
                                                              , m_nullIsn (params.m_nullIsn)
                                                              , m_minRto (params.m_minRto)
                                                              , m_clockGranularity (params.m_clockGranularity)
                                                              , m_retxThresh (params.m_retxThresh)
                                                              , m_limitedTx (params.m_limitedTx)
                                                              , m_maxWinSize (params.m_maxWinSize)
                                                              , m_mptcpEnabled (params.m_mptcpEnabled)
                                                              , m_winScalingEnabled (params.m_winScalingEnabled)
                                                              , m_timestampEnabled (params.m_timestampEnabled)
                                                              , m_cnTimeout (params.m_cnTimeout)
                                                              , m_synRetries (params.m_synRetries)
                                                              , m_dataRetries (params.m_dataRetries)
                                                              , m_delAckTimeout (params.m_delAckTimeout)
                                                              , m_delAckMaxCount (params.m_delAckMaxCount)
                                                              , m_noDelay (params.m_noDelay)
                                                              , m_persistTimeout (params.m_persistTimeout)
                                                              , m_closeNotified (params.m_closeNotified)
                                                              , m_closeOnEmpty (params.m_closeOnEmpty)
                                                              , m_shutdownSend (params.m_shutdownSend)
                                                              , m_shutdownRecv (params.m_shutdownRecv)
  {
  }

  TcpParameters::~TcpParameters ()
  {
    
  }
  
}