#!/bin/bash
# one-time setups for special demi development area
#
echo $PATH | grep -q hops4 || {
    [ `hostname` = demi ] && export PATH=/opt/hops4/bin:${PATH}
}
# eof
