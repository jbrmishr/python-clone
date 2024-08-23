import sys
import warnings
import struct

from _ctypes import CField
import ctypes

def round_down(n, multiple):
    assert(n >= 0);
    assert(multiple >= 0);
    if (multiple == 0):
        return n;
    return int(n / multiple) * multiple;

def round_up(n, multiple):
    assert(n >= 0);
    assert(multiple >= 0);
    if (multiple == 0):
        return n;
    return int((n + multiple - 1) / multiple) * multiple;

def LOW_BIT(offset):
    return offset & 0xFFFF;

def NUM_BITS(bitsize):
    return bitsize >> 16;

def BUILD_SIZE(bitsize, offset):
    assert(0 <= offset);
    assert(offset <= 0xFFFF);
    ## We don't support zero length bitfields.
    ## And GET_BITFIELD uses NUM_BITS(size)==0,
    ## to figure out whether we are handling a bitfield.
    assert(0 < bitsize);
    result = (bitsize << 16) + offset;
    assert(bitsize == NUM_BITS(result));
    assert(offset == LOW_BIT(result));
    return result;

class _BaseLayout:
    def __init__(self, cls, fields, is_struct, base, **kwargs):
        if kwargs:
            warnings.warn(f'Unknown keyword arguments: {list(kwargs.keys())}')
        self.cls = cls
        self.fields = fields
        self.is_struct = is_struct

        self.size = 0
        self.offset = 0
        base_align = 1
        if base:
            if is_struct:
                self.size = self.offset = ctypes.sizeof(base)
            base_align = ctypes.alignment(base)
        #print(base, is_struct, self.size)
        self.base = base

        self.align = getattr(cls, '_align_', 1)
        if self.align < 0:
            raise ValueError('_align_ must be a non-negative integer')
        elif self.align == 0:
            # Setting `_align_ = 0` amounts to using the default alignment
            self.align == 1

        self.swapped_bytes = hasattr(cls, '_swappedbytes_')
        if self.swapped_bytes:
            self.big_endian = sys.byteorder == 'little'
        else:
            self.big_endian = sys.byteorder == 'big'

        self.total_align = max(self.align, base_align)

        self._pack_ = getattr(cls, '_pack_', None)

    def __iter__(self):
        self.gave_up = False
        state_field_size = 0
        state_bitofs = 0
        state_offset = 0
        state_size = 0
        state_align = 0
        #print(self.base, self.is_struct, self.size, self.size * 8)
        if self.base and self.is_struct:
            state_size = state_offset = ctypes.sizeof(self.base)
            state_align = ctypes.alignment(self.base)


        for i, field in enumerate(self.fields):
            if not self.is_struct:
                if isinstance(self, GCCSysVLayout):
                    state_field_size = 0
                    state_bitofs = 0
                    state_offset = 0
                    state_size = 0
                    state_align = 0

            field = tuple(field)
            try:
                name, ftype = field
                is_bitfield = False
                bit_size = ctypes.sizeof(ftype)
            except ValueError:
                name, ftype, bit_size = field
                is_bitfield = True
                if bit_size <= 0:
                    raise ValueError(f'number of bits invalid for bit field {name!r}')
            size = ctypes.sizeof(ftype)
            offset = self.offset

            info_size = ctypes.sizeof(ftype)
            info_align = ctypes.alignment(ftype)

            if isinstance(self, GCCSysVLayout):
                # We don't use packstate->offset here, so clear it, if it has been set.
                state_bitofs += state_offset * 8;
                state_offset = 0

                if is_bitfield:
                    bitsize = bit_size
                else:
                    bitsize = 8 * info_size;

                ## We don't use packstate->offset here, so clear it, if it has been set.
                state_bitofs += state_offset * 8;
                state_offset = 0;

                assert(self._pack_ in (0, None)); ## TODO: This shouldn't be a C assertion

                state_align = info_align;

                ## Determine whether the bit field, if placed at the next free bit,
                ## fits within a single object of its specified type.
                ## That is: determine a "slot", sized & aligned for the specified type,
                ## which contains the bitfield's beginning:
                slot_start_bit = round_down(state_bitofs, 8 * info_align);
                slot_end_bit = slot_start_bit + 8 * info_size;
                ## And see if it also contains the bitfield's last bit:
                field_end_bit = state_bitofs + bitsize;
                if field_end_bit > slot_end_bit:
                    ## It doesn't: add padding (bump up to the next alignment boundary)
                    state_bitofs = round_up(state_bitofs, 8 * info_align);

                assert(state_offset == 0);

                offset = int(round_down(state_bitofs, 8 * info_align) / 8);
                if is_bitfield:
                    effective_bitsof = state_bitofs - 8 * offset;
                    size = BUILD_SIZE(bitsize, effective_bitsof);
                    assert(effective_bitsof <= info_size * 8);
                else:
                    size = info_size;

                state_bitofs += bitsize;
                state_size = int(round_up(state_bitofs, 8) / 8);
            else:
                if is_bitfield:
                    bitsize = bit_size
                else:
                    bitsize = 8 * info_size;

                if self._pack_:
                    state_align = min(self._pack_, info_align);
                else:
                    state_align = info_align;

                ## packstate->offset points to end of current bitfield.
                ## packstate->bitofs is generally non-positive,
                ## and 8 * packstate->offset + packstate->bitofs points just behind
                ## the end of the last field we placed.
                if ((0 < state_bitofs + bitsize) or (8 * info_size != state_field_size)):
                    ## Close the previous bitfield (if any).
                    ## and start a new bitfield:
                    state_offset = round_up(state_offset, state_align);

                    state_offset += info_size;

                    state_field_size = info_size * 8;
                    ## Reminder: 8 * (packstate->offset) + bitofs points to where we would start a
                    ## new field.  Ie just behind where we placed the last field plus an
                    ## allowance for alignment.
                    state_bitofs = - state_field_size;

                assert(8 * info_size == state_field_size);

                offset = state_offset - int((state_field_size) // 8);
                if is_bitfield:
                    assert(0 <= (state_field_size + state_bitofs));
                    assert((state_field_size + state_bitofs) < info_size * 8);
                    size = BUILD_SIZE(bitsize, state_field_size + state_bitofs);
                else:
                    size = info_size;
                assert(state_field_size + state_bitofs <= info_size * 8);

                state_bitofs += bitsize;
                state_size = state_offset;


            ################################## State check (remove this)
            state_to_check = struct.pack(
                "nnnnn",
                state_field_size, state_bitofs, state_offset, state_size, state_align
            )
            ##################################

            assert((not is_bitfield) or (LOW_BIT(size) <= size * 8));
            if self.big_endian and is_bitfield:
                size = BUILD_SIZE(NUM_BITS(size), 8*info_size - LOW_BIT(size) - bit_size);

            yield CField(
                name=name,
                type=ftype,
                size=size,
                offset=offset,
                bit_size=bit_size if is_bitfield else None,
                swapped_bytes=self.swapped_bytes,
                pack=self._pack_,
                index=i,
                state_to_check=state_to_check,
                **self._field_args(),
            )
            if self.is_struct:
                self.offset += self.size

    def _field_args(self):
        return {}


class WindowsLayout(_BaseLayout):
    def _field_args(self):
        return {'_ms': True}

class GCCSysVLayout(_BaseLayout):
    def __init__(self, cls, *args, **kwargs):
        if getattr(cls, '_pack_', None):
            raise ValueError('_pack_ is not compatible with gcc-sysv layout')
        return super().__init__(cls, *args, **kwargs)

if sys.platform == 'win32':
    NativeLayout = WindowsLayout
    DefaultLayout = WindowsLayout
else:
    NativeLayout = GCCSysVLayout

def default_layout(cls, *args, **kwargs):
    layout = getattr(cls, '_layout_', None)
    if layout is None:
        if sys.platform == 'win32' or getattr(cls, '_pack_', None):
            return WindowsLayout(cls, *args, **kwargs)
        return GCCSysVLayout(cls, *args, **kwargs)
    elif layout == 'ms':
        return WindowsLayout(cls, *args, **kwargs)
    elif layout == 'gcc-sysv':
        return GCCSysVLayout(cls, *args, **kwargs)
    else:
        raise ValueError(f'unknown _layout_: {layout!r}')
