#!/usr/bin/env python3

# Copyright 2019 - NXP
#
# Permission is hereby granted, free of charge, to any person obtaining
# A copy of this software and associated documentation files
# (the "Software"), to deal in the Software without restriction,
# Including without limitation the rights to use, copy, modify, merge,
# Publish, distribute, sublicense, and/or sell copies of the Software,
# And to permit persons to whom the Software is furnished to do so,
# Subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# Included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
# IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
# CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
# TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
# SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

import sys
import struct
import codecs
from trace_writer import TraceWriter
from babeltrace import CTFWriter, CTFStringEncoding

CPUS_NUM = 3

AF_DIRS_switcher = {
    0: "IN",
    1: "OUT"
}
AF_STAGES_switcher = {
    0: "NULL",
    1: "CP",
    2: "IM",
    3: "ADE",
    4: "DECODER",
    5: "PPP",
    6: "PPA",
    7: "OM"
}
AF_CP_MSG_switcher = {
#IM
    0:  "IM_SETUP_REQ",
    1:  "IM_SETUP_CNF",
    2:  "IM_OPEN_REQ",
    3:  "IM_OPEN_CNF",
    4:  "IM_START_REQ",
    5:  "IM_START_CNF",
    6:  "IM_STOP_REQ",
    7:  "IM_STOP_CNF",
    8:  "IM_DECODER_IND",
    9:  "IM_DECODER_RSP",
    10: "IM_CLOSE_REQ",
    11: "IM_CLOSE_CNF",
    12: "IM_DEVICE_EVT_IND", 
#IMRX
    256: "IMRX_START_REQ",
    257: "IMRX_START_CNF",
    258: "IMRX_STOP_REQ",
    259: "IMRX_STOP_CNF",
    260: "IMRX_DATA_REQ",
    261: "IMRX_DATA_CNF",
    262: "IMRX_ERROR_IND",
    263: "IMRX_AUDIO_HAL_EVT",
#OM
    512: "OM_SETUP_REQ",
    513: "OM_SETUP_CNF",
    514: "OM_SETUP_DELAY_REQ",
    515: "OM_SETUP_DELAY_CNF",
    516: "OM_SETUP_ROUTE_REQ",
    517: "OM_SETUP_ROUTE_CNF",
    518: "OM_OPEN_REQ",
    519: "OM_OPEN_CNF",
    520: "OM_START_REQ",
    521: "OM_START_CNF",
    522: "OM_FLUSH_REQ",
    523: "OM_FLUSH_CNF",
    524: "OM_STOP_REQ",
    525: "OM_STOP_CNF",
    526: "OM_CLOSE_REQ",
    527: "OM_CLOSE_CNF",
    528: "OM_MUTE_REQ",
    529: "OM_MUTE_CNF",
    530: "OM_SET_PARAM_REQ",
    531: "OM_SET_PARAM_CNF",
    532: "OM_ACTIVE_IND",
    533: "OM_AUDIO_HAL_EVT",
    534: "OM_VOICE_IND",
#CP_PING
    768: "CP_PING_IND",
    769: "CP_PING_RSP",
#CP
    1024: "CP_REST_CMD_REQ",
    1025: "CP_REST_CMD_CNF",
    1026: "CP_EVENT_IND",
#SPP
    1280: "PP_SETUP_REQ",
    1281: "PP_SETUP_CNF",
    1282: "PP_START_REQ",
    1283: "PP_START_CNF",
    1284: "PP_FLUSH_REQ",
    1285: "PP_FLUSH_CNF",
    1286: "PP_STOP_REQ",
    1287: "PP_STOP_CNF",
#DEC
    1536: "DEC_START_REQ",
    1537: "DEC_START_CNF",
    1538: "DEC_INFO_IND",
    1539: "DEC_INFO_RSP",
    1540: "DEC_STATUS_IND",
    1541: "DEC_CONFIG_REQ",
    1542: "DEC_CONFIG_CNF",
    1543: "DEC_FLUSH_REQ",
    1544: "DEC_FLUSH_CNF",
    1545: "DEC_STOP_REQ",
    1546: "DEC_STOP_CNF",
}

class TraceWriterLK(TraceWriter):
    def __init__(self, path, cpus_num):
        super().__init__(path, cpus_num)

    def define_lk_binary(self):
        self.lk_binary = CTFWriter.EventClass("binary")
        self.lk_binary.add_field(self.string_type, "_hex")
        self.add_event(self.lk_binary)

    def define_lk_string(self):
        self.lk_string = CTFWriter.EventClass("printf")
        self.lk_string.add_field(self.string_type, "_str")
        self.add_event(self.lk_string)

    def define_lk_preemp(self):
        self.lk_preemp = CTFWriter.EventClass("KERNEL_EVLOG_PREEMPT")
        self.lk_preemp.add_field(self.int32_type, "_tid")
        self.lk_preemp.add_field(self.array16_type, "_comm")
        self.add_event(self.lk_preemp)

    def define_lk_timer_tick(self):
        self.lk_timer_tick = CTFWriter.EventClass("KERNEL_EVLOG_TIMER_TICK")
        self.add_event(self.lk_timer_tick)

    def define_lk_timer_call(self):
        self.lk_timer_call = CTFWriter.EventClass("KERNEL_EVLOG_TIMER_CALL")
        self.lk_timer_call.add_field(self.uint64_type, "_call")
        self.lk_timer_call.add_field(self.uint64_type, "_arg")
        self.add_event(self.lk_timer_call)

    def define_lk_af_data(self):
        self.lk_af_data = CTFWriter.EventClass("AF")
        self.lk_af_data.add_field(self.string_type, "__id")
        self.lk_af_data.add_field(self.string_type, "__io")
        self.lk_af_data.add_field(self.uint32_type, "_id")
        self.lk_af_data.add_field(self.uint32_type, "_sample_rate")
        self.lk_af_data.add_field(self.uint32_type, "_bits_per_sample")
        self.lk_af_data.add_field(self.uint32_type, "_num_channels")
        self.lk_af_data.add_field(self.uint32_type, "_format")
        self.lk_af_data.add_field(self.uint32_type, "_chunk_size")
        self.lk_af_data.add_field(self.uint8_type, "_endian")
        self.lk_af_data.add_field(self.uint8_type, "_sign")
        self.add_event(self.lk_af_data)

    def define_lk_af_ctrl(self):
        self.lk_af_ctrl = CTFWriter.EventClass("AFCTRL")
        self.lk_af_ctrl.add_field(self.string_type, "__id")
        self.lk_af_ctrl.add_field(self.string_type, "__io")
        self.lk_af_ctrl.add_field(self.string_type, "_opc")
        self.add_event(self.lk_af_ctrl)

    def write_lk_binary(self, time_us, cpu_id, hex):
        event = CTFWriter.Event(self.lk_binary)
        self.clock.time = time_us
        self.set_int(event.payload("_cpu_id"), cpu_id)
        self.set_string(event.payload("_hex"), hex)
        self.stream[cpu_id].append_event(event)
        self.stream[cpu_id].flush()

    def write_lk_string(self, time_us, cpu_id, string):
        event = CTFWriter.Event(self.lk_string)
        self.clock.time = time_us
        self.set_int(event.payload("_cpu_id"), cpu_id)
        self.set_string(event.payload("_str"), string)
        self.stream[cpu_id].append_event(event)
        self.stream[cpu_id].flush()

    def write_lk_preemp(self, time_us, cpu_id, tid, comm):
        event = CTFWriter.Event(self.lk_preemp)
        self.clock.time = time_us
        self.set_char_array(event.payload("_comm"), comm)
        self.set_int(event.payload("_cpu_id"), cpu_id)
        self.set_int(event.payload("_tid"), tid)
        self.stream[cpu_id].append_event(event)
        self.stream[cpu_id].flush()

    def write_lk_timer_tick(self, time_us, cpu_id):
        event = CTFWriter.Event(self.lk_timer_tick)
        self.clock.time = time_us
        self.set_int(event.payload("_cpu_id"), cpu_id)
        self.set_int(event.payload("_tid"), tid)
        self.stream[cpu_id].append_event(event)
        self.stream[cpu_id].flush()

    def write_lk_timer_call(self, time_us, cpu_id, call, arg):
        event = CTFWriter.Event(self.lk_timer_call)
        self.clock.time = time_us
        self.set_int(event.payload("_cpu_id"), cpu_id)
        self.set_int(event.payload("_call"), call)
        self.set_int(event.payload("_arg"), arg)
        self.stream[cpu_id].append_event(event)
        self.stream[cpu_id].flush()

    def write_lk_af_data(self, time_us, cpu_id, _id, _io, id, sample_rate, bits_per_sample, num_channels, format, chunk_size, endian, sign):
        event = CTFWriter.Event(self.lk_af_data)
        self.clock.time = time_us
        self.set_int(event.payload("_cpu_id"), cpu_id)
        self.set_string(event.payload("__id"), AF_STAGES_switcher.get(_id))
        self.set_string(event.payload("__io"), AF_DIRS_switcher.get(_io))
        self.set_int(event.payload("_id"), id)
        self.set_int(event.payload("_sample_rate"), sample_rate)
        self.set_int(event.payload("_bits_per_sample"), bits_per_sample)
        self.set_int(event.payload("_num_channels"), num_channels)
        self.set_int(event.payload("_format"), format)
        self.set_int(event.payload("_chunk_size"), chunk_size)
        self.set_int(event.payload("_endian"), endian)
        self.set_int(event.payload("_sign"), sign)
        self.stream[cpu_id].append_event(event)
        self.stream[cpu_id].flush()

    def write_lk_af_ctrl(self, time_us, cpu_id, _id, _io, opc):
        event = CTFWriter.Event(self.lk_af_ctrl)
        self.clock.time = time_us
        self.set_int(event.payload("_cpu_id"), cpu_id)
        self.set_string(event.payload("__id"), AF_STAGES_switcher.get(_id))
        self.set_string(event.payload("__io"), AF_DIRS_switcher.get(_io))
        self.set_string(event.payload("_opc"), AF_CP_MSG_switcher.get(opc,"unknown"))
        self.stream[cpu_id].append_event(event)
        self.stream[cpu_id].flush()

    def define_events(self):
        super().define_events()
        self.define_lk_binary()
        self.define_lk_string()
        self.define_lk_preemp()
        self.define_lk_timer_tick()
        self.define_lk_timer_call()
        self.define_lk_af_data()
        self.define_lk_af_ctrl()

class Entry:
    def __init__(self, timestamp, type, subtype, cpu_id, data, length):
        self.timestamp = timestamp
        self.type = type
        self.subtype = subtype
        self.cpu_id = cpu_id
        self.data = data
        self.length = length

    def __eq__(self, other):
        if not isinstance(other, Entry):
            return NotImplemented
        return self.timestamp == other.timestamp

    def __lt__(self, other):
        if not isinstance(other, Entry):
            return NotImplemented
        return self.timestamp < other.timestamp

    def __le__(self, other):
        if not isinstance(other, Entry):
            return NotImplemented
        return self.timestamp <= other.timestamp

    def __gt__(self, other):
        if not isinstance(other, Entry):
            return NotImplemented
        return self.timestamp > other.timestamp

    def __ge__(self, other):
        if not isinstance(other, Entry):
            return NotImplemented
        return self.timestamp >= other.timestamp

class subtype_id:
    EMPTY = 0
    CONTEXT_SWITCH = 1
    PREEMPT = 2
    TIMER_TICK = 3
    TIMER_CALL = 4
    IRQ_ENTER = 5
    IRQ_EXIT = 6
    NUM = 7

class subtype_af_id:
    AF_EVLOG_NULL = 0
    AF_EVLOG_CP = 1
    AF_EVLOG_IM = 2
    AF_EVLOG_ADE = 3
    AF_EVLOG_DECODER = 4
    AF_EVLOG_PPP = 5
    AF_EVLOG_PPA = 6
    AF_EVLOG_OM = 7

class type_id:
    STR = 0
    THREAD = 1
    BINARY = 2
    AF = 3

def get_chunk(filename, chunksize=32):
    with open(filename, "rb") as f:
        while True:
            chunk = f.read(chunksize)
            if chunk:
                yield chunk
            else:
                break


if __name__ == '__main__':
    if len(sys.argv) < 3:
        print(f"Usage: {sys.argv[0]} <bin> <ctf>");
        sys.exit(1)

    _last_valid_timestamp=0
    trace_writer = TraceWriterLK(sys.argv[2], CPUS_NUM)

    print("Writing trace at {}".format(trace_writer.trace_path))

    header_pack = "=IQBBH"
    header_size = struct.calcsize(header_pack)
    cpu = [[] for i in range(CPUS_NUM)]

    with open(sys.argv[1], "rb") as f:
        while True:
            header = f.read(header_size)
            if not header:
                break

            (_magic, _timestamp, _type, _cpu_id, _len) = struct.unpack(header_pack, header)

            if format(_magic, "02x") != "deadbeef":
                print("Wrong magic! Last valid timestamp found:", _last_valid_timestamp)
                break

            type = (_type & 0xf)
            subtype = ((_type >> 4) & 0xf)
            data = f.read(_len)

            _timestamp = _timestamp * 1000

            _last_valid_timestamp = _timestamp

            entry = Entry(_timestamp, type, subtype, _cpu_id, data, _len)
            cpu[_cpu_id].append(entry)

    tot_events = 0
    for i in range(CPUS_NUM):
        tot_events += len(cpu[i])

    for i in range(tot_events):
        v = []
        for j in range(CPUS_NUM):
            if len(cpu[j]) > 0:
                v.append(cpu[j][0])

        v_sorted = sorted(v)
        event = v_sorted[0]
        cpu[event.cpu_id].pop(0)

        try:
            if (event.type == type_id.THREAD):
                if (event.subtype == subtype_id.IRQ_ENTER):
                    (_irq_num, ) = struct.unpack("B", event.data)
                    irq_name = f"#{_irq_num}"
                    trace_writer.write_irq_handler_entry(event.timestamp, event.cpu_id, _irq_num, irq_name)

                elif (event.subtype == subtype_id.IRQ_EXIT):
                    (_irq_num, ) = struct.unpack("B", event.data)
                    irq_name = f"#{_irq_num}"
                    trace_writer.write_irq_handler_exit(event.timestamp, event.cpu_id, _irq_num, 0)

                elif (event.subtype == subtype_id.CONTEXT_SWITCH):
                    (_prev_comm, _next_comm, _prev_tid, _next_tid, _prev_prio, _next_prio) = struct.unpack("32s32sIIII", event.data)
                    _prev_comm = _prev_comm.decode()
                    _next_comm = _next_comm.decode()
                    trace_writer.write_sched_switch(event.timestamp, event.cpu_id, _prev_comm, _prev_tid, _next_comm, _next_tid, _prev_prio, 1, _next_prio)

                elif (event.subtype == subtype_id.PREEMPT):
                    (_comm, _tid, _prio) = struct.unpack("32sII", event.data)
                    _comm = _comm.decode()
                    trace_writer.write_lk_preemp(event.timestamp, event.cpu_id, _tid, _comm)

                elif (event.subtype == subtype_id.TIMER_TICK):
                    trace_writer.write_lk_timer_call(event.timestamp, event.cpu_id)

                elif (event.subtype == subtype_id.TIMER_CALL):
                    (_callback, _arg) = struct.unpack("QQ", event.data)
                    trace_writer.write_lk_timer_call(event.timestamp, event.cpu_id, _callback, _arg)

            elif (event.type == type_id.STR):
                (_str, ) = struct.unpack(f"{_len}s", event.data)
                _str = _str.decode()
                trace_writer.write_lk_string(event.timestamp, event.cpu_id, _str)

            elif (event.type == type_id.BINARY):
                hex = codecs.encode(event.data, "hex")
                trace_writer.write_lk_binary(event.timestamp, event.cpu_id, hex)

            elif (event.type == type_id.AF):
                if (event.subtype == subtype_af_id.AF_EVLOG_CP):
                    (__id, __io, opc) = struct.unpack("2BI", event.data)
                    trace_writer.write_lk_af_ctrl(event.timestamp, event.cpu_id, __id, __io, opc)
                else:
                    (__id, __io, _id, _magic, _sample_rate, _bits_per_sample, _num_channels, _format, _chunk_size, _endian, _sign, _, _) = \
                            struct.unpack("2B7I2B2B", event.data)
                    trace_writer.write_lk_af_data(event.timestamp, event.cpu_id, __id, __io, _id, _sample_rate, _bits_per_sample, _num_channels, _format, _chunk_size, _endian, _sign)

        except ValueError:
            pass
