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
To Test the interoperability of 802.11a/b/n in 2.4 GHz frequency band.


SCENARIO:-
---------
There are 3 nodes in total. All are part of the same wireless subnet.
All the nodes operate in 2.4 GHz band.
Node 1 is AP. 
Node 1 is 802.11a, node 2 is 802.11b and node 3 is 802.11n.



                  1 - 2 - 3
                   

Application:
------------
CBR session is configured between Node 1 and Node 2, Node 1 and Node 3.



RUN:-
----
Run '<QUALNET_HOME>/bin/qualnet 2_4_ghz.config'.



DESCRIPTION OF THE FILES:-
-------------------------
1. 2_4_ghz.app -  QualNet configuration file for application input.
2. 2_4_ghz.config - QualNet configuration input file.
3. 2_4_ghz.nodes - QualNet node placement file for the simulation run.
4. 2_4_ghz.expected.stat - QualNet statistics collection.
5. README - This file.

