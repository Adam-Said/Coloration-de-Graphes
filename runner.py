import os
import time
from subprocess import Popen, PIPE

print("Compilation...")
os.system("mkdir obj")
os.system("mkdir bin")
os.system("make")
print("Démarrage du programme")

print("Fichier à parser \n")
fileName = str(input())
print("Port serveur \n")
port = int(input())

f = open("Files/"+fileName, "r")
nbNode = 0
for x in f:
  if x.startswith("p"):
        nbNode = int(x.split(" ")[2])
        break

print("Besoin de",nbNode , "clients")

file = '"' + fileName + '"'
print("[Runner] Démarrage du serveur sur " + str(port))
call = "./bin/serverMultiplex " + str(port) + " " + file +" &"
os.system(call)
time.sleep(1)
for x in range(0,nbNode) :
  time.sleep(0.2)
  tmpPort = port+x+1
  print("[Runner] Démarrage du client" , x , "sur" , str(tmpPort) ,"au serveur", str(port))
  call = "./bin/client 127.0.0.1 " + str(port) + " " + str(tmpPort) +" &"
  os.system(call)
  

time.sleep(20)
print("[Runner] Fin du programme\n[Runner] Destruction des processus")

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
            print("[Runner] Killed client.")
        else:
            print("[Runner] Killed server.")
