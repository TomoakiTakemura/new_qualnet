# Copyright (c) 2001-2015, SCALABLE Network Technologies, Inc.  All Rights Reserved.
#                          600 Corporate Pointe
#                          Suite 1200
#                          Culver City, CA 90230
#                          info@scalable-networks.com
#
# This source code is licensed, not sold, and is subject to a written
# license agreement.  Among other things, no portion of this source
# code may be copied, transmitted, disclosed, displayed, distributed,
# translated, used as the basis for a derivative work, or used, in
# whole or in part, for any program or purpose other than its intended
# use in compliance with the license agreement as part of the QualNet
# software.  This source code and certain of the algorithms contained
# within it are confidential trade secrets of Scalable Network
# Technologies, Inc. and may not be used as the basis for any other
# software, hardware, product or service.


PURPOSE:-
--------
To Test the working of Very High Throughput - Transmission Opportunity (VHT-TxOP) Power save mode in 802.11ac.


SCENARIO:-
---------
There are 4 nodes in total. All are part of the same wireless subnet.
Node 1 is AP.
All nodes are of type 802.11ac and each node has 6 antennas.
VHT-TxOP Power save mode is enabled on all the nodes.


                  1 - 2 - 3 - 4
                   

Application:
------------
CBR session is configured between Node 1 and Node 2, Node 1 and Node 3, Node 1 and Node 4.



RUN:-
----
Run '<QUALNET_HOME>/bin/qualnet vht_txop_ps.config'.



DESCRIPTION OF THE FILES:-
-------------------------
1. vht_txop_ps.app -  QualNet configuration file for application input.
2. vht_txop_ps.config - QualNet configuration input file.
3. vht_txop_ps.nodes - QualNet node placement file for the simulation run.
4. vht_txop_ps.expected.stat - QualNet statistics collection.
5. README - This file.

