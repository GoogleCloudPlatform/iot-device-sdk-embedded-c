#!/usr/bin/env python3

"""Tool to pull public keys from Microchip ATECC* secure element.

Dependencies installation:
    pip install cryptoauthlib ecdsa

Usage example:
    $ python atecc_get_pubkey.py --slot 2
"""

from cryptoauthlib import *
from ecdsa import VerifyingKey
from ecdsa.curves import NIST256p

ATCA_SUCCESS = 0x00


def key_to_pem(key_raw):
    k = VerifyingKey.from_string(key_raw, curve=NIST256p)
    return k.to_pem()

def hex_str(bytearray):
    return ''.join(format(x, '02X') for x in bytearray)

if __name__ == '__main__':
    import argparse

    parser = argparse.ArgumentParser(description='Get the public key from a slot')
    parser.add_argument('slot', type=int, help='Slot ID')
    args = parser.parse_args()

    load_cryptoauthlib()
    assert ATCA_SUCCESS == atcab_init(cfg_ateccx08a_kithid_default())

    key = bytearray(64)
    assert ATCA_SUCCESS == atcab_get_pubkey(args.slot, key)
    print("Public key from slot %d: %s" % (args.slot, hex_str(key)))
    print()
    print("In PEM format:\n%s" % key_to_pem(key).decode())
