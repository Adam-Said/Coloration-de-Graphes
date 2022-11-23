import os
import time
from subprocess import Popen, PIPE

print("Compilation...")
os.system("mkdir obj")
os.system("mkdir bin")
os.system("make")
print("Démarrage du programme")

print("Nombre de client:  \n")
nbClient = int(input())
print("IP Serveur:  \n")
ipServeur = str(input())
print("Port Serveur: \n")
portServeur = int(input())
print("Port premier client \n")
portClient = int(input())


print("Lancement de", nbClient , " clients")

for x in range(nbClient) :
  time.sleep(0.05)
  tmpPort = portClient+x+1
  print("[Runner] Démarrage du client" , x , "sur" , str(tmpPort) ,"au serveur")
  call = "./bin/client "+ str(ipServeur) + " " + str(portServeur) + " " + str(tmpPort) +" &"
  os.system(call)
  


