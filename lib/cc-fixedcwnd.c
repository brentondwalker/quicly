/*
 * Copyright (c) 2019 Fastly, Janardhan Iyengar
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 */
#include "quicly/cc.h"
#include "quicly.h"

/* TODO: Avoid increase if sender was application limited. */
static void fixedcwnd_on_acked(quicly_cc_t *cc, const quicly_loss_t *loss, uint32_t bytes, uint64_t largest_acked, uint32_t inflight,
                          uint64_t next_pn, int64_t now, uint32_t max_udp_payload_size)
{
    assert(inflight >= bytes);

    /* only know how to do one thing */
    cc->cwnd = cc->state.fixedcwnd.cwnd_val;
    cc->cwnd_maximum = cc->state.fixedcwnd.cwnd_val;
}

void quicly_cc_fixedcwnd_on_lost(quicly_cc_t *cc, const quicly_loss_t *loss, uint32_t bytes, uint64_t lost_pn, uint64_t next_pn,
                            int64_t now, uint32_t max_udp_payload_size)
{
  //quicly_cc__update_ecn_episodes(cc, bytes, lost_pn);

    /* only know how to do one thing */
    cc->cwnd = cc->state.fixedcwnd.cwnd_val;
    cc->cwnd_maximum = cc->state.fixedcwnd.cwnd_val;
    cc->ssthresh = cc->cwnd;    
}

void quicly_cc_fixedcwnd_on_persistent_congestion(quicly_cc_t *cc, const quicly_loss_t *loss, int64_t now)
{
    /* TODO */
}

void quicly_cc_fixedcwnd_on_sent(quicly_cc_t *cc, const quicly_loss_t *loss, uint32_t bytes, int64_t now)
{
    /* Unused */
}

static void fixedcwnd_reset(quicly_cc_t *cc, uint32_t initcwnd)
{
    memset(cc, 0, sizeof(quicly_cc_t));
    cc->type = &quicly_cc_type_fixedcwnd;
    cc->cwnd = cc->cwnd_initial = cc->cwnd_maximum = cc->state.fixedcwnd.cwnd_val = initcwnd;
    cc->ssthresh = cc->cwnd_minimum = cc->state.fixedcwnd.cwnd_val = initcwnd;
}

static int fixedcwnd_on_switch(quicly_cc_t *cc)
{
    if (cc->type == &quicly_cc_type_fixedcwnd) {
        return 1; /* nothing to do */
    } else if (cc->type == &quicly_cc_type_pico) {
        cc->type = &quicly_cc_type_fixedcwnd;
        cc->state.fixedcwnd.stash = cc->state.pico.stash;
        return 1;
    } else if (cc->type == &quicly_cc_type_cubic) {
        /* When in slow start, state can be reused as-is; otherwise, restart. */
        if (cc->cwnd_exiting_slow_start == 0) {
            cc->type = &quicly_cc_type_fixedcwnd;
        } else {
            fixedcwnd_reset(cc, cc->cwnd_initial);
        }
        return 1;
    }

    return 0;
}

static void fixedcwnd_init(quicly_init_cc_t *self, quicly_cc_t *cc, uint32_t initcwnd, int64_t now)
{
    fixedcwnd_reset(cc, initcwnd);
}

quicly_cc_type_t quicly_cc_type_fixedcwnd = {"fixedcwnd",
                                        &quicly_cc_fixedcwnd_init,
                                        fixedcwnd_on_acked,
                                        quicly_cc_fixedcwnd_on_lost,
                                        quicly_cc_fixedcwnd_on_persistent_congestion,
                                        quicly_cc_fixedcwnd_on_sent,
                                        fixedcwnd_on_switch};
quicly_init_cc_t quicly_cc_fixedcwnd_init = {fixedcwnd_init};

