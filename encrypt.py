from Crypto.Cipher import AES
from Crypto.Util.Padding import pad
from Crypto.Random import get_random_bytes
import hashlib
import json
import os

KEY = b'1234567890abcdef'

with open("firmware.ino.bin", "rb") as f:
    fw = f.read()

sz = len(fw) # Get actual size
sha = hashlib.sha256(fw).hexdigest()
iv = get_random_bytes(16)

c = AES.new(KEY, AES.MODE_CBC, iv)
enc = iv + c.encrypt(pad(fw, 16))

with open("firmware.enc", "wb") as f:
    f.write(enc)

meta = {
    "ver": 35,
    "sha": sha,
    "sz": sz  # Send size
}

m_plain = json.dumps(meta, separators=(',', ':')).encode()
iv_m = get_random_bytes(16)
c_m = AES.new(KEY, AES.MODE_CBC, iv_m)
m_enc = iv_m + c_m.encrypt(pad(m_plain, 16))

with open("meta.enc", "wb") as f:
    f.write(m_enc)

print(f"Done. Ver: 35, Size: {sz}")
