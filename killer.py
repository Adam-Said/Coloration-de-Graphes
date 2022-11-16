import os
import time
from subprocess import Popen, PIPE

print("Destruction des fichiers générés")
os.system("make clean")

print("Destruction des processus")

res = Popen(["ps", "aux"], stdout=PIPE)
res = ''.join(map(chr, (res.communicate())[0]))


for r in res.split('\n'):
    if (("bin/client" in r) or ("bin/serverMultiplex" in r)):
        num = r.split(' ')
        for n in num:
            if len(n) > 0 and n[0] in '0123456789':
                os.system("kill " + n)
                break
        if "client" in r:
            print("Killed client.")
        else:
            print("Killed server.")
