def bin_pad(n):
   s = bin(n)[2:]

   pad = 8
   while True:
       if pad >= len(s):
           break
       pad += 8

   s = s.rjust(pad,'0')

   r = ''

   i = 0

   while i < pad:
       r += s[i:i+8] + ' '
       i += 8
   return r[:-1]

def bin_pad_hex(h):
    return bin_pad(int(h,16))
